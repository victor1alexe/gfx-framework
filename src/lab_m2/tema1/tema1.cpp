#include "lab_m2/tema1/tema1.h"

#define SINKHOLE_RADIUS 1.0f

using namespace std;
using namespace m2;

struct ParticleTema1
{
    glm::vec4 position;
    glm::vec4 speed;
    glm::vec4 initialPos;
    glm::vec4 initialSpeed;
    float delay;
    float initialDelay;
    float lifetime;
    float initialLifetime;
    float t_bezier;
    float initial_t_bezier;
    float t_step;
    float curve_offset;

    ParticleTema1() {}

    ParticleTema1(const glm::vec4 &pos, const glm::vec4 &speed)
    {
        SetInitial(pos, speed);
    }

    void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed,
        float delay = 0, float lifetime = 0, float t_bezier = 0, float t_step = 0, float curve_offset = 0)
    {
        position = pos;
        initialPos = pos;

        this->speed = speed;
        initialSpeed = speed;

        this->delay = delay;
        initialDelay = delay;

        this->lifetime = lifetime;
        initialLifetime = lifetime;

        this->t_bezier = t_bezier;
        initial_t_bezier = t_bezier;

        this->t_step = t_step;
        this->curve_offset = curve_offset;
    }
};

ParticleEffect<ParticleTema1> *particleEffectTema1;

inline float Rand01()
{
    return rand() / static_cast<float>(RAND_MAX);
}

Tema1::Tema1()
{
    outputType = 0;
    no_of_instances = terrain_resolution_x * terrain_resolution_z;
}

Tema1::~Tema1() {}

void Tema1::LoadShader(const std::string &name)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".FS.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}

void Tema1::CreateFramebuffer(int width, int height)
{
    // TODO(student): In this method, use the attributes
    // 'framebuffer_object', 'color_texture'
    // declared in lab6.h

    // TODO(student): Generate and bind the framebuffer
    glGenFramebuffers(1, &particles_framebuffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, particles_framebuffer_object);



    // TODO(student): Generate and bind the color texture
    glGenTextures(1, &particles_color_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, particles_color_texture);



    // TODO(student): Initialize the color textures


    if (particles_color_texture) {
        //cubemap params
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (GLEW_EXT_texture_filter_anisotropic) {
            float maxAnisotropy;

            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Bind the color textures to the framebuffer as a color attachments
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, particles_color_texture, 0);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        std::vector<GLenum> draw_textures;
        draw_textures.push_back(GL_COLOR_ATTACHMENT0);
        glDrawBuffers(draw_textures.size(), &draw_textures[0]);

    }

    // TODO(student): Generate and bind the depth texture
    glGenTextures(1, &particles_depth_texture);
    glBindTexture(GL_TEXTURE_2D, particles_depth_texture);


    // TODO(student): Initialize the depth textures
    if (particles_depth_texture) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Bind the depth textures to the framebuffer as a depth attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, particles_depth_texture, 0);
    }


    if (particles_depth_texture) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, particles_depth_texture, 0);
    }

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Tema1::UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z)
{
    int width, height, chn;

    unsigned char *data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, STBI_rgb_alpha);
    unsigned char *data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, STBI_rgb_alpha);
    unsigned char *data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, STBI_rgb_alpha);
    unsigned char *data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, STBI_rgb_alpha);
    unsigned char *data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, STBI_rgb_alpha);
    unsigned char *data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, STBI_rgb_alpha);

    unsigned int textureID = 0;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAnisotropy;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_neg_z);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Free memory
    SAFE_FREE(data_pos_x);
    SAFE_FREE(data_pos_y);
    SAFE_FREE(data_pos_z);
    SAFE_FREE(data_neg_x);
    SAFE_FREE(data_neg_y);
    SAFE_FREE(data_neg_z);

    return textureID;
}

void Tema1::RenderMeshCustomView(Mesh * mesh, Shader * shader, const glm::mat4 & modelMatrix, glm::mat4 & view)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(GetSceneCamera()->GetProjectionMatrix()));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}

