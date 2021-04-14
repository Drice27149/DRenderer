#include "SceneMgr.hpp"

SceneMgr::SceneMgr()
{

}

void SceneMgr::AddObject(Object* obj)
{
    objs.push_back(obj);
}