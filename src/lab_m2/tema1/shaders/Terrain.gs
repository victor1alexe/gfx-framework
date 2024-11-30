#version 430

layout(points) in;
layout(triangle_strip, max_vertices = 256) out;

uniform mat4 View;
uniform mat4 Projection;

uniform sampler2D heightmap;

in int instance[1];

float rand(int seed)
{
    return fract(sin(float(seed) * 12.9898) * 43758.5453);
}

void main()
{
    vec2 terrain_size = vec2(20, 20);
    vec2 resolution = vec2(512, 512);

    vec2 pos_step = terrain_size / resolution;
    vec2 tex_step = 1.0f / resolution;

    vec2 pos_local = vec2(instance[0] % int(resolution.x), instance[0] / int(resolution.y)) * pos_step;
    vec2 texture_coords = pos_local / terrain_size;

    float factor = 5.0f;

    float height_1 = factor * texture(heightmap, texture_coords + tex_step * vec2(0, 0)).r;
    float height_2 = factor * texture(heightmap, texture_coords + tex_step * vec2(0, 1)).r;
    float height_3 = factor * texture(heightmap, texture_coords + tex_step * vec2(1, 0)).r;
    float height_4 = factor * texture(heightmap, texture_coords + tex_step * vec2(1, 1)).r;

    vec4 position = gl_in[0].gl_Position;
    gl_Position = Projection * View * vec4(pos_local.x, height_1, pos_local.y, 1);
    EmitVertex();

    position = gl_in[0].gl_Position;
    gl_Position = Projection * View * vec4(pos_local.x, height_2, pos_local.y + pos_step.y, 1);
    EmitVertex();

    position = gl_in[0].gl_Position;
    gl_Position = Projection * View * vec4(pos_local.x + pos_step.x, height_3, pos_local.y, 1);
    EmitVertex();

    position = gl_in[0].gl_Position;
    gl_Position = Projection * View * vec4(pos_local.x + pos_step.x, height_4, pos_local.y + pos_step.y, 1);
    EmitVertex();

    EndPrimitive();
}
