#version 330

in vec2 texture_coord;

out vec4 out_color;

uniform sampler2D color_texture;
uniform ivec2 screenSize;

void main()
{
    vec2 texelSize = 1.0f / vec2(screenSize);
    vec4 sum = vec4(0);

    for(int i = -12; i <= 12; i++)
        sum += texture(color_texture, texture_coord + vec2(0, i) * texelSize);

    out_color = sum / 25;
}