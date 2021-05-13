#include "Resource.hpp"
 
Resource::Resource(ID3D12Device* device, DXGI_FORMAT format, UINT width, UINT height)
{
	md3dDevice = device;

    mFormat = format;

	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };

	BuildResource();
}

Resource::Resource(ID3D12Device* device, D3D12_RESOURCE_DESC desc)
{
    md3dDevice = device;
    mFormat = desc.Format;
    mWidth = desc.Format;
    mHeight = desc.Height;
    mViewport = { 0.0f, 0.0f, (float)mWidth, (float)mHeight, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)mWidth, (int)mHeight };

    ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mResource)));
}

UINT Resource::Width()const
{
    return mWidth;
}

UINT Resource::Height()const
{
    return mHeight;
}

ID3D12Resource*  Resource::GetResource()
{
	return mResource.Get();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE Resource::Srv()const
{
	return mhGpuSrv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Resource::Dsv()const
{
	return mhCpuDsv;
}

D3D12_VIEWPORT Resource::Viewport()const
{
	return mViewport;
}

D3D12_RECT Resource::ScissorRect()const
{
	return mScissorRect;
}

void Resource::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
	                             CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
	                             CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv)
{
	// CpuSrv �ᱻ�������� shader resource view
	// CpuXXv ��������������Ӧ�� view ����, ����д
	// GpuSrv ������ setGraphics(), ������

	// Save references to the descriptors. 
	mhCpuSrv = hCpuSrv;
	mhGpuSrv = hGpuSrv;
    mhCpuDsv = hCpuDsv;

	//  Create the descriptors
	BuildDescriptors();
}

void Resource::OnResize(UINT newWidth, UINT newHeight)
{
	if((mWidth != newWidth) || (mHeight != newHeight))
	{
		mWidth = newWidth;
		mHeight = newHeight;

		BuildResource();

		// New resource, so we need new descriptors to that resource.
		BuildDescriptors();
	}
}
 
void Resource::BuildDescriptors()
{
    // Create SRV to resource so we can sample the shadow map in a shader program.
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; 
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateShaderResourceView(mResource.Get(), &srvDesc, mhCpuSrv);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc; 
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mResource.Get(), &dsvDesc, mhCpuDsv);
}

void Resource::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&mResource)));
}

Resource::Resource(
	ID3D12Device* device, 
	unsigned int width, 
	unsigned int height, 
	CD3DX12_CPU_DESCRIPTOR_HANDLE createHandle,
	CD3DX12_GPU_DESCRIPTOR_HANDLE readHandle,
	CD3DX12_CPU_DESCRIPTOR_HANDLE writeHandle
)
{
	md3dDevice = device;
	mWidth = width;
	mHeight = height;
	mViewport = { 0.0f, 0.0f, (float)mWidth, (float)mHeight, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)mWidth, (int)mHeight };
	this->createHandle = createHandle;
	this->readHandle = readHandle;
	this->writeHandle = writeHandle; 
}

void Resource::BuildRenderTargetArray(unsigned int number, DXGI_FORMAT format)
{
	CD3DX12_RESOURCE_DESC texDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,		// alignment
		mWidth, mHeight, number,
		1,		// mip levels
		format,
		1, 0,	// sample count/quality
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	// Performance tip: Tell the runtime at resource creation the desired clear value. 
	// D3D12_CLEAR_VALUE clearValue;
	// clearValue.Format = format;
	// if (clear_color == 0)
	// {
	// 	float clearColor[] = { 0, 0, 0, 1 };
	// 	memcpy(&clearValue.Color[0], &clearColor[0], 4 * sizeof(float));
	// }
	// else
	// 	memcpy(&clearValue.Color[0], &clear_color[0], 4 * sizeof(float));

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr, // &clearValue,
		IID_PPV_ARGS(&mResource)
	);

	D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
	ZeroMemory(&RTVDesc, sizeof(RTVDesc));
	RTVDesc.Format = format;
	RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
	RTVDesc.Texture2DArray.ArraySize = number;

	md3dDevice->CreateRenderTargetView(mResource.Get(), &RTVDesc, writeHandle);

	D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	SRVDesc.Texture2DArray.ArraySize = number;
	SRVDesc.Texture2DArray.MipLevels = 1;
	SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    md3dDevice->CreateShaderResourceView(mResource.Get(), &SRVDesc, createHandle);
	mResource->SetName(L"renderTargetArray");
}

Resource::Resource(ID3D12Device* device, unsigned int width, unsigned int height)
{
	md3dDevice = device;
	mWidth = width;
	mHeight = height;
	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };
}

void Resource::BuildDepthMap(DXGI_FORMAT resFormat, DXGI_FORMAT srvFormat, DXGI_FORMAT xxxFormat)
{
	// 0. commit resource
	// 1. build shader resource view
	// 2. build xxx view
	// note: create the resource with typeless components, and specialize the resource in a view
	// i.e: resource format = xxx_typeless, view format = xxx_uint 

	// commit resource
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = resFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&mResource)
		)
	);
	mResource->SetName(L"PreZ depthMap");

	// build shader resource view
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = srvFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    srvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateShaderResourceView(mResource.Get(), &srvDesc, srvCpu);

	// build xxx view
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc; 
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = xxxFormat;
    dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mResource.Get(), &dsvDesc, xxxCpu);
}

void Resource::BuildUAV(unsigned int elements, unsigned int size, bool haveCounter)
{
	// uav counter first
	if(haveCounter){
		CD3DX12_RESOURCE_DESC uavCntDesc(
			D3D12_RESOURCE_DIMENSION_BUFFER, 
			0, 
			sizeof(unsigned int), 
			1, 1, 1, 
			DXGI_FORMAT_UNKNOWN, 
			1, 
			0, 
			D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE
		);
		uavCntDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		md3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
			D3D12_HEAP_FLAG_NONE,
			&uavCntDesc,
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			nullptr,
			IID_PPV_ARGS(&CntResource)
		);
	}
	// unorder access view second
    CD3DX12_RESOURCE_DESC ResourceDesc(D3D12_RESOURCE_DIMENSION_BUFFER, 0, elements * size, 1, 1, 1, DXGI_FORMAT_UNKNOWN, 1, 0, D3D12_TEXTURE_LAYOUT_ROW_MAJOR, D3D12_RESOURCE_FLAG_NONE);
	ResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, 0, 0),
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&mResource)
    );

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN; //Needs to be UNKNOWN for structured buffer
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = elements;
	uavDesc.Buffer.StructureByteStride = size; //2 uint32s in struct
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE; //Not a raw view
	uavDesc.Buffer.CounterOffsetInBytes = 0; //First element in UAV counter resource

	md3dDevice->CreateUnorderedAccessView(mResource.Get(), CntResource?CntResource.Get():nullptr, &uavDesc, srvCpu);
}




