#version 430

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

layout(location = 0) out vec3 position;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

void main()
{
    position = vec3(Model * vec4(v_position, 1.0));

    vec4 pos = Projection * View * Model * vec4(v_position, 1.0);    
    gl_Position = pos.xyww;
}

// #version 430 core
// layout(location = 0) in vec3 aPos;

// out vec3 TexCoords;

// uniform mat4 projection;
// uniform mat4 view;

// void main()
// {
//     TexCoords = aPos;
//     vec4 pos = projection * view * vec4(aPos, 1.0);
//     gl_Position = pos.xyww; // Keep the depth at the far plane
// }


