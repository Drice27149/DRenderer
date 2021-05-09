#pragma once

#include <glm/glm.hpp>

struct PassUniform
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 SMView;
    glm::mat4 SMProj;
};

struct ObjectUniform
{
    glm::mat4 model;
    unsigned int id;
};