void Tema1::RenderSkybox(GLuint skyboxTextureID, glm::mat4 &view)
{
    auto shader = shaders["Skybox"];
    shader->Use();

    int model_location = shader->GetUniformLocation("Model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

    int loc_view_matrix = shader->GetUniformLocation("View");
    // if (view == glm::mat4(0))
    //     view = glm::mat4(glm::mat3(GetSceneCamera()->GetViewMatrix()));
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(view));

    int loc_projection_matrix = shader->GetUniformLocation("Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(GetSceneCamera()->GetProjectionMatrix()));

    int loc_texture = shader->GetUniformLocation("skybox");
    glUniform1i(loc_texture, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

    glDepthFunc(GL_LEQUAL);
    meshes["skybox"]->Render();
    glDepthFunc(GL_LESS);
}

void Tema1::ResetParticlesFire(float radius)
{
    unsigned int nrParticles = 4096;

    particleEffectTema1 = new ParticleEffect<ParticleTema1>();
    particleEffectTema1->Generate(nrParticles, true);

    auto particleSSBO = particleEffectTema1->GetParticleBuffer();
    ParticleTema1* data = const_cast<ParticleTema1*>(particleSSBO->GetBuffer());

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        glm::vec3 pos = glm::vec3(0, 0, 0);
        glm::vec3 speed = glm::vec3(0, 0, 0);
        float t = i / static_cast<float>(nrParticles);
        // std::cout << "particle " << i << " t: " << t << std::endl;
        float t_step = Rand01() * 0.3f + 0.1f;
        float curve_offset = Rand01() * 0.5f - 0.25f;
        float lifetime = 1;
        data[i].SetInitial(glm::vec4(pos, 1), glm::vec4(speed, 0), 0, lifetime, t, t_step, curve_offset);
    }

    particleSSBO->SetBufferData(data);
}

glm::vec3 Tema1::CalculateBezier(float t)
{
    return  control_p0 * static_cast<float>(pow((1 - t), 3)) +
            control_p1 * static_cast<float>(3 * t * pow((1 - t), 2)) +
            control_p2 * static_cast<float>(3 * pow(t, 2) * (1 - t)) +
            control_p3 * static_cast<float>(pow(t, 3));
}

void Tema1::ResetParticlesRainSnow(int xSize, int ySize, int zSize)
{
    unsigned int nrParticles = 100;

    particleEffectTema1 = new ParticleEffect<ParticleTema1>();
    particleEffectTema1->Generate(nrParticles, true);

    auto particleSSBO = particleEffectTema1->GetParticleBuffer();
    ParticleTema1* data = const_cast<ParticleTema1*>(particleSSBO->GetBuffer());


    int xhSize = xSize / 2;
    int yhSize = ySize / 2;
    int zhSize = zSize / 2;

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        glm::vec4 pos(1);
        pos.x = (rand() % xSize - xhSize) / 10.0f;
        pos.y = (rand() % ySize - yhSize) / 10.0f;
        pos.z = (rand() % zSize - zhSize) / 10.0f;

        glm::vec4 speed(0);
        speed.x = - (rand() % 20 - 10) / 10.0f;
        speed.z = - (rand() % 20 - 10) / 10.0f;
        speed.y = - (rand() % 2 + 2.0f);

        float delay = (rand() % 100 / 100.0f) * 3.0f;

        data[i].SetInitial(pos, speed, delay);
    }

    particleSSBO->SetBufferData(data);
}

