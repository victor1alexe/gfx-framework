#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;

// Heightmap texture
uniform sampler2D heightmap;

// Instance id
in int instance[1];

struct vertex_info {
    vec3 position;
    vec2 tex_coords;
};

// Define offset activation for each vertex of the quad
// vec2(0, 0) -> bottom left
// vec2(0, 1) -> top left
// vec2(1, 0) -> bottom right
// vec2(1, 1) -> top right
vec2 offset_activation[4] = {vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1)};

#define PI 3.14159265359

#define WATERFALL 0
#define SINKHOLE 1
#define HILL 2

struct base_height_info {
    float height;
    int type;
    float distance_to_center;
};

// Used to store the minimum distance between a point and the bezier curve
struct bezier_info {
    float t;
    float distance;
};

vec2 terrain_size = vec2(8, 8);   // size of the terrain in world space
vec2 resolution = vec2(512, 512);   // how many vertices we have in the terrain

vec2 world_step = terrain_size / resolution;  // step between two vertices in world space
vec2 tex_step = 1.0f / resolution;  // step between two vertices in texture space

vec3 world_center = vec3(0, 0, 0); // center of the terrain
vec3 world_start_pos = world_center - vec3(terrain_size.x / 2.0f, 0, terrain_size.y / 2.0f); // start position of the terrain

float d_sinkhole = 1.0f;    // threshold distance for sinkhole
float d_waterfall = 0.25f;   // threshold distance for waterfall
float h_max = 1.5;  // maximum height of the terrain

float global_tex_factor = 2.5f; // global factor for the heightmap texture

int first_vertex_row = instance[0] / int(resolution.y); // row of the first vertex of the quad
int first_vertex_col = instance[0] % int(resolution.x); // column of the first vertex of the quad

vertex_info vertices[4]; // vertices of the quad

// Bezier control points for waterfall/
// Define so that one end is at the center of the terrain and the other end is at the edge of the terrain
vec3 control_p0 = vec3(-1.5 * d_sinkhole,   h_max,          0.0);
vec3 control_p1 = vec3(-1.4 * d_sinkhole,   0.9 * h_max,    0.0);
vec3 control_p2 = vec3(-1.0 * d_sinkhole,  0.0,   0.0);
vec3 control_p3 = vec3(world_center.x,      -h_max,            0.0);

// Calculate the bezier curve of the waterfall at a certain t parameter
vec3 bezier(float t)
{
    return  control_p0 * pow((1 - t), 3) +
            control_p1 * 3 * t * pow((1 - t), 2) +
            control_p2 * 3 * pow(t, 2) * (1 - t) +
            control_p3 * pow(t, 3);
}

bezier_info closest_point_on_bezier(vec3 point)
{
    bezier_info info;
    info.t = 0.0f;
    info.distance = 1000000.0f;

    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        vec3 curve_point = bezier(t);
        float distance = distance(vec3(point.x, 0, point.z), vec3(curve_point.x, 0, curve_point.z));

        if (distance < info.distance)
        {
            info.t = t;
            info.distance = distance;
        }
    }

    return info;
}

base_height_info base_height(vec3 point)
{
    base_height_info info;
    info.height = 0.0f;
    info.type = HILL;
    info.distance_to_center = 0.0f;

    float distance_to_center = distance(vec3(point.x, 0, point.z), vec3(world_center.x, 0, world_center.z));
    distance_to_center /= d_sinkhole;
    info.distance_to_center = distance_to_center;

    if (distance_to_center < 1.0f)
    {
        info.type = SINKHOLE;

        info.height = h_max * pow(distance_to_center, 2) / 2.0f;
        // Perform offset so that the rim of the sinkhole is at the same height as the terrain
        info.height -= h_max / 2.0f;
        info.height += 0.15f / 2.0f * h_max;
    } else {
        info.height = 0.15f * h_max * (1.0f - pow(2 - distance_to_center, 2) / 2);
    }

    return info;
}

