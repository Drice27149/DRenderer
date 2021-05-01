#include <windows.h>
#include <wrl.h>
#include <vector>
// #include <dxgi1_4.h>
#include "d3d12.h"
#include "Global.hpp"
#include "FrameResource.h"

class DMesh
{
public:
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const;
	void DisposeUploaders();
    void BuildVertexAndIndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<Vertex> vs, std::vector<unsigned int> ids);

public:
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	D3D12_GPU_VIRTUAL_ADDRESS meshCB;
};