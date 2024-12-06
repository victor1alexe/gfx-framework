#version 430

// Input
layout(location = 0) in vec2 text_coord;
layout(location = 1) in vec3 world_position;
layout(location = 2) in vec3 world_normal;

// Uniform properties
uniform sampler2D texture_terrain;

// Output
layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;


float rand1()
{
    return fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand2()
{
    return fract(sin(dot(world_position.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float rand3()
{
    return fract(sin(dot(world_position.zy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = vec2(rand1(), rand2());

    out_world_position = vec4(world_position, 1);
    out_world_normal = vec4(normalize(world_normal), 0);
    // out_color = texture(texture_terrain, vec2(0.5, 0.5));
    out_color = vec4(0, 1, 1, 1);

    // if (is_butterfly == 1)
    //     out_color *= rand1() * 0.5 + 0.5;
}
