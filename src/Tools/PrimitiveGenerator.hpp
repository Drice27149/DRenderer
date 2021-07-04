#pragma once

#include "Surfel.hpp"
#include "Object.hpp"

class PrimitiveGenerator {
public:
    static Object* GenerateSurfelMesh(std::vector<Surfel> surfels);
public:
    
};

Object* PrimitiveGenerator::GenerateSurfelMesh(std::vector<Surfel> surfels)
{
    Object* surfelObj = new Object();
    surfelObj->drawType = DrawType::Debug;
    for(auto& surfel: surfels){
        Cube now = Cube(surfel.position);
        surfelObj->MergeMesh(now);
    }
    return surfelObj;
}