#include "Global.hpp"
#include "GraphicAPI.hpp"
#include "AssimpLoader.hpp"
#include "Mesh.hpp"
#include "Object.hpp"

 void framebuffer_size_callback(GLFWwindow* window, int width, int height)
 {
     glViewport(0, 0, width, height);
 }

 GLFWwindow* InitOpenGL(int width, int height) {
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
     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
     // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
     // glfwSetCursorPosCallback(window, mouseCallBack);

     return window;
 }

int main()
{
    // ������ִ�г�ʼ��, ���ܵ��� gl ����, ��������г���
    GLFWwindow* window = InitOpenGL(800, 600);
    assert(window != NULL);

    string fn = "../assets/backpack/backpack.obj";
    AssimpLoader* ld = new AssimpLoader();
    ld->LoadFile(fn);

    Mesh* mesh = new Mesh(ld->vs, ld->ids);

    printf("Load Mesh done\n");

    string vs_s = "../shaders/tvs.glsl";
    string fs_s = "../shaders/tfs.glsl";

    Shader sh(vs_s, fs_s);
    sh.use();

    return 0;
}