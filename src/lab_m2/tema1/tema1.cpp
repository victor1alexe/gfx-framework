#include "lab_m2/tema1/tema1.h"

#include <vector>
#include <iostream>
#include <limits>

#include "stb/stb_image.h" // Use stb_image to load textures

#define SINKHOLE_RADIUS 1.0f

using namespace std;
using namespace m2;

struct Particle
{
    glm::vec4 position;
    glm::vec4 speed;
    glm::vec4 initialPos;
    glm::vec4 initialSpeed;
    float delay;
    float initialDelay;
    float lifetime;
    float initialLifetime;

    Particle() {}

    Particle(const glm::vec4 &pos, const glm::vec4 &speed)
    {
        SetInitial(pos, speed);
    }

    void SetInitial(const glm::vec4 &pos, const glm::vec4 &speed,
        float delay = 0, float lifetime = 0)
    {
        position = pos;
        initialPos = pos;

        this->speed = speed;
        initialSpeed = speed;

        this->delay = delay;
        initialDelay = delay;

        this->lifetime = lifetime;
        initialLifetime = lifetime;
    }
};


ParticleEffect<Particle> *particleEffectTema1;

// Generates a random value between 0 and 1.
inline float Rand01()
{
    return rand() / static_cast<float>(RAND_MAX);
}

Tema1::Tema1()
{
    outputType = 0;
    no_of_instances = terrain_resolution_x * terrain_resolution_z;
}

Tema1::~Tema1()
{
}

void Tema1::LoadShader(const std::string &name)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab5", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".FS.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}

unsigned int Tema1::UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z)
{
    int width, height, chn;

    unsigned char* data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, 0);

    unsigned int textureID = 0;
    // TODO(student): Create the texture
    glGenTextures(1, &textureID);

    // TODO(student): Bind the texture
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

    // TODO(student): Load texture information for each face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when completing the tasks." << std::endl;
    }

    // Free memory
    SAFE_FREE(data_pos_x);
    SAFE_FREE(data_pos_y);
    SAFE_FREE(data_pos_z);
    SAFE_FREE(data_neg_x);
    SAFE_FREE(data_neg_y);
    SAFE_FREE(data_neg_z);

    return textureID;
}

void Tema1::RenderSkybox(GLuint skyboxTextureID)
{
    auto shader = shaders["Skybox"];
    shader->Use();

    int model_location = shader->GetUniformLocation("Model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

    int loc_view_matrix = shader->GetUniformLocation("View");
    glm::mat4 view = glm::mat4(glm::mat3(GetSceneCamera()->GetViewMatrix()));
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
    unsigned int nrParticles = 100;

    particleEffectTema1 = new ParticleEffect<Particle>();
    particleEffectTema1->Generate(nrParticles, true);

    auto particleSSBO = particleEffectTema1->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        // glm::vec3 pos(1);
        // pos.x = (rand() % 100 - 50)/ 100.0f ;
        // pos.y = (rand() % 100 - 50)/ 100.0f;
        // pos.z = (rand() % 100 - 50)/ 100.0f;
        // pos = glm::normalize(pos) * radius ;

        // glm::vec3 speed(0);
        // speed = glm::normalize(glm::vec3(0, 5, 0) - glm::vec3(pos));
        // speed *= (rand() % 100 / 100.0f);
        // speed += glm::vec3(rand() % 5 / 5.0f, rand() % 5 / 5.0f, rand() % 5 / 5.0f) * 0.2f;

        // set the pos and speed to match the bezier curve
        float t = i / static_cast<float>(nrParticles);
        float t_step = Rand01() * 0.01f + 0.01f;
        float curve_offset = Rand01() * 0.1f;
        glm::vec3 pos = glm::vec3(t, t_step, curve_offset);
        glm::vec3 speed = glm::vec3(0, 0, 0);
        float lifetime = 1;

        data[i].SetInitial(glm::vec4 (pos, 1), glm::vec4 (speed, 0), 0, lifetime);
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
    unsigned int nrParticles = 5000;

    particleEffectTema1 = new ParticleEffect<Particle>();
    particleEffectTema1->Generate(nrParticles, true);

    auto particleSSBO = particleEffectTema1->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());


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

    // TODO(student): Generate random texture data
    for (unsigned int i = 0; i < size; i += 3) {
        data[i] = 255 * static_cast<unsigned char>(Rand01());
        data[i + 1] = 255 * static_cast<unsigned char>(Rand01());
        data[i + 2] = 255 * static_cast<unsigned char>(Rand01());
    }

    // TODO(student): Generate and bind the new texture ID
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    if (GLEW_EXT_texture_filter_anisotropic) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
    }
    // TODO(student): Set the texture parameters (MIN_FILTER, MAG_FILTER and WRAPPING MODE) using glTexParameteri
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    CheckOpenGLError();

    // Use glTexImage2D to set the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    // TODO(student): Generate texture mip-maps
    glGenerateMipmap(GL_TEXTURE_2D);

    CheckOpenGLError();

    // Save the texture into a wrapper Texture2D class for using easier later during rendering phase
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
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "rain.png");

    control_p0 = glm::vec3(-2.0 * SINKHOLE_RADIUS, 0.5, 0.0);
    control_p1 = glm::vec3(-1.2 * SINKHOLE_RADIUS, 0.05, 0.0);
    control_p2 = glm::vec3(-1.0 * SINKHOLE_RADIUS, -0.03, 0.0);
    control_p3 = glm::vec3(world_center.x, -0.02, 0.0);

    // ResetParticlesRainSnow(100, 10, 10);
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


    std::string texture_path = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "tema1", "cubemap_night");
    skyboxTextureID = UploadCubeMapTexture(
        PATH_JOIN(texture_path, "pos_x.png"),
        PATH_JOIN(texture_path, "pos_y.png"),
        PATH_JOIN(texture_path, "pos_z.png"),
        PATH_JOIN(texture_path, "neg_x.png"),
        PATH_JOIN(texture_path, "neg_y.png"),
        PATH_JOIN(texture_path, "neg_z.png")
    );

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

    frameBuffer = new FrameBuffer();
    frameBuffer->Generate(resolution.x, resolution.y, 3);
    //frameBuffer contains 3 textures (position, normal and color)

    lightBuffer = new FrameBuffer();
    lightBuffer->Generate(resolution.x, resolution.y, 1, false);
    //lightBuffer contains 1 texture (light accumulation)

    for (int i = 0; i < 10; ++i)
    {
        LightInfoTema1 lightInfo;

        lightInfo.position = glm::vec3(Rand01() * 6 - 3, 2.5, Rand01() * 6 - 3);
        lightInfo.color = glm::vec3(Rand01(), Rand01(), Rand01());
        lightInfo.radius = 2;

        lights.push_back(lightInfo);
    }
}

