#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;

// out int instance;
// layout(location = 0) out vec2 texture_coord;
// layout(location = 1) out vec3 world_position;
// layout(location = 2) out vec3 world_normal;
layout(location = 0) out int instance;

void main()
{
    // texture_coord = v_texture_coord;
    // world_position = (Model * vec4(v_position, 1.0)).xyz;
    // world_normal = mat3(Model) * v_normal;
    instance = gl_InstanceID;

    gl_Position =  Model * vec4(v_position, 1);
}
