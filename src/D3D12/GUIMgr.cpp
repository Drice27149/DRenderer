#include "GUIMgr.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "DEngine.hpp"
#include "Util.hpp"
#include "Graphics.hpp"

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
    Graphics::heapMgr->GetNewSRV(srvCpu, srvGpu);

    ImGui_ImplWin32_Init(mhMainWnd);
    ImGui_ImplDX12_Init(device, 3, DXGI_FORMAT_R8G8B8A8_UNORM, Graphics::heapMgr->GetSRVHeap(), srvCpu, srvGpu);
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
        // hack: . for float
        bool haveDot = false;
        std::string result;
        // data->Buf[i] is input string
        for(int i = 0; i < data->BufSize; i++){
            if(data->Buf[i] == '.') haveDot = true;
            if((data->Buf[i]>='0' && data->Buf[i]<='9') || data->Buf[i]=='.' || data->Buf[i]=='-')
                result.push_back(data->Buf[i]);
            else 
                break;
        }
        recieve = result;
        // @TODO: implement it better
        if(haveDot){
            float value = String2Float(result);
            float* desc = (float*)(data->UserData);
            *desc = value;
        }
        else{
            int value = String2Int(result);
            int* desc = (int*)(data->UserData);
            *desc = value;
        }
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

    // @TODO: decouple

    if(ImGui::TreeNode("Scene Info")){
        std::shared_ptr<SceneInfo> sceneInfo = Graphics::constantMgr->GetSceneInfo();
        SceneInfo* addr = sceneInfo.get();

        for(metaData& member: SceneInfo::reflections){
            if(member.type == 1){
                float* ptr = (float*)((unsigned long long)addr + member.offset);
                std::string value = Float2String(*ptr, 6);
                CopyStringToBuffer(value, inBuff[bufferID]);
                ImGui::InputText(member.name.c_str(), inBuff[bufferID], 64, ImGuiInputTextFlags_CallbackCompletion, callback, ptr);
                bufferID++;
            }
            else{
                int* ptr = (int*)((unsigned long long)addr + member.offset);
                std::string value = Int2String(*ptr);
                CopyStringToBuffer(value, inBuff[bufferID]);
                ImGui::InputText(member.name.c_str(), inBuff[bufferID], 64, ImGuiInputTextFlags_CallbackCompletion, callback, ptr);
                bufferID++;
            }
        }

        ImGui::TreePop();
    }

    if(ImGui::TreeNode("debug cam")){
        float x = DEngine::GetCamMgr().GetCamera().position.x;
        float y = DEngine::GetCamMgr().GetCamera().position.y; 
        float z = DEngine::GetCamMgr().GetCamera().position.z;
        float pitch = DEngine::GetCamMgr().GetCamera().pitch;
        float yaw = DEngine::GetCamMgr().GetCamera().yaw;
        ImGui::Text(Float2String(x,3).c_str());
        ImGui::Text(Float2String(y,3).c_str());
        ImGui::Text(Float2String(z,3).c_str());
        ImGui::Text(Float2String(pitch,3).c_str());
        ImGui::Text(Float2String(yaw,3).c_str());
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