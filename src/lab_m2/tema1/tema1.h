#pragma once

#include <string>
#include <vector>

#include "components/simple_scene.h"
#include "components/transform.h"
#include "core/gpu/frame_buffer.h"
#include "core/gpu/particle_effect.h"

namespace m2
{
    struct LightInfoTema1
    {
        glm::vec3 position;
        glm::vec3 color;
        float radius;
    };

    class Tema1 : public gfxc::SimpleScene
    {
     public:
        Tema1();
        ~Tema1();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void LoadShader(const std::string &fileName);
        void RenderMeshInstanced(Mesh *mesh, Shader *shader, const glm::mat4 &modelMatrix, int instances, const glm::vec3 &color = glm::vec3(1));
        Texture2D* CreateRandomTexture(unsigned int width, unsigned int height);
        GLuint LoadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z);
        void RenderSkybox(GLuint skyboxTextureID);
        void ManualRenderSkybox(GLuint VAO, GLuint textureID, Shader *shader);
        unsigned int UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z);

        void ResetParticlesRainSnow(int xSize, int ySize, int zSize);
        void ResetParticlesFire(float radius);

     private:
         FrameBuffer *frameBuffer;
         FrameBuffer *lightBuffer;

         std::vector<LightInfoTema1> lights;

         int outputType;

         int no_of_instances;

         int terrain_size_x = 8;
         int terrain_size_z = 8;

         int terrain_resolution_x = 512;
         int terrain_resolution_z = 512;

         GLenum wireframe = GL_FILL;

         GLuint skyboxTextureID;
         GLuint skyboxVAO;

         Texture2D *heightmap_texture;

         glm::vec3 generator_position;
         float offset;
    };
}   // namespace m2
