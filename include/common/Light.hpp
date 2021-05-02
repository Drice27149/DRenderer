#include "Object.hpp"
#include "AssimpLoader.hpp"

class Light: public Object {
public:
    Light(DrawType drawType, glm::mat4 model = glm::mat4(1.0))
    {
        this->drawType = drawType;
        this->model = model;

        AssimpLoader ld;
        if(drawType == DrawType::SpotLgiht){
            ld.LoadFile(this, "../assets/LightShapes/cube.obj");
        }
        else if(drawType == DrawType::PointLight){
            ld.LoadFile(this, "../assets/LightShapes/sphere.obj");
        }
        else {
            // TODO: throw error, draw type not found
        }
    }
};