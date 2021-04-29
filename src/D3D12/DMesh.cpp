#include "DMesh.hpp"
#include "d3dUtil.h"

D3D12_VERTEX_BUFFER_VIEW DMesh::VertexBufferView()const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = VertexByteStride;
	vbv.SizeInBytes = VertexBufferByteSize;
	return vbv;
}

D3D12_INDEX_BUFFER_VIEW DMesh::IndexBufferView()const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = IndexFormat;
	ibv.SizeInBytes = IndexBufferByteSize;
	return ibv;
}

void DMesh::DisposeUploaders()
{
	VertexBufferUploader = nullptr;
	IndexBufferUploader = nullptr;
}

void DMesh::BuildVertexAndIndexBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, std::vector<Vertex> vs, std::vector<unsigned int> ids)
{
    unsigned int vsSize = (unsigned int) sizeof(Vertex) * vs.size();
    unsigned int idsSize = (unsigned int) sizeof(unsigned int) * ids.size();

    VertexBufferGPU = d3dUtil::CreateDefaultBuffer(device, cmdList, vs.data(), vsSize, VertexBufferUploader);

	IndexBufferGPU = d3dUtil::CreateDefaultBuffer(device, cmdList, ids.data(), idsSize, IndexBufferUploader);

	VertexByteStride = sizeof(Vertex);
	VertexBufferByteSize = vsSize;
	IndexFormat = DXGI_FORMAT_R32_UINT;
	IndexBufferByteSize = idsSize;
}