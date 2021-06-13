#include <iostream>
using namespace std;

namespace Reflect {
    
enum Type {
    INT,
    FLOAT,
    FLOAT3,
};

struct Data {
    unsigned long long offset;
    Type type;
    string name[3];
    float d[3];
};

};