#include "GraphicAPI.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "DEngine.hpp"
#include "Object.hpp"

float skyv[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
};

GLFWwindow* GraphicAPI::InitOpenGL(int width, int height) {
     glfwInit();
     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

     GLFWwindow* window = glfwCreateWindow(width, height, "DRenderer", NULL, NULL);
     if (window == NULL)
     {
         std::cout << "fail to create window" << std::endl;
         glfwTerminate();
         return NULL;
     }

     glfwMakeContextCurrent(window);

     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
     {
         std::cout << "Failed to initialize GLAD" << std::endl;
         return NULL;
     }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, GraphicAPI::FramebufferSizeCallback);
    glfwSetMouseButtonCallback(window, GraphicAPI::MouseButtonCallBack);
    glfwSetCursorPosCallback(window, GraphicAPI::MouseMoveCallBack);
    glfwSetKeyCallback(window, GraphicAPI::KeyCallBack);
    glfwSetScrollCallback(window, GraphicAPI::ScrollCallBack);

     return window;
}

void GraphicAPI::MouseMoveCallBack(GLFWwindow* window, double x, double y)
{
    InputMgr& inputMgr = DEngine::GetInputMgr();
    inputMgr.Tick(x, y);
}

void GraphicAPI::MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods)
{
    InputMgr& inputMgr = DEngine::GetInputMgr();
    if(button == GLFW_MOUSE_BUTTON_LEFT){
        if(GLFW_PRESS == action) 
            inputMgr.OnLMouseDown();
        else
            inputMgr.OnLMouseRelease();
    }
    else if(button == GLFW_MOUSE_BUTTON_RIGHT){
        if(GLFW_PRESS == action) 
            inputMgr.OnRMouseDown();
        else
            inputMgr.OnRMouseRelease();
    }
    else{
        // 不处理
    }
}

void GraphicAPI::ScrollCallBack(GLFWwindow* window, double dx, double dy)
{
    if(dy > 0)
        DEngine::GetInputMgr().OnZoomIn();
    else
        DEngine::GetInputMgr().OnZoomOut();
}

void GraphicAPI::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void GraphicAPI::KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS || GLFW_REPEAT == action){
        if(key == GLFW_KEY_W){
            // TODO: 修正移动的具体值
            DEngine::GetInputMgr().OnZoomIn();
        }
        if(key == GLFW_KEY_S){
            DEngine::GetInputMgr().OnZoomOut();
        }
        if(key == GLFW_KEY_A){
            DEngine::GetInputMgr().OnMoveLeft();
        }
        if(key == GLFW_KEY_D){
            DEngine::GetInputMgr().OnMoveRight();
        }
    }
}

void GraphicAPI::LoadMesh(Mesh& mesh)
{
    GraphicData& gd = mesh.gd;
    glGenVertexArrays(1, &(gd.VAO));
    glBindVertexArray(gd.VAO);
    // 顶点缓冲
    glGenBuffers(1, &(gd.VBO));
    glBindBuffer(GL_ARRAY_BUFFER, gd.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * (mesh.vs.size()), mesh.vs.data(), GL_STATIC_DRAW);
    // 索引缓冲
    glGenBuffers(1, &(gd.EBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gd.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (mesh.ids.size()), mesh.ids.data(), GL_STATIC_DRAW);
    // 配置顶点属性, 用于shader中读取
    // 顶点坐标
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vertex));
    // 顶点法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // 纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    // 切线方向
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    // b切线方向
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
}

void GraphicAPI::LoadImageTexture(Texture& tex, string fn, bool vflip)
{
    glGenTextures(1, &(tex.id));
    glBindTexture(GL_TEXTURE_2D, tex.id);
    int width = 0, height = 0, nrChannels = 0;
    // 竖直反转
    if (vflip) stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(fn.c_str(), &width, &height, &nrChannels, 0);
    if (data && nrChannels > 0) {
        if (nrChannels == 1) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        }
        else if (nrChannels == 2) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, width, height, 0, GL_RG, GL_UNSIGNED_BYTE, data);
        }
        else if (nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (nrChannels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else {
            int channelsOutOfBounds = 1;
            assert(channelsOutOfBounds);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
}

void GraphicAPI::LoadImageCubeMap(CubeMap& cube, vector<string> faces)
{
    assert(faces.size() == 6);
    
    glGenTextures(1, &(cube.id));
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube.id);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// TODO: 定义 RenderMgr
void GraphicAPI::Temp_DrawMesh(Mesh& mesh, Shader& sh)
{
    glBindVertexArray(mesh.gd.VAO);

    mat4 model = glm::mat4(1.0);
    mat4 view = DEngine::GetCamMgr().GetViewTransform();
    mat4 projection = DEngine::GetCamMgr().GetProjectionTransform();
    
    sh.use();
    // 模型变换和透视投影变换
    sh.setMat4("view", view);
    sh.setMat4("proj", projection);
    sh.setFloat("gloss", 128.0);
    sh.setVec3("light0.color", vec3(1.0, 1.0, 1.0));
    sh.setVec3("viewPos", DEngine::GetCamMgr().GetViewPos());
    // 定位纹理贴图
    // TODO: 还有很多纹理贴图
    int texNum = 0;
    if (mesh.mask & (1 << aiTextureType_DIFFUSE)) {
        glActiveTexture(GL_TEXTURE0 + texNum);
        glBindTexture(GL_TEXTURE_2D, mesh.texs[aiTextureType_DIFFUSE].id);
        sh.setInt("modelTex.diffuse", texNum);
        texNum++;
    }
    if (mesh.mask & (1 << aiTextureType_NORMALS)) {
        glActiveTexture(GL_TEXTURE0 + texNum);
        glBindTexture(GL_TEXTURE_2D, mesh.texs[aiTextureType_NORMALS].id);
        sh.setInt("modelTex.normal", texNum);
        texNum++;
    }

    glDrawElements(GL_TRIANGLES, mesh.ids.size(), GL_UNSIGNED_INT, 0);
}

void GraphicAPI::Temp_DrawGrid(Mesh& mesh, Shader& sh)
{
    glBindVertexArray(mesh.gd.VAO);

    mat4 model = glm::mat4(1.0);
    mat4 view = DEngine::GetCamMgr().GetViewTransform();
    mat4 projection = DEngine::GetCamMgr().GetProjectionTransform();
    
    sh.use();
    // 模型变换和透视投影变换
    sh.setMat4("model", model);
    sh.setMat4("view", view);
    sh.setMat4("proj", projection);

    glDrawElements(GL_LINES, mesh.ids.size(), GL_UNSIGNED_INT, 0);
}

void GraphicAPI::Temp_DrawObject(Object& obj, Shader& sh)
{
    sh.use();
    sh.setMat4("model", obj.model);
    for (Mesh& mesh : obj.meshes) {
        GraphicAPI::Temp_DrawMesh(mesh, sh);
    }
}

void GraphicAPI::Temp_DrawSkyBox(CubeMap& cube, Shader& sh)
{
    // very temp, will be fixed by RenderMgr
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*36*3, skyv, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(float)*3, (void*)0);

    mat4 model = glm::mat4(1.0);
    mat4 view = DEngine::GetCamMgr().GetViewTransform();
    mat4 projection = DEngine::GetCamMgr().GetProjectionTransform();

    sh.use();
    // 模型变换和透视投影变换
    sh.setMat4("model", model);
    sh.setMat4("view", view);
    sh.setMat4("proj", projection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube.id);
    sh.setInt("skybox", 0);

    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);
}



