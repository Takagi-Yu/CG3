#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"

class DirectXCommon
{
public: // メンバ関数
	void Initialize(WinApp* winApp);
	void DeviceInitialize(Microsoft::WRL::ComPtr<ID3D12Device> device_, Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_);
	void CommandInitialize();
	void SwapchainInitialize();
	void depthBufferInitialize();

public: // メンバ変数
	WinApp* winApp_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

	ID3D12CommandAllocator* commandAllocator_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	ID3D12CommandQueue* commandQueue_ = nullptr;

	IDXGISwapChain4* swapChain_ = nullptr;

};

