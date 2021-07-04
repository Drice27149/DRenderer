#pragma once

#include <ctime>
#include <cstdlib>
#include <cassert>

#include "Surfel.hpp"
#include "WorldTriangle.hpp"

class SurfelGenerator {
public:
    float sumArea = -1.0;
public:
    float RandNorm()
    {
        return (float)rand() / (float)RAND_MAX;
    }

    vec3 RandBarycentricCoordinates()
    {
        float a = RandNorm(), b = RandNorm();
        return vec3(1.0-sqrt(a), sqrt(a)*(1.0-b), sqrt(a)*b);
    }

    std::vector<Surfel> GenerateSurfels(std::vector<WorldTriangle>& trians, int surfelCnt)
    {
        srand(time(NULL));

        std::vector<Surfel> res;

        sumArea = 0.0;
        for(auto& t: trians) sumArea += t.area;
        for(int i = 0; i < surfelCnt; i++){
            float randArea = RandNorm() * sumArea;
            float accArea = 0.0;
            WorldTriangle* ptr = nullptr;
            for(auto& t: trians){
                accArea += t.area;
                if(accArea >= randArea){
                    ptr = &t;
                    break;
                }
            }
            assert(ptr != nullptr);
            vec3 bcoord = RandBarycentricCoordinates();
            Surfel now;
            now.normal = ptr->normal;
            now.position = bcoord.x * ptr->position[0] + bcoord.y * ptr->position[1] + bcoord.z * ptr->position[2];
            now.albedo = vec3(1.0, 1.0, 1.0);
            res.push_back(now);
        }
        return res;
    }

    std::vector<Surfel> MergeSurfels(std::vector<Surfel>& surfels)
    {
        return surfels;
    }

    std::vector<SurfelCluster> GenerateSurfelClusters(std::vector<Surfel>& surfels, int maxSurfelPerCluster)
    {
        std::vector<SurfelCluster> res;
        // distance = d(albedo) * 1000 + d(position) ?
        
    }
};