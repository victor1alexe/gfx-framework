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
//    if (gray < treshold)
//        return 0.0;
//
//    if (gray > 1.0 - treshold)
//        return 1.0;

    if (gray > treshold)
        return 1.0;

    return sin(a * gl_FragCoord.x + b * gl_FragCoord.y) > c ? 1 : 0;
}

void main()
{
    float gray = grayscale(texture(color_texture, texture_coord));

    float hash1 = hash(-100, 100, 0.99, gray, 0.11);
    float hash2 = hash(70, 70, 0.01, gray, 0.63);
    float hash3 = hash(-400, 400, 0.01, gray, 0.45);

    out_color1 = vec4(hash1);
    out_color2 = vec4(hash2);
    out_color3 = vec4(hash3);

    out_color_final = clamp(vec4(hash1 * hash2 * hash3), 0.0, 1.0);
}

