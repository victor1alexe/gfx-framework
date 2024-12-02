#version 430

// Input
layout(location = 0) in vec2 text_coord;
layout(location = 3) in vec3 world_position;
layout(location = 4) in vec3 world_normal;

// Uniform properties
uniform sampler2D texture_1;

// Output
layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;


void main()
{
    out_world_position = vec4(world_position, 1);
    out_world_normal = vec4(normalize(world_normal), 0);
    out_color = texture(texture_1, text_coord);

    if (out_color.a < 0.1)
    {
        discard;
    }
}


// #version 430

// // Input
// layout(location = 0) in vec2 texture_coord;

// // Uniform properties
// uniform sampler2D texture_1;

// // Output
// layout(location = 0) out vec4 out_color;


// void main()
// {
//     vec3 color = texture(texture_1, texture_coord).xyz;
//     out_color = vec4(color, 1);
// }
