#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "d3dx12.h"
#include "d3dUtil.h"

using namespace Microsoft::WRL;

namespace ResFatory {
	void CreateCubeMapResource(ComPtr<ID3D12Resource>& resource, unsigned int width, unsigned int height, unsigned int mipLevels = 1);
	void CreateTexture2DResource(ComPtr<ID3D12Resource>& resource, ComPtr<ID3D12Resource>& uploadBuffer, std::string fn);
	void CreateRenderTarget2DResource(ComPtr<ID3D12Resource>& resource, unsigned int width, unsigned int height, DXGI_FORMAT format);
};