void Tema1::FrameStart()
{
}

void Tema1::RenderMeshInstanced(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix, int instances, const glm::vec3 &color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader
    glUseProgram(shader->program);

    // Bind model matrix
    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glPolygonMode(GL_FRONT_AND_BACK, wireframe);
    glLineWidth(3);

    // Draw the object instanced
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instances);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Tema1::Update(float deltaTimeSeconds)
{
    ClearScreen();

    for (auto& l : lights) {
        glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), deltaTimeSeconds, glm::vec3(0, 1, 0));
        l.position = rotateMatrix * glm::vec4(l.position, 1.0f);
    }

    // ------------------------------------------------------------------------
    // Deferred rendering pass
    {
        frameBuffer->Bind();

        // glLineWidth(3);
        // glEnable(GL_BLEND);
        // glDisable(GL_DEPTH_TEST);
        // glBlendFunc(GL_ONE, GL_ONE);
        // glBlendEquation(GL_FUNC_ADD);
        auto shader = shaders["RainSnow"];
        shader->Use();
        TextureManager::GetTexture("rain.png")->BindToTextureUnit(GL_TEXTURE0);
        glUniform3fv(glGetUniformLocation(shader->program, "generator_position"), 1, glm::value_ptr(generator_position));
        glUniform1f(glGetUniformLocation(shader->program, "deltaTime"), deltaTimeSeconds);
        glUniform1f(glGetUniformLocation(shader->program, "offset"), offset);
        // Also send the heightmap texture
        TextureManager::GetTexture("heightmap")->BindToTextureUnit(GL_TEXTURE1);
        glUniform1i(glGetUniformLocation(shader->program, "heightmap"), 1);

        // Send a value from [0, 1] to the shader to animate the particles
        // When it hits 1, reset the time
        normalized_time += deltaTimeSeconds;
        if (normalized_time > 1.0f) {
            normalized_time = 0.0f;
        }

        glUniform1f(glGetUniformLocation(shader->program, "time"), normalized_time);
        // std::cout << normalized_time << std::endl;
        particleEffectTema1->Render(GetSceneCamera(), shader);
        // glEnable(GL_DEPTH_TEST);
        // glDisable(GL_BLEND);

        // auto shader = shaders["Render2Texture"];

        // TextureManager::GetTexture("default.png")->BindToTextureUnit(GL_TEXTURE0);
        // for (auto &l : lights) {
        //     auto model = glm::translate(glm::mat4(1), l.position);
        //     model = glm::scale(model, glm::vec3(0.2f));
        //     RenderMesh(meshes["sphere"], shader, model);
        // }

        // TextureManager::GetTexture("ground.jpg")->BindToTextureUnit(GL_TEXTURE0);
        // RenderMesh(meshes["plane"], shader, glm::vec3(0, 0, 0), glm::vec3(0.5f));

        shader = shaders["TerrainShader"];
        shader->Use();
        TextureManager::GetTexture("heightmap")->BindToTextureUnit(GL_TEXTURE0);
        TextureManager::GetTexture("ground.jpg")->BindToTextureUnit(GL_TEXTURE1);

        int loc_heightmap = shader->GetUniformLocation("heightmap");
        glUniform1i(loc_heightmap, 0);

        int loc_texture = shader->GetUniformLocation("texture_terrain");
        glUniform1i(loc_texture, 1);

        RenderMeshInstanced(meshes["point"], shader, glm::mat4(1), no_of_instances);

        RenderSkybox(skyboxTextureID);
    }

    // ------------------------------------------------------------------------
    // Lighting pass
    {
        glm::vec3 ambientLight(0.2f);
        //Set the initial light accumulation in each pixel to be equal to the ambient light.
        lightBuffer->SetClearColor(glm::vec4(ambientLight.x, ambientLight.y, ambientLight.z, 1.0f));
        lightBuffer->Bind();
        glClearColor(0, 0, 0, 1);

        // Enable buffer color accumulation
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);

        auto shader = shaders["LightPass"];
        shader->Use();

        {
            int texturePositionsLoc = shader->GetUniformLocation("texture_position");
            glUniform1i(texturePositionsLoc, 0);
            frameBuffer->BindTexture(0, GL_TEXTURE0);
        }

        {
            int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
            glUniform1i(textureNormalsLoc, 1);
            frameBuffer->BindTexture(1, GL_TEXTURE0 + 1);
        }

        auto camera = GetSceneCamera();
        glm::vec3 cameraPos = camera->m_transform->GetWorldPosition();
        int loc_eyePosition = shader->GetUniformLocation("eye_position");
        glUniform3fv(loc_eyePosition, 1, glm::value_ptr(cameraPos));

        auto resolution = window->GetResolution();
        int loc_resolution = shader->GetUniformLocation("resolution");
        glUniform2i(loc_resolution, resolution.x, resolution.y);

        //Front face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        for (auto& lightInfo : lights)
        {
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

        auto shader = shaders["Composition"];
        shader->Use();

        int outputTypeLoc = shader->GetUniformLocation("output_type");
        glUniform1i(outputTypeLoc, outputType);

        {
            int texturePositionsLoc = shader->GetUniformLocation("texture_position");
            glUniform1i(texturePositionsLoc, 1);
            frameBuffer->BindTexture(0, GL_TEXTURE0 + 1);
        }

        {
            int textureNormalsLoc = shader->GetUniformLocation("texture_normal");
            glUniform1i(textureNormalsLoc, 2);
            frameBuffer->BindTexture(1, GL_TEXTURE0 + 2);
        }

        {
            int textureColorLoc = shader->GetUniformLocation("texture_color");
            glUniform1i(textureColorLoc, 3);
            frameBuffer->BindTexture(2, GL_TEXTURE0 + 3);
        }

        {
            int textureDepthLoc = shader->GetUniformLocation("texture_depth");
            glUniform1i(textureDepthLoc, 4);
            frameBuffer->BindDepthTexture(GL_TEXTURE0 + 4);
        }

        {
            int textureLightLoc = shader->GetUniformLocation("texture_light");
            glUniform1i(textureLightLoc, 5);
            lightBuffer->BindTexture(0, GL_TEXTURE0 + 5);
        }

        // Render the object again but with different properties
        RenderMesh(meshes["quad"], shader, glm::vec3(0, 0, 0));
    }
}

void Tema1::FrameEnd()
{
    DrawCoordinateSystem();
}


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}

void Tema1::OnKeyPress(int key, int mods)
{
    int index = key - GLFW_KEY_0;
    if (index >= 0 && index <= 9) {
        outputType = index;
    }

    // Toggle wireframe mode
    if (key == GLFW_KEY_F) {
        switch (wireframe) {
        case GL_FILL:
            wireframe = GL_LINE;
            break;
        case GL_LINE:
            wireframe = GL_FILL;
            break;
        }
    }
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema1::OnWindowResize(int width, int height)
{
    // Treat window resize event
    frameBuffer->Resize(width, height, 32);
    lightBuffer->Resize(width, height, 32);
}
