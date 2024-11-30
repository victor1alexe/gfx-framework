#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

out vec3 position;

void main()
{
    int instance_id = gl_InstanceID;
    int terrain_x = 1024;
    int terrain_z = 1024;
    int no_of_instances = terrain_x * terrain_z;

    int x = int(mod(instance_id, terrain_x));
    int z = int(instance_id / terrain_x);
    float x_pos = (float(x) - float(terrain_x) / 2.0) * 0.02;
    float z_pos = (float(z) - float(terrain_z) / 2.0) * 0.02;

    position = vec3(v_position.x + x_pos, v_position.y, v_position.z + z_pos);
    gl_Position = Projection * View * Model * vec4(position, 1);
}
