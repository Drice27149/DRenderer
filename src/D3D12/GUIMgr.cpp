#include "GUIMgr.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "DEngine.hpp"
#include "Util.hpp"

void GUIMgr::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
    CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
    heapMgr->GetNewSRV(srvCpu, srvGpu);

    ImGui_ImplWin32_Init(mhMainWnd);
    ImGui_ImplDX12_Init(device, 3, DXGI_FORMAT_R8G8B8A8_UNORM, heapMgr->GetSRVHeap(), srvCpu, srvGpu);
}

void GUIMgr::Update()
{
    // imgui stuff
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Editor Window");
    // performance, not accurate but ok
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);    
    // temp input buffer
    static char inBuff[128][128];
    static string recieve = "";
    int bufferID = 0;

    auto callback = [](ImGuiInputTextCallbackData* data)->int{
        std::string s;
        for(int i = 0; i < data->BufSize; i++){
            if((data->Buf[i]>='0' && data->Buf[i]<='9') || data->Buf[i]=='.' || data->Buf[i]=='-')
                s.push_back(data->Buf[i]);
            else 
                break;
        }
        recieve = s;
        float value = String2Float(s);
        float* desc = (float*)(data->UserData);
        *desc = value;
        return 0;
    };

    if(ImGui::TreeNode("Objects Info")){
        std::vector<Object*>& objs = DEngine::gobjs;
        int objID = 0;
        for(Object* obj: objs){
            std::string nodeName = Int2String(objID++);
            if(ImGui::TreeNode(nodeName.c_str())){
                for(metaData& member: Object::reflection){
                    float* ptr = (float*)((unsigned long long)obj + member.offset);
                    std::string value = Float2String(*ptr, 6);
                    CopyStringToBuffer(value, inBuff[bufferID]);
                    ImGui::InputText(member.name.c_str(), inBuff[bufferID], 64, ImGuiInputTextFlags_CallbackCompletion, callback, ptr);
                    bufferID++;
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Scene Info")){
        std::shared_ptr<SceneInfo> sceneInfo = constantMgr->GetSceneInfo();
        SceneInfo* addr = sceneInfo.get();

        for(metaData& member: SceneInfo::reflections){
            float* ptr = (float*)((unsigned long long)addr + member.offset);
            std::string value = Float2String(*ptr, 6);
            CopyStringToBuffer(value, inBuff[bufferID]);
            ImGui::InputText(member.name.c_str(), inBuff[bufferID], 64, ImGuiInputTextFlags_CallbackCompletion, callback, ptr);
            bufferID++;
        }

        ImGui::TreePop();
    }

    ImGui::Text(recieve.c_str());
    
    ImGui::End();

    ImGui::Render();
}
    
void GUIMgr::Draw()
{
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}