#version 410

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 View;
uniform mat4 ViewReflected;
uniform mat4 Projection;

// Output
layout(location = 0) out vec3 world_position;
layout(location = 1) out vec3 world_normal;
layout(location = 2) out vec4 clip_space;

void main()
{
    world_position = v_position;
    world_normal = v_normal;

    clip_space = Projection * ViewReflected * vec4(v_position, 1.0);

    gl_Position = Projection * View * vec4(world_position, 1);
}
