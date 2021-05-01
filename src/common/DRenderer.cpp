// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <iostream>
// #include <cmath>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>
// #include <cassert>

// #include "Global.hpp"
// #include "Shader.hpp"
// #include "Camera.hpp"
// #include "Loader.hpp"
// #include "Object.hpp"
// #include "Scene.hpp"
// #include "Renderer.hpp"

// int MyHeight = 600;
// int MyWidth = 800;

// void framebuffer_size_callback(GLFWwindow* window, int width, int height)
// {
//     glViewport(0, 0, width, height);
// }

// float lastX = -1.0f, lastY = -1.0f;
// Camera* myCam;

// void mouseCallBack(GLFWwindow* window, double xpos, double ypos)
// {
//     if (lastX < 0) {
//         lastX = xpos;
//         lastY = ypos;
//     }
//     else {
//         myCam->rotatePitch(lastY - ypos);
//         myCam->rotateYaw(xpos - lastX);
//         lastX = xpos;
//         lastY = ypos;
//     }
// }

// GLFWwindow* initOpenGL() {
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//     GLFWwindow* window = glfwCreateWindow(MyWidth, MyHeight, "LearnOpenGL", NULL, NULL);
//     if (window == NULL)
//     {
//         std::cout << "fail to create window" << std::endl;
//         glfwTerminate();
//         return NULL;
//     }

//     glfwMakeContextCurrent(window);

//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cout << "Failed to initialize GLAD" << std::endl;
//         return NULL;
//     }

//     glViewport(0, 0, MyWidth, MyHeight);
//     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//     // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
//     glfwSetCursorPosCallback(window, mouseCallBack);

//     return window;
// }

// Object obj;
// Object obj1;
// Object obj2;

// int main(int argc, const char* argv[])
// {
//     AssimpLoad

//     return 0;

//     GLFWwindow* window = initOpenGL();
//     assert(window != NULL);

//     Scene scene;
//     scene.height = MyHeight;
//     scene.width = MyWidth;
//     Renderer r;
//     r.height = MyHeight;
//     r.width = MyWidth;
//     r.scene = &scene;

//     // load object and texture
//     printf("Loading object\n");
//     // obj.LoadModel("../assets/backpack/backpack.obj");
//     // obj.LoadTexture("../assets/backpack/diffuse.jpg");
//     // obj.LoadNormal("../assets/backpack/normal.png");
//     obj.LoadModel("../assets/spot/spot_triangulated_good.obj");
//     obj.LoadTexture("../assets/spot/spot_texture.png");
//     obj.Transform(glm::translate(glm::mat4(1.0), glm::vec3(0, -1.3, 0)));
//     scene.AddObject(&obj);

//     printf("Set up scene\n");
//     glm::mat4 projection = glm::perspective(90.0, 1.0, 0.1, 2000.0);
//     r.projection = projection;

//     // light postion and direction
//     glm::vec3 eyePos(0, 0, -1.5);
//     glm::vec3 lightPos(-1.0, 1.8, -3.0);
//     glm::vec3 lightDir(0, -1.0, 0.2);
//     // glm::vec3 lightPos(eyePos);
//     glm::vec3 inten(2.5, 2.5, 2.5);
//     glm::vec3 inten_a(0.3, 0.3, 0.3);
//     glm::vec3 eyeDir(0, 0, 1.0);
//     vector<glm::vec3> light = { lightPos, lightDir, inten };
//     scene.AddLight(light);
//     scene.inten_a = inten_a;

//     // eye postion and direction
//     r.eyePos = eyePos;
//     r.eyeDir = eyeDir;

//     Camera camera(eyePos, eyeDir, 0.002f, 0.005f);
//     myCam = &camera;
//     r.camera = myCam;

//     printf("Compileing Shader\n");
//     Shader shadowShader("../shaders/vertexShader.glsl", "../shaders/depthShader.glsl");
//     Shader shader("../shaders/vertexShader.glsl", "../shaders/fragmentShader.glsl");
//     Shader gShader("../shaders/gvs.glsl", "../shaders/gfs.glsl");
//     Shader sShader("../shaders/gvs.glsl", "../shaders/ssaoFS.glsl");
//     // model transform, temp for spot model
//     obj.Transform(glm::translate(glm::mat4(1.0), vec3(0, 1.0, 0)));

//     printf("Begin rendering\n");
//     int frameCnt = 0;
//     float lastSecond = -1.0;
//     float cycle = 3.0;

//     r.InitRender();
//     r.InitSSAO();

//     while (!glfwWindowShouldClose(window))
//     {
//         float nowSecond = glfwGetTime();
//         if (lastSecond < 0) lastSecond = nowSecond;
//         else {
//             if (nowSecond - lastSecond >= cycle) {
//                 lastSecond = nowSecond;
//                 printf("fps: %.2f\n", (float)frameCnt / cycle);
//                 frameCnt = 0;
//             }
//             else {
//                 frameCnt++;
//             }
//         }
//         camera.processInput(window);

//         // 物体随时间旋�?
//         // obj.Transform(glm::rotate(glm::mat4(1.0), 0.0012f, vec3(0, 1.0, 0)));

//         r.ShadowPass(&shadowShader);
//         r.GPass(&gShader);
//         r.SSAOPass(&sShader);
//         r.Render(&shader);

//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     glfwTerminate();
//     return 0;
// }