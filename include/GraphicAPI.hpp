#pragma once
#include "Global.hpp"

class Mesh;
class Shader;
class Texture;
class Object;
class CubeMap;

namespace GraphicAPI {

GLFWwindow* InitOpenGL(int width, int height);

void MouseMoveCallBack(GLFWwindow* window, double x, double y);

void MouseButtonCallBack(GLFWwindow* window, int button, int action, int mods);

void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

void ScrollCallBack(GLFWwindow* window, double dx, double dy);

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

void LoadMesh(Mesh& mesh);

void LoadImageTexture(Texture& tex, string fn, bool vflip = false);

void LoadImageCubeMap(CubeMap& cube, vector<string> fns);

// 顺时针
void Temp_DrawGrid(Mesh& mesh, Shader& sh);

void Temp_DrawSkyBox(CubeMap& cube, Shader& sh);

// 测试函数
void Temp_DrawMesh(Mesh& mesh, Shader& sh); 

void Temp_DrawObject(Object& object, Shader& sh);

}