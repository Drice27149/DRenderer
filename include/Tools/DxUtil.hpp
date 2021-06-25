#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include "d3dx12.h"
#include "Fatory.hpp"
#include "Graphics.hpp"

class DxUtil {
public:
	static void BindGraphicsRoot(std::vector<RootEntry>& ens)
	{
		unsigned int index = 0;
		for(auto& en: ens){
			if(en.type == RootType::CBV && en.addr != 0)
				Graphics::GCmdList->SetGraphicsRootConstantBufferView(index, en.addr);
			else if(en.type == RootType::SRV)
				Graphics::GCmdList->SetGraphicsRootDescriptorTable(index, en.handle);
			
			index++;
		}
	}

    static void FullScreenPass()
    {
		Graphics::GCmdList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Graphics::GCmdList->DrawInstanced(6, 1, 0, 0);
    }
};