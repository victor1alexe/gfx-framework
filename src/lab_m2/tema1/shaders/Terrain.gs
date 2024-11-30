#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;

uniform sampler2D heightmap;

in int instance[1];

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

    float d_sinkhole = 1.5f;
    float h_sinkhole = 0.0f;

    if (distance(final_pos_1, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_1 = h_sinkhole;
    } else {
        height_1 = factor * texture(heightmap, texture_coords).r - height_offset;
    }

    if (distance(final_pos_2, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_2 = h_sinkhole;
    } else {
        height_2 = factor * texture(heightmap, texture_coords + vec2(0, tex_step.y)).r - height_offset;
    }

    if (distance(final_pos_3, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_3 = h_sinkhole;
    } else {
        height_3 = factor * texture(heightmap, texture_coords + vec2(tex_step.x, 0)).r - height_offset;
    }

    if (distance(final_pos_4, vec3(center.x, 0, center.y)) < d_sinkhole)
    {
        height_4 = h_sinkhole;
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