Texture2D* Tema1::CreateRandomTexture(unsigned int width, unsigned int height)
{
    GLuint textureID = 0;
    unsigned int channels = 3;
    unsigned int size = width * height * channels;
    unsigned char* data = new unsigned char[size];

    for (unsigned int i = 0; i < size; i += 3) {
        data[i] = 255 * static_cast<unsigned char>(Rand01());
        data[i + 1] = 255 * static_cast<unsigned char>(Rand01());
        data[i + 2] = 255 * static_cast<unsigned char>(Rand01());
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    if (GLEW_EXT_texture_filter_anisotropic) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    CheckOpenGLError();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    Texture2D* texture = new Texture2D();
    texture->Init(textureID, width, height, channels);

    SAFE_FREE_ARRAY(data);

    return texture;
}

void Tema1::Init()
{
    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, 2, 3.5), glm::quat(glm::vec3(-20 * TO_RADIANS, 0, 0)));
    camera->Update();

    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "ground.jpg");
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "terrain_textures"), "heightmap3.png", "heightmap");
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1"), "droplet.png", "droplet");

    control_p0 = glm::vec3(-2.0 * SINKHOLE_RADIUS, 0.5, 0.0);
    control_p1 = glm::vec3(-1.2 * SINKHOLE_RADIUS, 0.05, 0.0);
    control_p2 = glm::vec3(-1.0 * SINKHOLE_RADIUS, -0.03, 0.0);
    control_p3 = glm::vec3(world_center.x, -0.02, 0.0);

    ResetParticlesFire(0.25);

    generator_position = glm::vec3(0, 0, 0);
    offset = 0.05f;

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Create skybox cube
    {
        Mesh* mesh = new Mesh("skybox");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Load butterfly mesh
    {
        Mesh *mesh = new Mesh("butterfly");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "custom_models"), "MONARCH.OBJ");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // std::string texture_path = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "cubemap_night");
    // skyboxTextureID = UploadCubeMapTexture(
    //     PATH_JOIN(texture_path, "pos_x.png"),
    //     PATH_JOIN(texture_path, "pos_y.png"),
    //     PATH_JOIN(texture_path, "pos_z.png"),
    //     PATH_JOIN(texture_path, "neg_x.png"),
    //     PATH_JOIN(texture_path, "neg_y.png"),
    //     PATH_JOIN(texture_path, "neg_z.png")
    // );

    std::string texture_path = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "cubemap_custom");
    skyboxTextureID = UploadCubeMapTexture(
        PATH_JOIN(texture_path, "pos_x.png"),
        PATH_JOIN(texture_path, "pos_y.png"),
        PATH_JOIN(texture_path, "pos_z.png"),
        PATH_JOIN(texture_path, "neg_x.png"),
        PATH_JOIN(texture_path, "neg_y.png"),
        PATH_JOIN(texture_path, "neg_z.png")
    );

    // std::cout << "Skybox texture ID: " << skyboxTextureID << std::endl;
    // Load skybox shader
    {
        Shader *shader = new Shader("Skybox");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Skybox.vs"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Skybox.fs"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a single vertex mesh to be used with drawElementsInstanced
    {
        glm::vec3 pos(-terrain_size_x / 2, 0, -terrain_size_z / 2);
        glm::vec3 normal(0, 1, 0);
        glm::vec3 color(0, 1, 1);

        vector<VertexFormat> vertices
        {
            VertexFormat(pos, color, normal),
        };

        vector<unsigned int> indices =
        {
            0,
        };

        meshes["point"] = new Mesh("point");
        meshes["point"]->InitFromData(vertices, indices);
        meshes["point"]->SetDrawMode(GL_POINTS);
    }

    // Create a shader program for creating a CUBEMAP
    {
        Shader *shader = new Shader("Framebuffer");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Framebuffer.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Framebuffer.GS.glsl"), GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Framebuffer.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Load reflection shader
    {
        Shader *shader = new Shader("Reflection");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Reflexion.VS"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Reflexion.FS"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Load terrain shaders
    {
        Shader *shader = new Shader("TerrainShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Terrain.vs"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Terrain.gs"), GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Terrain.fs"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Load particle shader
    {
        Shader *shader = new Shader("RainSnow");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Particle_fireworks.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Particle.GS.glsl"), GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "shaders", "Particle_simple.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    LoadShader("Render2Texture");
    LoadShader("Composition");
    LoadShader("LightPass");

    auto resolution = window->GetResolution();

    geometryBuffer = new FrameBuffer();
    geometryBuffer->Generate(resolution.x, resolution.y, 3);
    
    reflexionGeometryBuffer = new FrameBuffer();
    reflexionGeometryBuffer->Generate(resolution.x, resolution.y, 3);

    lightBuffer = new FrameBuffer();
    lightBuffer->Generate(resolution.x, resolution.y, 1, false);
    
    reflexionLightAccumulationBuffer = new FrameBuffer();
    reflexionLightAccumulationBuffer->Generate(resolution.x, resolution.y, 1, false);

    finalReflectionBuffer = new FrameBuffer();
    finalReflectionBuffer->Generate(resolution.x, resolution.y, 1, false);

    for (int i = 0; i < 20; ++i) {
        LightInfoTema1 lightInfo;

        lightInfo.position = glm::vec3(Rand01() * 8 - 4, 2.5, Rand01() * 8 - 4);
        lightInfo.color = glm::vec3(Rand01(), Rand01(), Rand01());
        lightInfo.radius = 3;

        // if (i == 0)
        //     lightInfo.position = glm::vec3(0, 1, 0);

        lights.push_back(lightInfo);
    }
}

void Tema1::FrameStart() {}

void Tema1::Update(float deltaTimeSeconds)
{
    ClearScreen();

    for (auto& l : lights) {
        glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), deltaTimeSeconds, glm::vec3(0, 1, 0));
        l.position = rotateMatrix * glm::vec4(l.position, 1.0f);
    }

    Mesh *mesh = nullptr;
    Shader *shader = nullptr;
    glm::vec3 ambientLight(0.2f);
    glm::mat4 model = glm::mat4(1);
    glm::mat4 view = GetSceneCamera()->GetViewMatrix();
    glm::mat4 projection = GetSceneCamera()->GetProjectionMatrix();

    glm::vec3 camera_initial_pos = GetSceneCamera()->m_transform->GetWorldPosition();
    glm::vec3 camera_initial_forward = GetSceneCamera()->m_transform->GetWorldRotation() * glm::vec3(0, 0, -1);
    glm::mat4 view_initial = GetSceneCamera()->GetViewMatrix();

    glm::vec3 camera_reflected_pos = glm::vec3(camera_initial_pos.x, -camera_initial_pos.y, camera_initial_pos.z);
    glm::vec3 camera_reflected_forward = glm::vec3(camera_initial_forward.x, -camera_initial_forward.y, camera_initial_forward.z);

    if (camera_initial_forward.y > 0)
        camera_reflected_forward.y = camera_initial_forward.y;

    glm::mat4 reflected_view = glm::lookAt(camera_reflected_pos, camera_reflected_pos + camera_reflected_forward, glm::vec3(0, 1, 0));

    // ------------------------------------------------------------------------
    // Deferred rendering pass
    {
        // Particle reflection frame buffer
        // {
        //     glBindFramebuffer(GL_FRAMEBUFFER, particles_framebuffer_object);
        //     // Set the clear color for the color buffer
        //     glClearColor(0, 0, 0, 1);
        //     // Clears the color buffer (using the previously set color) and depth buffer
        //     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //     glViewport(0, 0, 1024, 1024);

        //     shader = shaders["Framebuffer"];
        //     shader->Use();

        // }

        // Reflexion Geometry pass
        {
            reflexionGeometryBuffer->Bind();

            // Render particles
            shader = shaders["RainSnow"];
            glUseProgram(shader->program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("droplet")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture"), 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("heightmap")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "heightmap"), 1);
            // Send the cubemap for reflections
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
            glUniform1i(glGetUniformLocation(shader->program, "texture_reflection"), 2);
            glUniform3fv(glGetUniformLocation(shader->program, "generator_position"), 1, glm::value_ptr(generator_position));
            glUniform1f(glGetUniformLocation(shader->program, "deltaTime"), deltaTimeSeconds);
            glUniform1f(glGetUniformLocation(shader->program, "offset"), offset);
            particleEffectTema1->RenderCustomView(GetSceneCamera(), shader, no_of_instances, reflected_view);

            // Render light spheres
            // mesh = meshes["sphere"];
            mesh = meshes["butterfly"];
            shader = shaders["Render2Texture"];
            glUseProgram(shader->program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("default.png")->GetTextureID());
            for (auto &l : lights) {
                model = glm::mat4(1);
                model = glm::translate(glm::mat4(1), l.position);
                // model = glm::scale(model, glm::vec3(0.2f));
                model = glm::scale(model, glm::vec3(4.0f));
                RenderMeshCustomView(mesh, shader, model, reflected_view);
            }

            mesh = meshes["point"];
            shader = shaders["TerrainShader"];
            glUseProgram(shader->program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("heightmap")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "heightmap"), 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("ground.jpg")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture_terrain"), 1);
            glUniformMatrix4fv(glGetUniformLocation(shader->program, "Model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shader->program, "View"), 1, GL_FALSE, glm::value_ptr(reflected_view)); // change to reflected view
            glUniformMatrix4fv(glGetUniformLocation(shader->program, "Projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(mesh->GetBuffers()->m_VAO);
            glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, no_of_instances);
            glBindVertexArray(0);

            // Render skybox
            glm::mat4 skybox_view = glm::mat4(glm::mat3(reflected_view));
            RenderSkybox(skyboxTextureID, skybox_view);
        }

        // Reflexion Light pass
        {
            glm::vec3 ambientLightReflexion(0.8f);
            reflexionLightAccumulationBuffer->SetClearColor(glm::vec4(ambientLightReflexion.x, ambientLightReflexion.y, ambientLightReflexion.z, 1.0f));
            reflexionLightAccumulationBuffer->Bind();
            shader = shaders["LightPass"];
            int texturePositionsLoc = shader->GetUniformLocation("texture_position");
            int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
            int loc_eyePosition = shader->GetUniformLocation("eye_position");
            auto resolution = window->GetResolution();
            int loc_resolution = shader->GetUniformLocation("resolution");
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendEquation(GL_FUNC_ADD);
            glBlendFunc(GL_ONE, GL_ONE);

            glUseProgram(shader->program);
            glUniform1i(texturePositionsLoc, 0);
            reflexionGeometryBuffer->BindTexture(0, GL_TEXTURE0);
            glUniform1i(textureNormalsLoc, 1);
            reflexionGeometryBuffer->BindTexture(1, GL_TEXTURE0 + 1);
            glUniform3fv(loc_eyePosition, 1, glm::value_ptr(camera_initial_pos));
            glUniform2i(loc_resolution, resolution.x, resolution.y);

            for (auto& lightInfo : lights) {
                model = glm::mat4(1);
                model = glm::translate(model, lightInfo.position);
                model = glm::scale(model, 2 * lightInfo.radius * glm::vec3(1.f, 1.f, 1.f));

                glUniform3fv(glGetUniformLocation(shader->program, "light_position"), 1, glm::value_ptr(lightInfo.position));
                glUniform3fv(glGetUniformLocation(shader->program, "light_color"), 1, glm::value_ptr(lightInfo.color));
                glUniform1f(glGetUniformLocation(shader->program, "light_radius"), lightInfo.radius);
                glUniformMatrix4fv(glGetUniformLocation(shader->program, "Model"), 1, GL_FALSE, glm::value_ptr(model));
                glUniformMatrix4fv(glGetUniformLocation(shader->program, "View"), 1, GL_FALSE, glm::value_ptr(reflected_view));
                glUniformMatrix4fv(glGetUniformLocation(shader->program, "Projection"), 1, GL_FALSE, glm::value_ptr(projection));

                meshes["sphere"]->Render();
            }

            glDisable(GL_CULL_FACE);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        // World Geometry pass
        {
            geometryBuffer->Bind();

            // Render particles
            shader = shaders["RainSnow"];
            glUseProgram(shader->program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("droplet")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture"), 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("heightmap")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "heightmap"), 1);

            // Send the cubemap for reflections
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
            glUniform1i(glGetUniformLocation(shader->program, "texture_reflection"), 2);

            glUniform3fv(glGetUniformLocation(shader->program, "generator_position"), 1, glm::value_ptr(generator_position));
            glUniform1f(glGetUniformLocation(shader->program, "deltaTime"), deltaTimeSeconds);
            glUniform1f(glGetUniformLocation(shader->program, "offset"), offset);
            particleEffectTema1->Render(GetSceneCamera(), shader);

            // Render light spheres
            // mesh = meshes["sphere"];
            mesh = meshes["butterfly"];
            shader = shaders["Render2Texture"];
            glUseProgram(shader->program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("default.png")->GetTextureID());
            for (auto &l : lights) {
                model = glm::mat4(1);
                model = glm::translate(glm::mat4(1), l.position);
                // model = glm::scale(model, glm::vec3(0.2f));
                model = glm::scale(model, glm::vec3(4.0f));
                RenderMesh(mesh, shader, model);
            }

            // Render terrain
            mesh = meshes["point"];
            shader = shaders["TerrainShader"];
            glUseProgram(shader->program);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("heightmap")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "heightmap"), 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("ground.jpg")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture_terrain"), 1);
            glUniformMatrix4fv(glGetUniformLocation(shader->program, "Model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shader->program, "View"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shader->program, "Projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(mesh->GetBuffers()->m_VAO);
            glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, no_of_instances);
            glBindVertexArray(0);

            // Render skybox
            glm::mat4 skybox_view = glm::mat4(glm::mat3(GetSceneCamera()->GetViewMatrix()));
            RenderSkybox(skyboxTextureID, skybox_view);

            // Render lake
            mesh = meshes["plane"];
            shader = shaders["Reflection"];
            model = glm::mat4(1);
            model *= glm::scale(glm::mat4(1), glm::vec3(0.045f));
            glUseProgram(shader->program);
            glUniform1i(shader->GetUniformLocation("texture_color"), 0);
            reflexionGeometryBuffer->BindTexture(2, GL_TEXTURE0);
            glUniform1i(shader->GetUniformLocation("texture_light"), 1);
            reflexionLightAccumulationBuffer->BindTexture(0, GL_TEXTURE0 + 1);
            glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(shader->GetUniformLocation("View"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(shader->GetUniformLocation("ViewReflected"), 1, GL_FALSE, glm::value_ptr(reflected_view));
            glUniformMatrix4fv(shader->GetUniformLocation("Projection"), 1, GL_FALSE, glm::value_ptr(projection));

            mesh->Render();
        }
    }

    // ------------------------------------------------------------------------
    // Lighting pass
    {
        lightBuffer->SetClearColor(glm::vec4(ambientLight.x, ambientLight.y, ambientLight.z, 1.0f));
        lightBuffer->Bind();
        shader = shaders["LightPass"];
        int texturePositionsLoc = shader->GetUniformLocation("texture_position");
        int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
        auto camera = GetSceneCamera();
        glm::vec3 cameraPos = camera->m_transform->GetWorldPosition();
        int loc_eyePosition = shader->GetUniformLocation("eye_position");
        auto resolution = window->GetResolution();
        int loc_resolution = shader->GetUniformLocation("resolution");
        glClearColor(0, 0, 0, 1);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        glUseProgram(shader->program);
        glUniform1i(texturePositionsLoc, 0);
        geometryBuffer->BindTexture(0, GL_TEXTURE0);
        glUniform1i(textureNormalsLoc, 1);
        geometryBuffer->BindTexture(1, GL_TEXTURE0 + 1);
        glUniform3fv(loc_eyePosition, 1, glm::value_ptr(cameraPos));
        glUniform2i(loc_resolution, resolution.x, resolution.y);
        glUniform1i(glGetUniformLocation(shader->program, "is_reflection"), 0);
        glUniformMatrix4fv(glGetUniformLocation(shader->program, "ViewReflexion"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        for (auto& lightInfo : lights) {
            glUniform3fv(glGetUniformLocation(shader->program, "light_position"), 1, glm::value_ptr(lightInfo.position));
            glUniform3fv(glGetUniformLocation(shader->program, "light_color"), 1, glm::value_ptr(lightInfo.color));
            glUniform1f(glGetUniformLocation(shader->program, "light_radius"), lightInfo.radius);

            RenderMesh(meshes["sphere"], shader, lightInfo.position, 2 * lightInfo.radius * glm::vec3(1.f, 1.f, 1.f));
        }

        glDisable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    // ------------------------------------------------------------------------
    // Composition pass
    {
        FrameBuffer::BindDefault();
        shader = shaders["Composition"];
        int outputTypeLoc = shader->GetUniformLocation("output_type");
        int texturePositionsLoc = shader->GetUniformLocation("texture_position");
        int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
        int textureColorLoc = shader->GetUniformLocation("texture_color");
        int textureDepthLoc = shader->GetUniformLocation("texture_depth");
        int textureLightLoc = shader->GetUniformLocation("texture_light");

        glUseProgram(shader->program);
        glUniform1i(outputTypeLoc, outputType);

        glUniform1i(texturePositionsLoc, 1);
        geometryBuffer->BindTexture(0, GL_TEXTURE0 + 1);

        glUniform1i(textureNormalsLoc, 2);
        geometryBuffer->BindTexture(1, GL_TEXTURE0 + 2);

        glUniform1i(textureColorLoc, 3);
        geometryBuffer->BindTexture(2, GL_TEXTURE0 + 3);

        glUniform1i(textureDepthLoc, 4);
        geometryBuffer->BindDepthTexture(GL_TEXTURE0 + 4);

        glUniform1i(textureLightLoc, 5);
        lightBuffer->BindTexture(0, GL_TEXTURE0 + 5);

        RenderMesh(meshes["quad"], shader, glm::vec3(0, 0, 0));
    }
}

void Tema1::FrameEnd()
{
    // DrawCoordinateSystem();
}

void Tema1::OnInputUpdate(float deltaTime, int mods) {}

void Tema1::OnKeyPress(int key, int mods)
{
    int index = key - GLFW_KEY_0;

    // Toggle render mode
    if (index >= 0 && index <= 9) {
        outputType = index;
    }

    // Toggle polygon mode
    if (key == GLFW_KEY_F) {
        switch (polygon_mode) {
        case GL_FILL:
            polygon_mode = GL_LINE;
            break;
        case GL_LINE:
            polygon_mode = GL_FILL;
            break;
        default:
            break;
        }
    }
}

void Tema1::OnKeyRelease(int key, int mods) {}

void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {}

void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

void Tema1::OnWindowResize(int width, int height)
{
    // Treat window resize event
    geometryBuffer->Resize(width, height, 32);
    lightBuffer->Resize(width, height, 32);
}
