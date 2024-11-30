#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;

uniform sampler2D heightmap;

// Bezier curve control points
// uniform vec3 control_p0;
// uniform vec3 control_p1;
// uniform vec3 control_p2;
// uniform vec3 control_p3;

// layout(std140) uniform ControlPoints {
//     vec3 control_p0;
//     vec3 control_p1;
//     vec3 control_p2;
//     vec3 control_p3;
// };

// control_p0 = glm::vec3(-100.0, 4, 0.0);
// control_p1 = glm::vec3(-90.0, 3.0, 0.0);
// control_p2 = glm::vec3(-80.0, 2.0, 0.0);
// control_p3 = glm::vec3(-70.0, 1.0, 0.0);

vec3 control_p0 = vec3(-5.0, 0.5, 0.0);
vec3 control_p1 = vec3(-4.0, -0.8, 0.0);
vec3 control_p2 = vec3(-3.0, -0.9, 0.0);
vec3 control_p3 = vec3(-2.5, -1.0, 0.0);

in int instance[1];

vec3 bezier(float t)
{
    return  control_p0 * pow((1 - t), 3) +
            control_p1 * 3 * t * pow((1 - t), 2) +
            control_p2 * 3 * pow(t, 2) * (1 - t) +
            control_p3 * pow(t, 3);
}

void main()
{
    vec2 terrain_size = vec2(20, 20);
    vec2 resolution = vec2(512, 512);

    vec2 pos_step = terrain_size / resolution;
    vec2 tex_step = 1.0f / resolution;

    vec2 start_pos = vec2(-terrain_size.x / 2, -terrain_size.y / 2);
    vec2 final_pos = start_pos + vec2(instance[0] % int(resolution.x), instance[0] / int(resolution.y)) * pos_step;

    vec2 texture_coords = final_pos / terrain_size;
    float factor = 15.0f;

    /*
        Based on final position, we calculate the height of the terrain at that position, for each vertex of the quad.
        If the distance between the final position and the center is smaller than a certain threshold,
        we consider the height to be negative, so we create a sinkhole.
        Otherwise, we consider the height to be positive, so we create a hill, using the heightmap texture.
    */
    vec2 center = vec2(0, 0);

    float height_1 = 0.0f;
    float height_2 = 0.0f;
    float height_3 = 0.0f;
    float height_4 = 0.0f;

    vec3 final_pos_1 = vec3(final_pos.x, 0, final_pos.y);
    vec3 final_pos_2 = vec3(final_pos.x, 0, final_pos.y + pos_step.y);
    vec3 final_pos_3 = vec3(final_pos.x + pos_step.x, 0, final_pos.y);
    vec3 final_pos_4 = vec3(final_pos.x + pos_step.x, 0, final_pos.y + pos_step.y);

    // Calculate the height offset so that the terrain starts at y = 
    float height_offset = factor * texture(heightmap, vec2(0, 0)).r - 0.5f;

    float d_sinkhole = 2.5f;
    float h_sinkhole = -1.0f;

    float d_bezier = 0.5f;

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
        // height_1 = 10.0f;
    } else if (distance(final_pos_1, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_1 = h_sinkhole;
        // height_1 = -100.0f;
    } else {
        height_1 = factor * texture(heightmap, texture_coords).r - height_offset;
    }

    if (min_dist_2 < d_bezier)
    {
        height_2 = bezier(t_min_2).y;
        // height_2 = 10.0f;
    } else if (distance(final_pos_2, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_2 = h_sinkhole;
        // height_2 = -100.0f;
    } else {
        height_2 = factor * texture(heightmap, texture_coords + vec2(0, tex_step.y)).r - height_offset;
    }

    if (min_dist_3 < d_bezier)
    {
        height_3 = bezier(t_min_3).y;
        // height_3 = 10.0f;
    } else if (distance(final_pos_3, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_3 = h_sinkhole;
        // height_3 = -100.0f;
    } else {
        height_3 = factor * texture(heightmap, texture_coords + vec2(tex_step.x, 0)).r - height_offset;
    }

    if (min_dist_4 < d_bezier)
    {
        height_4 = bezier(t_min_4).y;
        // height_4 = 10.0f;
    } else if (distance(final_pos_4, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_4 = h_sinkhole;
        // height_4 = -100.0f;
    } else {
        height_4 = factor * texture(heightmap, texture_coords + tex_step).r - height_offset;
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
}
