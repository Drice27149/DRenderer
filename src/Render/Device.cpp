#include "Device.hpp"
#include "RenderStruct.hpp"
#include "Util.hpp"
#include "Struct.hpp"
#include "RenderPass.hpp"
#include "Context.hpp"
#include "ResourceManager.hpp"
#include "Renderer.hpp"

ComPtr<ID3D12Device> Device::GDevice = nullptr;

void Device::SetDevice(ComPtr<ID3D12Device> device)
{
    GDevice = device;
}

ID3D12Device* Device::GetDevice()
{
    return GDevice.Get();
}

std::map<std::string, ComPtr<ID3DBlob>>& Device::GetShaderMap(ShaderEnum::Type type)
{
    if(type == ShaderEnum::Type::CS)
        return css;
    else if(type == ShaderEnum::Type::VS)
        return vss;
    else if(type == ShaderEnum::Type::GS)
        return gss;
    else if(type == ShaderEnum::Type::PS)
        return pss;
    else if(type == ShaderEnum::Type::CS)
        return css;
    return css;
}

ID3DBlob* Device::GetShader(ShaderData data)
{
    auto& shaders = GetShaderMap(data.type);
    
    if(shaders.count(data.name))
        return shaders[data.name].Get();
    if(data.type == ShaderEnum::Type::CS)
        shaders[data.name] = d3dUtil::CompileShader(WString(data.name).c_str(), nullptr, "CS", "cs_5_1");
    else if(data.type == ShaderEnum::Type::GS)
        shaders[data.name] = d3dUtil::CompileShader(WString(data.name).c_str(), nullptr, "GS", "gs_5_1");
    else if(data.type == ShaderEnum::Type::PS)
        shaders[data.name] = d3dUtil::CompileShader(WString(data.name).c_str(), nullptr, "PS", "ps_5_1");
    else
        shaders[data.name] = d3dUtil::CompileShader(WString(data.name).c_str(), nullptr, "VS", "vs_5_1");
    return shaders[data.name].Get();
}

