#version 430

// Input
layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;
layout(location = 2) in vec3 camera_position;

// Uniform properties
uniform samplerCube skybox;

// Output
layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;


void main()
{
    out_world_position = vec4(world_position, 1);
    out_world_normal = vec4(normalize(world_normal), 0);

    vec3 I = normalize(world_position - camera_position);
    vec3 R = reflect(I, normalize(world_normal));

    out_color = texture(skybox, R);
}
