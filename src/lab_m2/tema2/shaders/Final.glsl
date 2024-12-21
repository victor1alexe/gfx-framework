#version 330

// Input
in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage1;
uniform sampler2D textureImage2;

// Output
layout(location = 0) out vec4 out_color;

void main()
{
    vec4 color1 = texture(textureImage1, vec2(texture_coord.x, texture_coord.y));
    vec4 color2 = texture(textureImage2, vec2(texture_coord.x, texture_coord.y));

    out_color = color1 * color2;
}

