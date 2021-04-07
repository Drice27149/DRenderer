#include "Scene.hpp"

Scene::Scene()
{

}

void Scene::AddLight(vector<glm::vec3> light)
{
    // {lightPos, lightDirection, lightIntensity}
    lights.push_back(light);
}

void Scene::AddObject(Object* object)
{
    objects.push_back(object);
}