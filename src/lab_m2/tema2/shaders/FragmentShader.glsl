#version 330

// Input
in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage;

// Output
layout(location = 0) out vec4 out_color;


void main()
{
    out_color = texture(textureImage, texture_coord);
}
