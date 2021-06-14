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