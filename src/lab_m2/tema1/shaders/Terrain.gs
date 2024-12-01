#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;

// Heightmap texture
uniform sampler2D heightmap;

// layout(location = 0) in vec2 tex_coords[1];
// layout(location = 1) in vec3 pos_in[1];
// layout(location = 2) in vec3 normal_in[1];
layout(location = 0) in int instance[1];

layout(location = 0) out vec2 tex_coords_out;
layout(location = 1) out vec3 pos_out;
layout(location = 2) out vec3 normal_out;

struct vertex_info {
    vec3 position;
    vec2 tex_coords;
    vec3 normal;
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
vec3 control_p0 = vec3(-2.5 * d_sinkhole, 0.0, 0.0);
vec3 control_p1 = vec3(-1.4 * d_sinkhole, 0.0, 0.0);
vec3 control_p2 = vec3(-1.0 * d_sinkhole, 0.0, 0.0);
vec3 control_p3 = vec3(world_center.x, -h_max, 0.0);

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

vec3 normal(vec2 uv)
{
    vec3 p = vec3(uv.x, texture(heightmap, uv).r, uv.y);
    vec3 dp1 = vec3(2.0 * world_step.x, texture(heightmap, uv + vec2(tex_step.x, 0)).r - texture(heightmap, uv - vec2(tex_step.x, 0)).r, 0.0);
    vec3 dp2 = vec3(0.0, texture(heightmap, uv + vec2(0, tex_step.y)).r - texture(heightmap, uv - vec2(0, tex_step.y)).r, 2.0 * world_step.y);

    return normalize(cross(dp1, dp2));
}

/* CC edu */
vec3 ComputeNormal(vec2 uv) {
    float hL = texture(heightmap, uv + vec2(-1.0, 0.0) / 2048.0).r;
    float hR = texture(heightmap, uv + vec2(1.0, 0.0) / 2048.0).r;
    float hU = texture(heightmap, uv + vec2(0.0, -1.0) / 2048.0).r;
    float hD = texture(heightmap, uv + vec2(0.0, 1.0) / 2048.0).r;

    return normalize(vec3(hL - hR, 0.1, hD - hU));
}

void main()
{
    vec3 world_pos_first_vertex = world_start_pos + vec3(world_step.x * first_vertex_col, 0, world_step.y * first_vertex_row);   // world position of the first vertex of the quad

    // Calculate the position of the vertices of the quad
    for (int i = 0; i < 4; i++) {
        // Calculate x and z position of the vertex
        vec3 offset = vec3(world_step.x * offset_activation[i].x, 0, world_step.y * offset_activation[i].y);
        vertices[i].position = world_pos_first_vertex + offset;
        vertices[i].tex_coords = vec2((vertices[i].position.x - world_start_pos.x) / terrain_size.x, (vertices[i].position.z - world_start_pos.z) / terrain_size.y);
        vertices[i].normal = normal(vec2(vertices[i].tex_coords.x, vertices[i].tex_coords.y));

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
                    float tex_factor = (base_info.distance_to_center < d_sinkhole) ? 0.0f : (base_info.distance_to_center - d_sinkhole) / base_info.distance_to_center;
                    tex_factor *= global_tex_factor;
                    // normalize position to [0, 1] for texture sampling
                    vertices[i].position.y += tex_factor * texture(heightmap, vertices[i].tex_coords).r;
                }
                break;
        }

        // Case 1: The vertex is inside the waterfall
        bezier_info info = closest_point_on_bezier(vertices[i].position);
        vertices[i].position.y = mix(bezier(info.t).y, vertices[i].position.y, 1 - sin(PI / 2 - PI / 2 * clamp(info.distance / d_waterfall, 0.0f, 1.0f))); // Interpolate between the height of the bezier curve and the height of the vertex
    }

    // Emit the vertices of the quad
    for (int i = 0; i < 4; i++) {
        pos_out = vertices[i].position;
        normal_out = vertices[i].normal;
        tex_coords_out = vertices[i].tex_coords;
        gl_Position = Projection * View * vec4(vertices[i].position, 1.0f);
        EmitVertex();
    }

    EndPrimitive();
}
