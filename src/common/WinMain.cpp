
#include "Global.hpp"
#include "Object.hpp"
#include "AssimpLoader.hpp"
#include "DEngine.hpp"
#include "Graphics.hpp"
#include "Light.hpp"
#include "Parallel.hpp"
#include "GIBaker.hpp"

DEngine* DEngine::instance = nullptr;
std::vector<Object*> DEngine::gobjs;

// temporal, sorry..
ID3D12Device* Graphics::GDevice = nullptr;
ID3D12GraphicsCommandList* Graphics::GCmdList = nullptr;
unsigned int Graphics::viewPortWidth;
unsigned int Graphics::viewPortHeight;

std::shared_ptr<ConstantMgr> Graphics::constantMgr = nullptr;
std::shared_ptr<HeapMgr> Graphics::heapMgr = nullptr;
std::shared_ptr<TextureMgr> Graphics::textureMgr = nullptr;
std::shared_ptr<PreZMgr> Graphics::preZMgr = nullptr;
std::shared_ptr<ShadowMgr> Graphics::shadowMgr = nullptr;
std::shared_ptr<SkyBoxMgr> Graphics::skyBoxMgr = nullptr;
std::shared_ptr<ClusterMgr> Graphics::clusterMgr = nullptr;
std::shared_ptr<LightCullMgr> Graphics::lightCullMgr = nullptr;
std::shared_ptr<DebugVisMgr> Graphics::debugVisMgr = nullptr;
std::shared_ptr<PBRMgr> Graphics::pbrMgr = nullptr;
std::shared_ptr<GUIMgr> Graphics::guiMgr = nullptr;
std::shared_ptr<AAMgr> Graphics::aaMgr = nullptr;
std::shared_ptr<TemporalAA> Graphics::temporalAA = nullptr;
std::shared_ptr<ToneMapping> Graphics::toneMapping = nullptr;
std::shared_ptr<Bloom> Graphics::bloom = nullptr;
std::shared_ptr<PrefilterIBL> Graphics::prefilterIBL = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    try
    {
        DEngine::Launch();

        Graphics theApp(hInstance);

        DEngine::gobjs.clear();

        DEngine::instance->Init();

        // load scene config
        for(auto obj: DEngine::gobjs){
            waitLoadObj.push_back(obj);
        }
        // load obj file
        auto WorkFunc = [](int64_t id)->void{
            auto obj = waitLoadObj[id];
            AssimpLoader ld;
            ld.LoadFile(obj, obj->fn);
        };
        parallelInit();
        parallelFor1D(WorkFunc, waitLoadObj.size(), 1);
        parallelCleanUp();

        // Bake GI
        GIBaker baker;
        baker.GetClusters();
        DEngine::gobjs.push_back(baker.debugObj);

        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBoxW(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}