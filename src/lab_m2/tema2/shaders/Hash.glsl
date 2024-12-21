#version 330

in vec2 texture_coord;

layout (location=0) out vec4 out_color1;
layout (location=1) out vec4 out_color2;
layout (location=2) out vec4 out_color3;
layout (location=3) out vec4 out_color_final;

uniform sampler2D color_texture;
uniform ivec2 screenSize;

float grayscale(vec4 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

// sin(a⋅x+b⋅y)>c
float hash(float a, float b, float c, float gray, float treshold)
{
    if (gray < treshold)
    {
        return 0.0;
    }

    if (gray > 1.0 - treshold)
    {
        return 1.0;
    }

    return sin(a * gl_FragCoord.x + b * gl_FragCoord.y) > c ? 1 : 0;
}


void main()
{

    float gray = grayscale(texture(color_texture, texture_coord));

    out_color1 = vec4(hash(200, 200, 0.5, gray, 0));
    out_color2 = vec4(hash(-150, 150, 0.8, gray, 0.07));
    out_color3 = vec4(hash(1, -2, 0.1, gray, 0.2));

    out_color_final = clamp(out_color1 + out_color2 + out_color3, 0, 1);
}

