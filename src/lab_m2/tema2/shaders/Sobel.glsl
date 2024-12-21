#version 330

in vec2 texture_coord;

uniform sampler2D color_texture;
uniform ivec2 screenSize;

out vec4 out_color;

float grayscale(vec4 color)
{
    return 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
}

void main()
{
    float threshold = 0.3;
    float gx = 0.0;
    float gy = 0.0;
    float sobel_x[9] = float[9](1.0, 0.0, -1.0, 2.0, 0.0, -2.0, 1.0, 0.0, -1.0);
    float sobel_y[9] = float[9](1.0, 2.0, 1.0, 0.0, 0.0, 0.0, -1.0, -2.0, -1.0);

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            int index = (i + 1) * 3 + (j + 1);

            gx += sobel_x[index] * grayscale(texture(color_texture, texture_coord + vec2(i, j) / screenSize));
            gy += sobel_y[index] * grayscale(texture(color_texture, texture_coord + vec2(i, j) / screenSize));
        }
    }

    float g = sqrt(gx * gx + gy * gy);

    out_color = g < threshold ? vec4(1.0) : vec4(0.0);
}

