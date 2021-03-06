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
                auto datas = obj->serialize();
                for(auto data: datas){
                    if(data.type == Reflect::Type::FLOAT){
                        float* ptr = (float*)((unsigned long long)obj + data.offset);
                        std::string value = Float2String(*ptr, 6);
                        CopyStringToBuffer(value, inBuff[bufferID]);
                        ImGui::InputFloat(data.name[0].c_str(), ptr);
                    }
                    else if(data.type == Reflect::Type::FLOAT3){
                        float* ptr = (float*)((unsigned long long)obj + data.offset);
                        std::string wholeName = data.name[0] + ", " + data.name[1] + ", " + data.name[2];
                        ImGui::InputFloat3(wholeName.c_str(), ptr);
                    }
                    else if(data.type == Reflect::Type::INT){
                        int* ptr = (int*)((unsigned long long)obj + data.offset);
                        ImGui::InputInt(data.name[0].c_str(), ptr); 
                    }
                    else if(data.type == Reflect::Type::STRING){

                    }
                }
                ImGui::TreePop();
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

    if(ImGui::TreeNode("Camera Info")){
        auto datas = DEngine::GetCamMgr().GetCamera().serialize();
        auto obj = &(DEngine::GetCamMgr().GetCamera());
        for(auto data: datas){
            if(data.type == Reflect::Type::FLOAT){
                float* ptr = (float*)((unsigned long long)obj + data.offset);
                std::string value = Float2String(*ptr, 6);
                CopyStringToBuffer(value, inBuff[bufferID]);
                ImGui::InputFloat(data.name[0].c_str(), ptr);
            }
            else if(data.type == Reflect::Type::FLOAT3){
                float* ptr = (float*)((unsigned long long)obj + data.offset);
                std::string wholeName = data.name[0] + ", " + data.name[1] + ", " + data.name[2];
                ImGui::InputFloat3(wholeName.c_str(), ptr);
            }
            else if(data.type == Reflect::Type::INT){
                int* ptr = (int*)((unsigned long long)obj + data.offset);
                ImGui::InputInt(data.name[0].c_str(), ptr); 
            }
            else if(data.type == Reflect::Type::STRING){

            }
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