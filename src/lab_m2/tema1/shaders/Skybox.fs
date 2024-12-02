#version 430

layout(location = 0) in vec3 position;

out vec4 color;

uniform samplerCube skybox;

void main()
{
    color = texture(skybox, position);
    // color = vec4(position, 1.0);
}

// #version 430 core
// in vec3 TexCoords;

// out vec4 FragColor;

// uniform samplerCube skybox;

// void main()
// {
//     // FragColor = texture(skybox, TexCoords);
//     FragColor = vec4(1, 1, 1, 1.0);
// }