void main()
{
    vec3 world_pos_first_vertex = world_start_pos + vec3(world_step.x * first_vertex_col, 0, world_step.y * first_vertex_row);   // world position of the first vertex of the quad

    // Calculate the position of the vertices of the quad
    for (int i = 0; i < 4; i++) {
        // Calculate x and z position of the vertex
        vec3 offset = vec3(world_step.x * offset_activation[i].x, 0, world_step.y * offset_activation[i].y);
        vertices[i].position = world_pos_first_vertex + offset;

        // Calculate height (y coordinate) of the vertex
        /*
         * There are 3 cases:
         * 1. The vertex is inside the waterfall, so we calculate the height based on the bezier curve.
         * 2. The vertex is inside the sinkhole, so we calculate the height based on the distance to the center of the sinkhole.
         * 3. The vertex is outside the waterfall and the sinkhole, so we calculate the height based on the heightmap texture.
         */

        base_height_info base_info = base_height(vertices[i].position);
        switch (base_info.type) {
            // Case 2: The vertex is inside the sinkhole
            case SINKHOLE:
                vertices[i].position.y = base_info.height;
                break;
            // Case 3: The vertex is outside the waterfall and the sinkhole
            case HILL:
                vertices[i].position.y = base_info.height;
                // If the distance between the vertex and the center of the terrain is greater than a certain threshold, we apply the heightmap texture
                if (base_info.distance_to_center > d_sinkhole) {
                    // We want the noise texture to matter less as we are closer to the center of the terrain
                    // The range of tex_factor is [0, 1]
                    // We want a linear decrease of tex_factor as we move away from the center of the terrain
                    // float tex_factor = 1.0f - (terrain_size.x - base_info.distance_to_center * terrain_size.x) / terrain_size.x;
                    float tex_factor = (base_info.distance_to_center < d_sinkhole) ? 0.0f : (base_info.distance_to_center - d_sinkhole) / base_info.distance_to_center;
                    tex_factor *= global_tex_factor;
                    vertices[i].position.y += tex_factor * texture(heightmap, vec2(vertices[i].position.x, vertices[i].position.z)).r;
                }
                break;
        }

        // Case 1: The vertex is inside the waterfall
        bezier_info info = closest_point_on_bezier(vertices[i].position);
        vertices[i].position.y = mix(bezier(info.t).y, vertices[i].position.y, 1 - sin(PI / 2 - PI / 2 * clamp(info.distance / d_waterfall, 0.0f, 1.0f))); // Interpolate between the height of the bezier curve and the height of the vertex
    }

    // Emit the vertices of the quad
    for (int i = 0; i < 4; i++) {
        gl_Position = Projection * View * vec4(vertices[i].position, 1.0f);
        EmitVertex();
    }

    EndPrimitive();
}

