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

    // {
    //     static float f = 0.0f;
    //     static int counter = 0;

    //     ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    //     ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    //     // ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
    //     // ImGui::Checkbox("Another Window", &show_another_window);

    //     ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    //     // ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    //     if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
    //         counter++;
    //     ImGui::SameLine();
    //     ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    //     ImGui::End();
    // }

    
    // temp input buffer
    static char inBuff[128][128];
    static string recieve = "";
    int id = 0;

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

    std::vector<Object*>& objs = DEngine::gobjs;
    int tired = 0;
    for(Object* obj: objs){
        std::string nodeName = Int2String(tired++);
        if(ImGui::TreeNode(nodeName.c_str())){
            for(metaData& member: Object::reflection){
                float* ptr = (float*)((unsigned long long)obj + member.offset);
                std::string value = Float2String(*ptr, 6);
                CopyStringToBuffer(value, inBuff[id]);
                ImGui::InputText(member.name.c_str(), inBuff[id], 64, ImGuiInputTextFlags_CallbackCompletion, callback, ptr);
                id++;
            }
            ImGui::TreePop();
        }
    }
    ImGui::Button(recieve.c_str());

    ImGui::End();

    ImGui::Render();
}
    
void GUIMgr::Draw()
{
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}