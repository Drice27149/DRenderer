#include "GraphicAPI.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "DEngine.hpp"

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * (mesh.vs.size()), &(mesh.vs[0]), GL_STATIC_DRAW);
    // 索引缓冲
    glGenBuffers(1, &(gd.EBO));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gd.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * (mesh.ids.size()), &(mesh.ids[0]), GL_STATIC_DRAW);
    // 配置顶点属性, 用于shader中读取
    // 顶点坐标
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, v));
    // 顶点法线
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vn));
    // 纹理坐标
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vt));
    // TODO: tagent, bitagent
}

void GraphicAPI::LoadImageTexture(Texture& tex, string fn, bool vflip)
{
    glGenTextures(1, &(tex.id));
    glBindTexture(GL_TEXTURE_2D, tex.id);
    int width, height, nrChannels;
    // 竖直反转
    if (vflip) stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(fn.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
}

void GraphicAPI::Temp_DrawMesh(Mesh& mesh, Shader& sh)
{
    glBindVertexArray(mesh.gd.VAO);

    mat4 model = glm::mat4(1.0);
    mat4 view = DEngine::GetCamMgr().GetCamera().getCamTransform();
    mat4 projection = glm::perspective(90.0, 1.0, 0.1, 2000.0);
    
    sh.use();
    // 模型变换和透视投影变换
    sh.setMat4("model", model);
    sh.setMat4("view", view);
    sh.setMat4("proj", projection);
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
    mat4 view = DEngine::GetCamMgr().GetCamera().getCamTransform();
    mat4 projection = glm::perspective(90.0, 1.0, 0.1, 2000.0);
    
    sh.use();
    // 模型变换和透视投影变换
    sh.setMat4("model", model);
    sh.setMat4("view", view);
    sh.setMat4("proj", projection);

    glDrawElements(GL_LINES, mesh.ids.size(), GL_UNSIGNED_INT, 0);
}