/*
// vec2 world_pos_first_vertex = world_start_pos + world_step * vec2(instance[0] % int(resolution.x), instance[0] / int(resolution.y));   // current position in world space based on the instance id
    // vec2 tex_pos = world_pos_first_vertex / terrain_size; // current position in texture space
 float factor = 1.0f;
        // Based on final position, we calculate the height of the terrain at that position, for each vertex of the quad.
        // If the distance between the final position and the center is smaller than a certain threshold,
        // we consider the height to be negative, so we create a sinkhole.
        // Otherwise, we consider the height to be positive, so we create a hill, using the heightmap texture.
    
    vec2 center = vec2(0, 0);
    float height_offset = 0.0f;
    float d_sinkhole = 2.5f;
    float h_sinkhole = -1.0f;
    float d_bezier = 0.5f;
    float hmax = 0.45f;

    float height_1 = 0.0f;
    float height_2 = 0.0f;
    float height_3 = 0.0f;
    float height_4 = 0.0f;

    // height_1 = hmax * (1.0f - pow(2 - distance(world_pos_first_vertex, vec2(center.x, center.y)), 2) / 2);
    // height_2 = hmax * (1.0f - pow(2 - distance(world_pos_first_vertex + world_step * vec2(0, 1), vec2(center.x, center.y)), 2) / 2);
    // height_3 = hmax * (1.0f - pow(2 - distance(world_pos_first_vertex + world_step * vec2(1, 0), vec2(center.x, center.y)), 2) / 2);
    // height_4 = hmax * (1.0f - pow(2 - distance(world_pos_first_vertex + world_step, vec2(center.x, center.y)), 2) / 2);

    vec3 final_pos_1 = vec3(world_pos_first_vertex.x, 0, world_pos_first_vertex.y);
    vec3 final_pos_2 = vec3(world_pos_first_vertex.x, 0, world_pos_first_vertex.y + world_step.y);
    vec3 final_pos_3 = vec3(world_pos_first_vertex.x + world_step.x, 0, world_pos_first_vertex.y);
    vec3 final_pos_4 = vec3(world_pos_first_vertex.x + world_step.x, 0, world_pos_first_vertex.y + world_step.y);

    // Calculate the height offset so that the terrain starts at y = 
    // float height_offset = factor * texture(heightmap, vec2(0, 0)).r - 0.5f;

    // iterate through the bezier curve with small increments
    // find the t parameter for which the distance between the final position and the point on the curve is minimum
    // calculate the height of the terrain at that position as the height of the point on the curve
    float min_dist_1 = 1000000.0f;
    float t_min_1 = 0.0f;
    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        vec3 point = bezier(t);
        // float dist = distance(final_pos_1, point);
        // calculate the distance ignoring the y component
        float dist = distance(vec3(final_pos_1.x, 0, final_pos_1.z), vec3(point.x, 0, point.z));
        if (dist < min_dist_1)
        {
            min_dist_1 = dist;
            t_min_1 = t;
        }
    }

    float min_dist_2 = 1000000.0f;
    float t_min_2 = 0.0f;
    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        vec3 point = bezier(t);
        // float dist = distance(final_pos_2, point);
        float dist = distance(vec3(final_pos_2.x, 0, final_pos_2.z), vec3(point.x, 0, point.z));
        if (dist < min_dist_2)
        {
            min_dist_2 = dist;
            t_min_2 = t;
        }
    }

    float min_dist_3 = 1000000.0f;
    float t_min_3 = 0.0f;
    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        vec3 point = bezier(t);
        // float dist = distance(final_pos_3, point);
        float dist = distance(vec3(final_pos_3.x, 0, final_pos_3.z), vec3(point.x, 0, point.z));
        if (dist < min_dist_3)
        {
            min_dist_3 = dist;
            t_min_3 = t;
        }
    }

    float min_dist_4 = 1000000.0f;
    float t_min_4 = 0.0f;
    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        vec3 point = bezier(t);
        // float dist = distance(final_pos_4, point);
        float dist = distance(vec3(final_pos_4.x, 0, final_pos_4.z), vec3(point.x, 0, point.z));
        if (dist < min_dist_4)
        {
            min_dist_4 = dist;
            t_min_4 = t;
        }
    }

    if (min_dist_1 < d_bezier)
    {
        height_1 = bezier(t_min_1).y;
        // float hypothetical_height = factor * texture(heightmap, tex_pos).r - height_offset;
        // height_1 = mix(hypothetical_height, bezier(t_min_1).y, 1 - sin(3.14f / 2 - 3.14f / 2 * clamp(min_dist_1 / d_bezier, 0.0f, 1.0f)));
        // height_1 = 10.0f;
    } else if (distance(final_pos_1, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        // height_1 = h_sinkhole;
        // height_1 = -100.0f;
        height_1 =- 1.0f + pow(distance(final_pos_1, vec3(center.x, 0, center.y)), 2) / 2.0f * hmax;
    } else {
        height_1 += factor * texture(heightmap, tex_pos).r - height_offset;
    }

    if (min_dist_2 < d_bezier)
    {
        height_2 = bezier(t_min_2).y;
        // float hypothetical_height = factor * texture(heightmap, tex_pos + vec2(0, tex_step.y)).r - height_offset;
        // height_2 = mix(hypothetical_height, bezier(t_min_2).y, 1 - sin(3.14f / 2 - 3.14f / 2 * clamp(min_dist_2 / d_bezier, 0.0f, 1.0f)));
        // height_2 = 10.0f;
    } else if (distance(final_pos_2, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        // height_2 = h_sinkhole;
        // height_2 = -100.0f;
        height_2 = -1.0f + pow(distance(final_pos_2, vec3(center.x, 0, center.y)), 2) / 2.0f * hmax;
    } else {
        height_2 += factor * texture(heightmap, tex_pos + vec2(0, tex_step.y)).r - height_offset;
    }

    if (min_dist_3 < d_bezier)
    {
        height_3 = bezier(t_min_3).y;
        // float hypothetical_height = factor * texture(heightmap, tex_pos + vec2(tex_step.x, 0)).r - height_offset;
        // height_3 = mix(hypothetical_height, bezier(t_min_3).y, 1 - sin(3.14f / 2 - 3.14f / 2 * clamp(min_dist_3 / d_bezier, 0.0f, 1.0f)));
        // height_3 = 10.0f;
    } else if (distance(final_pos_3, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        // height_3 = h_sinkhole;
        // height_3 = -100.0f;
        height_3 = -1.0f + pow(distance(final_pos_3, vec3(center.x, 0, center.y)), 2) / 2.0f * hmax;
    } else {
        height_3 += factor * texture(heightmap, tex_pos + vec2(tex_step.x, 0)).r - height_offset;
    }

    if (min_dist_4 < d_bezier)
    {
        height_4 = bezier(t_min_4).y;
        // float hypothetical_height = factor * texture(heightmap, tex_pos + tex_step).r - height_offset;
        // height_4 = mix(hypothetical_height, bezier(t_min_4).y, 1 - sin(3.14f / 2 - 3.14f / 2 * clamp(min_dist_4 / d_bezier, 0.0f, 1.0f)));
        // height_4 = 10.0f;
    } else if (distance(final_pos_4, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        // height_4 = h_sinkhole;
        // height_4 = -100.0f;
        height_4 = -1.0f + pow(distance(final_pos_4, vec3(center.x, 0, center.y)), 2) / 2.0f * hmax;
    } else {
        height_4 += factor * texture(heightmap, tex_pos + tex_step).r - height_offset;
    }

    final_pos_1.y = height_1;
    final_pos_2.y = height_2;
    final_pos_3.y = height_3;
    final_pos_4.y = height_4;

    gl_Position = Projection * View * vec4(final_pos_1, 1.0f); EmitVertex();
    gl_Position = Projection * View * vec4(final_pos_2, 1.0f); EmitVertex();
    gl_Position = Projection * View * vec4(final_pos_3, 1.0f); EmitVertex();
    gl_Position = Projection * View * vec4(final_pos_4, 1.0f); EmitVertex();

    EndPrimitive();
*/