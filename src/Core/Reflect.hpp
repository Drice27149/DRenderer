#pragma once

#include <iostream>
using namespace std;

namespace Reflect {
    
enum Type {
    INT,
    FLOAT,
    FLOAT3,
    STRING,
};

struct Data {
    unsigned long long offset;
    Type type;
    string name[3];
    float f[3];
    int i[3];
    string s;
};

struct Element {
    std::vector<Reflect::Data> datas;
};

};

class Reflectable {
public:
    virtual std::vector<Reflect::Data> serialize() = 0;

    virtual void deserialize(std::vector<Reflect::Data> datas)
    {
        for(const auto& data: datas){
            if(data.type == Reflect::Type::FLOAT){
                float* ptr = (float*)((unsigned long long)this + data.offset);
                *ptr = data.f[0];
            }
            if(data.type == Reflect::Type::FLOAT3){
                float* ptr0 = (float*)((unsigned long long)this + data.offset);
                float* ptr1 = (float*)((unsigned long long)this + data.offset + sizeof(float));
                float* ptr2 = (float*)((unsigned long long)this + data.offset + 2*sizeof(float));
                *ptr0 = data.f[0];
                *ptr1 = data.f[1];
                *ptr2 = data.f[2];
            }
            if(data.type == Reflect::Type::INT){
                int* ptr = (int*)((unsigned long long)this + data.offset);
                *ptr = data.i[0];
            }
            if(data.type == Reflect::Type::STRING){
                string* ptr = (string*)((unsigned long long)this + data.offset);
                *ptr = data.s;
            }
        }
    }
};