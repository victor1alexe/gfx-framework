#version 410

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Output
layout(location = 0) out vec3 world_position;
layout(location = 1) out vec3 world_normal;
layout(location = 2) out vec3 camera_position;

void main()
{
    world_position = (Model * vec4(v_position, 1.0)).xyz;
    world_normal = mat3(Model) * v_normal;
    camera_position = (inverse(View) * vec4(0, 0, 0, 1)).xyz;
    gl_Position = Projection * View * vec4(world_position, 1);
}
