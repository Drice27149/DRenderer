#include "Device.hpp"
#include "RenderStruct.hpp"
#include "Util.hpp"
#include "Struct.hpp"
#include "RenderPass.hpp"
#include "Context.hpp"
#include "ResourceManager.hpp"

ComPtr<ID3D12Device> Device::GDevice = nullptr;

void Device::SetDevice(ComPtr<ID3D12Device> device)
{
    GDevice = device;
}

ID3D12Device* Device::GetDevice()
{
    return GDevice.Get();
}

ID3DBlob* Device::GetShader(ShaderData data)
{
    if(shaders.count(data.name))
        return shaders[data.name].Get();
    if(data.type == ShaderEnum::Type::CS)
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "CS", "cs_5_1");
    else if(data.type == ShaderEnum::Type::GS)
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "GS", "gs_5_1");
    else if(data.type == ShaderEnum::Type::PS)
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "PS", "ps_5_1");
    else
        shaders[data.name] = d3dUtil::CompileShader(GetWString(data.name), nullptr, "VS", "vs_5_1");
    return shaders[data.name].Get();
}

ID3D12RootSignature* Device::CreateRSt(const PassData& data)
{
    ComPtr<ID3D12RootSignature> rst;

    std::vector<CD3DX12_ROOT_PARAMETER> rootParams;
    unsigned cbv = 0;
    unsigned srv = 0;
    for(const auto& input: data.inputs){
        if(input.state == ResourceEnum::State::Read) {
            if(input.type==ResourceEnum::Type::Constant){
                CD3DX12_ROOT_PARAMETER param;
                param.InitAsConstantBufferView(cbv++);
                rootParams.push_back(param);
            }
            else if(input.type==ResourceEnum::Type::Texture2D || input.type==ResourceEnum::Type::TextureCube){
                CD3DX12_DESCRIPTOR_RANGE desc;
                desc.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srv++);
                CD3DX12_ROOT_PARAMETER param;
                param.InitAsDescriptorTable(1, &desc , D3D12_SHADER_VISIBILITY_PIXEL);
                rootParams.push_back(param);
            }
            // @TODO: else
        }
    }

    // sampler for texture fetch
    auto staticSamplers = GetStaticSamplers();
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(rootParams.size(), rootParams.data(), (unsigned int)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr){
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(Device::GetDevice()->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&rst)
        )
    );

    // keep ref
    rsts.push_back(rst);

    return rst.Get();
}

void Device::SetUpRenderPass(RenderPass& renderPass, const PassData& data)
{
    // create root signature
    renderPass.rst = Device::CreateRSt(data);

    // create pipeline state object
    ComPtr<ID3D12PipelineState> pso;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { Util::inputLayout.data(), (unsigned int)Util::inputLayout.size() };
    psoDesc.pRootSignature = renderPass.rst;
    psoDesc.VS = {nullptr, 0};
    psoDesc.GS = {nullptr, 0};
    psoDesc.PS = {nullptr, 0};

    // compile shaders
    for(const auto& shader: data.shaders){
        ID3DBlob* sh = Device::GetShader(shader);
        if(shader.type==ShaderEnum::VS){
            psoDesc.VS = {
                reinterpret_cast<BYTE*>(sh->GetBufferPointer()), 
	            sh->GetBufferSize() 
            };
        }
        else if(shader.type==ShaderEnum::PS){
            psoDesc.PS = {
                reinterpret_cast<BYTE*>(sh->GetBufferPointer()), 
	            sh->GetBufferSize() 
            };
        }
    }

    // set up render targets
    psoDesc.NumRenderTargets = 0;
    for(const auto& out: data.outputs){
        if(out.format == ResourceEnum::Format::R32G32B32A32_FLOAT){
            psoDesc.RTVFormats[psoDesc.NumRenderTargets] = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
        else if(out.format == ResourceEnum::Format::R8G8B8A8_UNORM){
            psoDesc.RTVFormats[psoDesc.NumRenderTargets] = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        else if(out.format == ResourceEnum::Format::R16G16_FLOAT){
            psoDesc.RTVFormats[psoDesc.NumRenderTargets] = DXGI_FORMAT_R16G16_FLOAT;
        }
        // @TODO: else
        psoDesc.NumRenderTargets++;
    }

    // set up depth stencil
    if(data.psoData.enableDepth){
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    }
    else{
        psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
    }

    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.FrontCounterClockwise = true;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));

    renderPass.pso = pso.Get();

    // keep ref
    psos.push_back(pso);
}

void Device::ExecuteRenderPass(RenderPass& renderPass, const PassData& data)
{   
    // pso, rst, renderTargets
    Context::GetContext()->SetPipelineState(renderPass.pso);
    Context::GetContext()->SetGraphicsRootSignature(renderPass.rst);

    // output, render targets and depth stencil
    // @TODO: set render resolution
    if(data.outputs.size()!=0){
        std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rts;
        // depth buffer cpu handle
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;
        for(const auto& out: data.outputs){
            CD3DX12_CPU_DESCRIPTOR_HANDLE view = resMgr->GetCPU(out.name, ResourceEnum::View::RTView);
            if(out.type != ResourceEnum::View::DSView)
                rts.push_back(view);
            else 
                dsv = view;
        }
        if(data.psoData.enableDepth)
            Context::GetContext()->OMSetRenderTargets(rts.size(), rts.data(), false, &dsv);
        else
            Context::GetContext()->OMSetRenderTargets(rts.size(), rts.data(), false, nullptr);
    }

    // input, graphics root params
    for(unsigned int i = 0; i < (int)data.inputs.size(); i++){
        const auto& in = data.inputs[i];
        // @TODO: set graphics root params
        if(in.type == ResourceEnum::Type::Constant){
            ID3D12Resource* res = resMgr->GetResource(in.name);
            Context::GetContext()->SetGraphicsRootConstantBufferView(i, res->GetGPUVirtualAddress());
        }
        else if(in.type == ResourceEnum::Type::Texture2D || in.type == ResourceEnum::Type::TextureCube){
            CD3DX12_GPU_DESCRIPTOR_HANDLE view = resMgr->GetGPU(in.name, ResourceEnum::View::SRView);
            Context::GetContext()->SetGraphicsRootDescriptorTable(i, view);
        }
        // @TODO: buffer bounding
        else if(in.type == ResourceEnum::Type::Buffer){
            
        }
    }
}

template<typename T>
void Device::SetShaderConstant(std::string name, T* data)
{
    resMgr->CommitConstantBuffer(name, data);
}


