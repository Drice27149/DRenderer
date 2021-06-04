#include "Renderer.hpp"
#include "Device.hpp"
#include "Context.hpp"
#include "FrameGraph.hpp"
#include "ResourceManager.hpp"

FrameGraph* Renderer::FG = nullptr;
Device* Renderer::GDevice = nullptr;
Context* Renderer::GContext = nullptr;
ResourceManager* Renderer::ResManager = nullptr;

void Renderer::InitRenderer()
{
    FG = new FrameGraph();
    GDevice = new Device();
    GContext = new Context();
    ResManager = new ResourceManager();
}