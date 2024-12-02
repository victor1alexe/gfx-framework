#version 430

#define PI 3.14159265359

#define NR_EMIT_PER_INSTANCE 4

#define WATERFALL 0
#define SINKHOLE 1
#define HILL 2

#define SINKHOLE_RADIUS 1.0f
#define WATERFALL_RADIUS 0.1f
#define HEIGHT_MAX 0.3f
#define GLOBAL_NOISE_FACTOR 5.0f

layout(points) in;
layout(triangle_strip, max_vertices = 256) out;

// Input
layout(location = 0) in int instance[1];

// Uniforms
uniform mat4 View;
uniform mat4 Projection;
uniform sampler2D heightmap;

// Output
layout(location = 0) out vec2 tex_coords_out;
layout(location = 1) out vec3 pos_out;
layout(location = 2) out vec3 normal_out;

struct v_data {
    vec3 position;
    vec3 normal;
    vec2 uv;
};
v_data v_quad[4];

// Define offset activation for each vertex of the quad
// vec2(0, 0) -> bottom left
// vec2(0, 1) -> top left
// vec2(1, 0) -> bottom right
// vec2(1, 1) -> top right
vec2 v_pos_offset_enable[4] = {vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1)};

vec3 world_center = vec3(0, 0, 0);
vec2 terrain_size = vec2(8, 8);
vec3 terrain_first_v_pos = world_center - vec3(terrain_size.x / 2.0f, 0, terrain_size.y / 2.0f);

vec2 resolution = vec2(512, 512);

int v_input_row = instance[0] / int(resolution.y); // row of the first vertex of the quad
int v_input_col = instance[0] % int(resolution.x); // column of the first vertex of the quad

vec2 v_pos_step = terrain_size / resolution;
vec2 uv_step = 1.0f / resolution;

// ############## BEZIER WATERFALL ##############

vec3 control_p0 = vec3(-2.0 * SINKHOLE_RADIUS, 0.5, 0.0);
vec3 control_p1 = vec3(-1.2 * SINKHOLE_RADIUS, 0.05, 0.0);
vec3 control_p2 = vec3(-1.0 * SINKHOLE_RADIUS, -0.03, 0.0);
vec3 control_p3 = vec3(world_center.x, -0.02, 0.0);

vec3 bezier(float t)
{
    return  control_p0 * pow((1 - t), 3) +
            control_p1 * 3 * t * pow((1 - t), 2) +
            control_p2 * 3 * pow(t, 2) * (1 - t) +
            control_p3 * pow(t, 3);
}

struct bezier_closes_point_info {
    float t;
    float d_to_quad_v;
};

bezier_closes_point_info closest_point_on_bezier(vec3 point)
{
    bezier_closes_point_info info;
    info.t = 0.0f;
    info.d_to_quad_v = 1000000.0f;

    for (float t = 0.0f; t <= 1.0f; t += 0.01f)
    {
        vec3 curve_point = bezier(t);
        float distance = distance(vec3(point.x, 0, point.z), vec3(curve_point.x, 0, curve_point.z));

        if (distance < info.d_to_quad_v)
        {
            info.t = t;
            info.d_to_quad_v = distance;
        }
    }

    return info;
}

// ############## BASE HEIGHT ##############

struct v_base_height_info {
    int type;
    float height;
};

v_base_height_info get_v_base_height(vec3 point)
{
    v_base_height_info info;
    info.height = 0.0f;
    info.type = HILL;

    float distance_to_center = distance(vec3(point.x, 0, point.z), vec3(world_center.x, 0, world_center.z));
    distance_to_center /= SINKHOLE_RADIUS;

    if (distance_to_center < 1.0f)
    {
        info.type = SINKHOLE;
        info.height = HEIGHT_MAX * pow(distance_to_center, 6) / 2.0f;
    } else {
        info.height = HEIGHT_MAX * (1.0f - pow(2 - distance_to_center, 2) / 2);
    }

    return info;
}

// ############## NORMAL CALCULATION ##############

/*
 * După toate aceste operații, forma terenului este gata, dar mai trebuie calculate normalele pentru a se putea implementa iluminare.
 * După ce avem mesh-ul, putem calcula cu ușurință normalele prin calcularea vectorilor de direcție dintre vertecși adiacenți pe axele X și Z,
 * iar apoi realizarea produsului vectorial dintre acești vectori și normalizarea rezultatului.

  * Pentru extremitățile mesh-ului este posibil să nu avem vecini ai vertecșilor în fiecare direcție.
  * În cazul în care vertecșii nu au vecini pe anumite direcții, se vor folosi doar vecinii pe care îi au pentru a calcula vectorii de direcție.
*/
vec3 compute_normal()
{
    vec3 v0 = v_quad[0].position;
    vec3 v1 = v_quad[1].position;
    vec3 v2 = v_quad[2].position;
    vec3 v3 = v_quad[3].position;

    vec3 normal = normalize(cross(v1 - v0, v2 - v0));

    return normal;
}

/*
 * Normalize the position from [-terrain_size / 2, terrain_size / 2] to [0, 1]
 */
vec2 normalize_pos(vec2 pos)
{
    return (pos - terrain_first_v_pos.xz) / terrain_size;
}

float lerp(float a, float b, float t)
{
    return (1 - t) * a + t * b;
}

float waterfall(float y_b_closest, float h, float d_bezier, float r_waterfall)
{
    return lerp(y_b_closest, h, 1 - sin(PI / 2 - PI / 2 * clamp(d_bezier / r_waterfall, 0.0f, 1.0f)));
}

// ############## MAIN ##############

void main()
{
    vec3 v_quad_first_pos = terrain_first_v_pos + vec3(v_pos_step.x * v_input_col, 0, v_pos_step.y * v_input_row);

    for (int i = 0; i < NR_EMIT_PER_INSTANCE; i++) {
        vec3 offset = vec3(v_pos_step.x * v_pos_offset_enable[i].x, 0, v_pos_step.y * v_pos_offset_enable[i].y);

        v_quad[i].position = v_quad_first_pos + offset;
        v_quad[i].uv = normalize_pos(vec2(v_quad[i].position.x, v_quad[i].position.z));

        // Base height
        v_base_height_info v_height_info = get_v_base_height(v_quad[i].position);
        v_quad[i].position.y = v_height_info.height;

        // Apply noise
        if (v_height_info.type == HILL) {
            float v_d_to_center = distance(vec3(v_quad[i].position.x, 0, v_quad[i].position.z), vec3(world_center.x, 0, world_center.z));
            float v_d_to_sinkhole_rim = v_d_to_center - SINKHOLE_RADIUS;

            float noise_factor = GLOBAL_NOISE_FACTOR;
            noise_factor *= v_d_to_sinkhole_rim / (terrain_size.x / 2.0f);
    
            v_quad[i].position.y += noise_factor * texture(heightmap, v_quad[i].uv).r;
        }

        // Waterfall curve
        bezier_closes_point_info bezier_info = closest_point_on_bezier(v_quad[i].position);
        v_quad[i].position.y += waterfall(bezier(bezier_info.t).y, v_quad[i].position.y, bezier_info.d_to_quad_v, WATERFALL_RADIUS);
    }

    vec3 quad_normal = compute_normal();

    // Emit the vertices of the quad
    for (int i = 0; i < NR_EMIT_PER_INSTANCE; i++) {
        pos_out = v_quad[i].position;
        normal_out = quad_normal;
        tex_coords_out = v_quad[i].uv;
        gl_Position = Projection * View * vec4(v_quad[i].position, 1.0f);
        EmitVertex();
    }

    EndPrimitive();
}
