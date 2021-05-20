#pragma once

#include "d3dUtil.h"

class Resource
{
public:
	Resource(ID3D12Device* device, DXGI_FORMAT format, UINT width, UINT height);
    Resource(ID3D12Device* device, D3D12_RESOURCE_DESC desc);
	Resource(
		ID3D12Device* device, 
		unsigned int width, 
		unsigned int height, 
		CD3DX12_CPU_DESCRIPTOR_HANDLE createHandle,
		CD3DX12_GPU_DESCRIPTOR_HANDLE readHandle,
		CD3DX12_CPU_DESCRIPTOR_HANDLE writeHandle
	);
	Resource(ID3D12Device* device, ID3D12GraphicsCommandList*  commandList):md3dDevice(device), commandList(commandList){}

	Resource(const Resource& rhs)=delete;
	Resource& operator=(const Resource& rhs)=delete;
	~Resource()=default;

    UINT Width()const;
    UINT Height()const;
	ID3D12Resource* GetResource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv()const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv()const;

	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv);
	void OnResize(UINT newWidth, UINT newHeight);

private:
	void BuildDescriptors();
	void BuildResource();

private:

	ID3D12Device* md3dDevice = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;
	UINT mWidth = 0;
	UINT mHeight = 0;

	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS; // DXGI_FORMAT_R24G8_TYPELESS;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;
public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE createHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE readHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE writeHandle;

	Microsoft::WRL::ComPtr<ID3D12Resource> mResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> CntResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer = nullptr;
	// New
public:
	void BuildRenderTargetArray(unsigned int number, DXGI_FORMAT format);
	CD3DX12_GPU_DESCRIPTOR_HANDLE ReadHandle()const { return readHandle; }
	CD3DX12_CPU_DESCRIPTOR_HANDLE WriteHandle()const { return writeHandle; }
	// New and decouple
public:
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvCpu;
	CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu;
	CD3DX12_GPU_DESCRIPTOR_HANDLE srvGpu;
	CD3DX12_CPU_DESCRIPTOR_HANDLE xxxCpu;

	Resource(ID3D12Device* device, unsigned int width, unsigned int height);
	void BuildDepthMap(DXGI_FORMAT resFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT xxxFormat);
	void BuildUAV(unsigned int elements, unsigned int size, bool haveCounter);
	// hack: put implementation in header files to avoid link error
	template<typename T> void BuildStructureBuffer(unsigned int elements, unsigned int size, T* data)
	{
		auto byteSize = elements * size;
		mResource = d3dUtil::CreateDefaultBuffer(md3dDevice, commandList, data, byteSize, uploadBuffer);
	}

	ID3D12Resource* GetCounterResource() { return CntResource.Get(); }

	void BuildTextureResource(std::string fn);
	void BuildRenderTarget(unsigned int width, unsigned int height, DXGI_FORMAT format, bool enableDepth);
	void AppendTexture2DUAV(unsigned int width, unsigned int height, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE uavCpu);
	void AppendTexture2DSRV(unsigned int width, unsigned int height, DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE srvCpu);
	void BuildRenderTarget(unsigned int width, unsigned int height, DXGI_FORMAT format);
	void AppendTexture2DRTV(DXGI_FORMAT format, CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCpu);
};

 