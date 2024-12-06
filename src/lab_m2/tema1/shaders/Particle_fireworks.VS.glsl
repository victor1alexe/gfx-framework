#version 430

#define PI 3.14159265359
#define SINKHOLE_RADIUS 1.0f
#define WATERFALL_RADIUS 0.1f
#define WATERFALL 0
#define SINKHOLE 1
#define HILL 2
#define HEIGHT_MAX 0.3f
#define GLOBAL_NOISE_FACTOR 5.0f

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform vec3 generator_position;
uniform float deltaTime;
uniform float time;

uniform sampler2D heightmap;

out float vert_lifetime;
out float vert_iLifetime;
out vec3 vert_normal;

struct Particle
{
    vec4 position;
    vec4 speed;
    vec4 iposition;
    vec4 ispeed;
    float delay;
    float iDelay;
    float lifetime;
    float iLifetime;
    float t_bezier;
    float initial_t_bezier;
    float t_step;
    float curve_offset;
};

vec3 world_center = vec3(0.0, 0.0, 0.0);
vec2 terrain_size = vec2(8, 8);
vec3 terrain_first_v_pos = world_center - vec3(terrain_size.x / 2.0f, 0, terrain_size.y / 2.0f);
vec2 resolution = vec2(64, 64);

vec3 control_p0 = vec3(-2.0 * SINKHOLE_RADIUS, 0.5, 0.0);
vec3 control_p1 = vec3(-1.2 * SINKHOLE_RADIUS, 0.05, 0.0);
vec3 control_p2 = vec3(-1.0 * SINKHOLE_RADIUS, -0.03, 0.0);
vec3 control_p3 = vec3(world_center.x, -0.02, 0.0);

layout(std430, binding = 0) buffer particles {
    Particle data[];
};

struct bezier_closes_point_info {
    float t;
    float d_to_quad_v;
};

vec3 bezier(float t)
{
    return  control_p0 * pow((1 - t), 3) +
            control_p1 * 3 * t * pow((1 - t), 2) +
            control_p2 * 3 * pow(t, 2) * (1 - t) +
            control_p3 * pow(t, 3);
}

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

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

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

void main()
{
    float t = data[gl_VertexID].t_bezier;
    float t_step = data[gl_VertexID].t_step;
    float curve_offset = data[gl_VertexID].curve_offset;

    float dt = deltaTime * t_step;
    vec3 pos = bezier(t);

    t += dt;
    if (t > 1.0f)
        t = 0.0f;

    v_base_height_info v_height_info = get_v_base_height(pos);
    pos.y += v_height_info.height;

    if (v_height_info.type == HILL) {
        // float v_d_to_center = distance(vec3(v_quad[i].position.x, 0, v_quad[i].position.z), vec3(world_center.x, 0, world_center.z));
        // float v_d_to_sinkhole_rim = v_d_to_center - SINKHOLE_RADIUS;

        // float noise_factor = GLOBAL_NOISE_FACTOR;
        // noise_factor *= v_d_to_sinkhole_rim / (terrain_size.x / 2.0f);

        // v_quad[i].position.y += noise_factor * texture(heightmap, v_quad[i].uv).r;

        float v_d_to_center = distance(vec3(pos.x, 0, pos.z), vec3(world_center.x, 0, world_center.z));
        float v_d_to_sinkhole_rim = v_d_to_center - SINKHOLE_RADIUS;

        float noise_factor = GLOBAL_NOISE_FACTOR;
        noise_factor *= v_d_to_sinkhole_rim / (terrain_size.x / 2.0f);

        vec2 uv = normalize_pos(vec2(pos.x, pos.z));
        pos.y += noise_factor * texture(heightmap, uv).r;
    }

    pos.y += 0.2f;
    pos.z += curve_offset;
    pos.y += rand(vec2(curve_offset, time)) * 0.1f;
    pos.y -= 0.6f;

    data[gl_VertexID].position.xyz = vec3(0, 0, 0);
    data[gl_VertexID].speed.xyz = vec3(0, 0, 0);

    data[gl_VertexID].t_bezier = t;

    vert_normal = v_normal;

    gl_Position = Model * vec4(pos + generator_position, 1);
}
