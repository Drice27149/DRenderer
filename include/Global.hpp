#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <random>
#include <string>
using glm::vec2;
using glm::vec3;
using glm::mat4;
using std::vector;
// diffuse: texture0
// depthmap: texture1
// nomalmap: texture2
// gpostion: 3
// gnormal: 4
// gcolor: 5
// ssao: 6