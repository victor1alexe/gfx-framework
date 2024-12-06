#version 430

// Input
layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;
layout(location = 2) in vec4 clip_space;

// Uniform properties
uniform sampler2D texture_color;
uniform sampler2D texture_light;

// Output
layout(location = 0) out vec4 out_world_position;
layout(location = 1) out vec4 out_world_normal;
layout(location = 2) out vec4 out_color;

vec2 uv_reflection = clip_space.xy / clip_space.w * 0.5 + 0.5;

vec3 color()
{
    return texture(texture_color, uv_reflection).xyz;
}

vec3 light()
{
    return texture(texture_light, uv_reflection).xyz;
}

void main()
{
    out_world_position = vec4(world_position, 1);
    out_world_normal = vec4(normalize(world_normal), 0);

    
    // out_color = texture(texture_color, uv_reflection) * texture(texture_light, uv_reflection);
    out_color = vec4(color(), 1) * vec4(light(), 1);
}
