#include "lab_m2/tema2/Tema2.h"

#include <vector>
#include <iostream>

#include "pfd/portable-file-dialogs.h"

using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
    window->SetSize(600, 600);
}


Tema2::~Tema2()
= default;


void Tema2::Init()
{
    // init frame buffers
    medianHorizontal = new FrameBuffer();
    medianVertical = new FrameBuffer();
    hash = new FrameBuffer();
    sobel = new FrameBuffer();
    final = new FrameBuffer();

    medianHorizontal->Generate(window->GetResolution().x, window->GetResolution().y, 1, false);
    medianVertical->Generate(window->GetResolution().x, window->GetResolution().y, 1, false);
    hash->Generate(window->GetResolution().x, window->GetResolution().y, 4, false);
    sobel->Generate(window->GetResolution().x, window->GetResolution().y, 1, false);
    final->Generate(window->GetResolution().x, window->GetResolution().y, 1, false);

    // originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema2", "earth.png"), "earth", "image", true, true);
    originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "earth.png"), nullptr, "image", true, true);

    {
        const auto mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    const std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema2", "shaders");

    {
        auto *shader = new Shader("MedianFilterHorizontal");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "MedianHorizontal.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        auto *shader = new Shader("MedianFilterVertical");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "MedianVertical.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        auto *shader = new Shader("Hash");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Hash.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        auto *shader = new Shader("Sobel");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Sobel.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        auto *shader = new Shader("Final");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Final.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        auto *shader = new Shader("FS");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentShader.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

}


void Tema2::FrameStart()
{
}


void Tema2::Update(float deltaTimeSeconds)
{
    // Horizontal blur pass
    {
        medianHorizontal->Bind();
        ClearScreen();

        const auto shader = shaders["MedianFilterHorizontal"];
        const auto resolution = window->GetResolution();

        shader->Use();
        originalImage->BindToTextureUnit(GL_TEXTURE0);

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1i(shader->GetUniformLocation("color_texture"), 0);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }

    // Vertical blur pass
    {
        medianVertical->Bind();
        ClearScreen();

        const auto shader = shaders["MedianFilterVertical"];
        const auto resolution = window->GetResolution();

        shader->Use();
        medianHorizontal->GetTexture(0)->BindToTextureUnit(GL_TEXTURE0);

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1i(shader->GetUniformLocation("color_texture"), 0);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }

    // Hash pass
    {
        hash->Bind();
        ClearScreen();

        const auto shader = shaders["Hash"];
        const auto resolution = window->GetResolution();

        shader->Use();
        medianVertical->GetTexture(0)->BindToTextureUnit(GL_TEXTURE0);

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1i(shader->GetUniformLocation("color_texture"), 0);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }

    // Sobel pass
    {
        sobel->Bind();
        ClearScreen();

        const auto shader = shaders["Sobel"];
        const auto resolution = window->GetResolution();

        shader->Use();
        originalImage->BindToTextureUnit(GL_TEXTURE0);

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1i(shader->GetUniformLocation("color_texture"), 0);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }

    // Final pass
    {
        final->Bind();
        ClearScreen();

        const auto shader = shaders["Final"];
        const auto resolution = window->GetResolution();

        shader->Use();
        hash->GetTexture(3)->BindToTextureUnit(GL_TEXTURE0);
        sobel->GetTexture(0)->BindToTextureUnit(GL_TEXTURE1);

        glUniform2i(shader->GetUniformLocation("screenSize"), resolution.x, resolution.y);
        glUniform1i(shader->GetUniformLocation("textureImage1"), 0);
        glUniform1i(shader->GetUniformLocation("textureImage2"), 1);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }

    // Default framebuffer pass
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ClearScreen();

        const auto shader = shaders["FS"];
        shader->Use();

        final->GetTexture(0)->BindToTextureUnit(GL_TEXTURE0);
        glUniform1i(shader->GetUniformLocation("color_texture"), 0);

        RenderMesh(meshes["quad"], shader, glm::mat4(1));
    }
}


void Tema2::FrameEnd()
{
    DrawCoordinateSystem();
}


void Tema2::OnFileSelected(const std::string &fileName)
{
    if (!fileName.empty())
    {
        std::cout << fileName << endl;
        originalImage = TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        processedImage = TextureManager::LoadTexture(fileName, nullptr, "newImage", true, true);

        const float aspectRatio = static_cast<float>(originalImage->GetWidth()) / static_cast<float>(originalImage->GetHeight());
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}


void Tema2::GrayScale() const
{
    const unsigned int channels = originalImage->GetNrChannels();
    const unsigned char* data = originalImage->GetImageData();
    unsigned char* newData = processedImage->GetImageData();

    if (channels < 3)
        return;

    const auto imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    for (int i = 0; i < imageSize.y; i++)
    {
        for (int j = 0; j < imageSize.x; j++)
        {
            const unsigned int offset = channels * (i * imageSize.x + j);

            // Reset save image data
            const char value = static_cast<char>(static_cast<float>(data[offset + 0]) * 0.2f + static_cast<float>(data[offset + 1]) * 0.71f + static_cast<float>(data[offset + 2]) * 0.07);
            memset(&newData[offset], value, 3);
        }
    }

    processedImage->UploadNewData(newData);
}


void Tema2::SaveImage(const std::string &fileName) const
{
    cout << "Saving image! ";
    processedImage->SaveToFile((fileName + ".png").c_str());
    cout << "[Done]" << endl;
}


void Tema2::OpenDialog()
{
    const std::vector<std::string> filters =
    {
        "Image Files", "*.png *.jpg *.jpeg *.bmp",
        "All Files", "*"
    };

    const auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Tema2::OnKeyPress(const int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }
}


void Tema2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema2::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
