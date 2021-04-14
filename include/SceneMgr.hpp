#include "Global.hpp"
#include "Object.hpp"

class SceneMgr {
public:
    SceneMgr();
    void AddObject(Object* obj);
public:
    vector<Object*> objs;
};