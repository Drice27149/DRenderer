#pragma once

#include "DEngine.hpp"
#include "SurfelGenerator.hpp"
#include "PrimitiveGenerator.hpp"

class GIBaker {
public:
    std::vector<WorldTriangle> GetTriangleList();
    std::vector<SurfelCluster> GetClusters();
public:
    Object* debugObj = nullptr;
};

std::vector<WorldTriangle> GIBaker::GetTriangleList()
{
    std::vector<WorldTriangle> res;
    for(Object* obj: DEngine::gobjs){
        auto tList = obj->GetTriangleList();
        for(auto t: tList){
            res.push_back(t);
        }
    }
    return res;
}

std::vector<SurfelCluster> GIBaker::GetClusters()
{
    SurfelGenerator surfelGen;
    int surfelCnt = 16384; int maxSurfelPerCluster = 8;
    std::vector<Surfel> surfels = surfelGen.GenerateSurfels(GetTriangleList(), surfelCnt);
    surfels = surfelGen.MergeSurfels(surfels);

    debugObj = PrimitiveGenerator::GenerateSurfelMesh(surfels);

    std::vector<SurfelCluster> clusters = surfelGen.GenerateSurfelClusters(surfels, maxSurfelPerCluster);
    return clusters;
}