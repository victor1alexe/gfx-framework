#version 430

layout(location = 0) in vec3 position;

uniform samplerCube skybox;

layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;

void main()
{
    out_world_position = vec4(position, 1);
    out_world_normal = vec4(normalize(position), 0);
    out_color = texture(skybox, position);
    // out_color = vec4(position, 1.0);
}

// #version 430

// layout(location = 0) in vec3 position;

// out vec4 color;

// uniform samplerCube skybox;

// void main()
// {
//     color = texture(skybox, position);
//     // color = vec4(position, 1.0);
// }

// #version 430 core
// in vec3 TexCoords;

// out vec4 FragColor;

// uniform samplerCube skybox;

// void main()
// {
//     // FragColor = texture(skybox, TexCoords);
//     FragColor = vec4(1, 1, 1, 1.0);
// }