ID3D12RootSignature* Device::CreateRSt(const PassData& data, const std::string& name)
{
    if(rstCache.count(name))
        return rstCache[name];

    ComPtr<ID3D12RootSignature> rst;

    std::vector<CD3DX12_ROOT_PARAMETER> rootParams;
    unsigned cbv = 0;
    unsigned srv = 0;
    CD3DX12_DESCRIPTOR_RANGE desc[32];
    int cdesc = -1;
    for(const auto& input: data.inputs){
        if(input.state == ResourceEnum::State::Read) {
            if(input.type==ResourceEnum::Type::Constant){
                CD3DX12_ROOT_PARAMETER param;
                param.InitAsConstantBufferView(cbv++);
                rootParams.push_back(param);
            }
            else if(input.type==ResourceEnum::Type::Texture2D || input.type==ResourceEnum::Type::TextureCube){
                desc[++cdesc].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, srv++);
                CD3DX12_ROOT_PARAMETER param;
                param.InitAsDescriptorTable(1, &(desc[cdesc]) , D3D12_SHADER_VISIBILITY_PIXEL);
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

    return rstCache[name] = rst.Get();
}

void Device::SetUpRenderPass(RenderPass& renderPass, const PassData& data, const std::string& name)
{
    // create root signature
    renderPass.rst = Device::CreateRSt(data, name);

    if(psoCache.count(name))
        renderPass.pso = psoCache[name];
    
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

    // set up blend state
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    if(data.psoData.enableAdd){
        psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
        psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    }

    // set up rasterizer state, conservative rasterization on/off
    if(data.psoData.conservative){
        D3D12_RASTERIZER_DESC rasterDescFront;
        rasterDescFront.AntialiasedLineEnable = FALSE;
        rasterDescFront.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        rasterDescFront.CullMode = D3D12_CULL_MODE_NONE;
        rasterDescFront.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rasterDescFront.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterDescFront.DepthClipEnable = TRUE;
        rasterDescFront.FillMode = D3D12_FILL_MODE_SOLID;
        rasterDescFront.ForcedSampleCount = 0;
        rasterDescFront.FrontCounterClockwise = FALSE;
        rasterDescFront.MultisampleEnable = FALSE;
        rasterDescFront.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        psoDesc.RasterizerState = rasterDescFront;
    }
    else{
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.FrontCounterClockwise = true;
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    }
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    
    ThrowIfFailed(Device::GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));

    renderPass.pso = psoCache[name] = pso.Get();

    // keep ref, may be very slow
    psos.push_back(pso);
}

void Device::ExecuteRenderPass(RenderPass& renderPass, const PassData& data)
{   
    // pso, rst, renderTargets
    Context::GetContext()->SetPipelineState(renderPass.pso);
    Context::GetContext()->SetGraphicsRootSignature(renderPass.rst);

    // resource state transition
    for(const auto& res: data.inputs){
        if(res.type != ResourceEnum::Type::Constant)
            Renderer::ResManager->ResourceBarrier(res.name, res.state);
    }

    for(const auto& res: data.outputs){
        Renderer::ResManager->ResourceBarrier(res.name, res.state);
    }
    // depth stencil state transition
    if(data.psoData.enableDepth){
        Renderer::ResManager->ResourceBarrier(data.psoData.depthStencil.name, data.psoData.depthStencil.state);
    }

    // output, render targets and depth stencil
    std::vector<CD3DX12_CPU_DESCRIPTOR_HANDLE> rts;
    for(const auto& out: data.outputs){
        CD3DX12_CPU_DESCRIPTOR_HANDLE view = Renderer::ResManager->GetCPU(out.name, ResourceEnum::View::RTView);
        rts.push_back(view);
    }
    if(data.psoData.enableDepth){
        CD3DX12_CPU_DESCRIPTOR_HANDLE dsv = Renderer::ResManager->GetCPU(data.psoData.depthStencil.name, ResourceEnum::View::DSView);
        Context::GetContext()->OMSetRenderTargets(rts.size(), rts.data(), false, &dsv);
    }
    else
        Context::GetContext()->OMSetRenderTargets(rts.size(), rts.data(), false, nullptr);
    if(data.psoData.width > 0 && data.psoData.height > 0){
        D3D12_VIEWPORT screenViewport; 
        D3D12_RECT scissorRect;

        screenViewport.TopLeftX = 0;
        screenViewport.TopLeftY = 0;
        screenViewport.Width    = (float)(data.psoData.width);
        screenViewport.Height   = (float)(data.psoData.height);
        screenViewport.MinDepth = 0.0f;
        screenViewport.MaxDepth = 1.0f;
        scissorRect = { 0, 0, (long)data.psoData.width, (long)data.psoData.height };

        // viewport information for viewport transform/culling
        Context::GetContext()->RSSetViewports(1, &screenViewport);
        // scissor test: self define culling
        Context::GetContext()->RSSetScissorRects(1, &scissorRect);
    }
    

    // input, graphics root params
    for(unsigned int i = 0; i < (int)data.inputs.size(); i++){
        const auto& in = data.inputs[i];
        // @TODO: set graphics root params
        if(in.type == ResourceEnum::Type::Constant){
            ID3D12Resource* res = Renderer::ResManager->GetResource(in.name);
            Context::GetContext()->SetGraphicsRootConstantBufferView(i, res->GetGPUVirtualAddress());
        }
        else if(in.type == ResourceEnum::Type::Texture2D || in.type == ResourceEnum::Type::TextureCube){
            CD3DX12_GPU_DESCRIPTOR_HANDLE view = Renderer::ResManager->GetGPU(in.name, ResourceEnum::View::SRView);
            if(view.ptr)
                Context::GetContext()->SetGraphicsRootDescriptorTable(i, view);
        }
        else if(in.type == ResourceEnum::Type::Texture3D){
            CD3DX12_GPU_DESCRIPTOR_HANDLE view;
            if(in.state == ResourceEnum::State::Write)
                view = Renderer::ResManager->GetGPU(in.name, ResourceEnum::View::UAView);
            else
                view = Renderer::ResManager->GetGPU(in.name, ResourceEnum::View::SRView);
            if(view.ptr)
                Context::GetContext()->SetGraphicsRootDescriptorTable(i, view);
            else   
                assert(0);
        }
        // @TODO: structure buffer bounding
        else if(in.type == ResourceEnum::Type::Buffer){
            
        }
    }
}



