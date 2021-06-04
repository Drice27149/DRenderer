#pragma once

class FrameGraph;
class Device;
class Context;
class ResourceManager;

class Renderer {
public:
    void InitRenderer();
public:
    static FrameGraph* FG;
    static Device* GDevice;
    static Context* GContext;
    static ResourceManager* ResManager;
};