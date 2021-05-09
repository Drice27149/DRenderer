#include "PassMgr.hpp"
#include "Resource.hpp"

class PreZMgr: public PassMgr {
public:
    PreZMgr();
public:
    std::shared_ptr<Resource> depthMap;
};