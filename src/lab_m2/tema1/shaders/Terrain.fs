#version 430

// Input
// layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 world_position;
layout(location = 2) in vec3 world_normal;

uniform sampler2D heightmap;
uniform sampler2D texture_terrain;

// Output
layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;

vec3 world_center = vec3(0, 0, 0);
vec2 terrain_size = vec2(8, 8);
vec3 terrain_first_v_pos = world_center - vec3(terrain_size.x / 2.0f, 0, terrain_size.y / 2.0f);

vec2 normalize_pos(vec2 pos)
{
    return (pos - terrain_first_v_pos.xz) / terrain_size;
}

void main()
{
    if (world_position.y < 0.0f) {
        discard;
    }

    out_world_position = world_position / world_position.w;
    out_world_normal = vec4(normalize(world_normal), 0);
    // out_color = vec4(world_normal, 1);
    vec2 uv = normalize_pos(world_position.xz / world_position.w);
    out_color = texture(texture_terrain, uv);
}
