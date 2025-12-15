#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class DirectXCommon
{
public: // ƒƒ“ƒoŠÖ”
	void Initialize();
	void DeviceInitialize();

public: // ƒƒ“ƒo•Ï”
	IDXGIFactory7 *dxgiFactory_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;

};

