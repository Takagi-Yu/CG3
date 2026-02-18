#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <dxcapi.h>

class DirectXCommon
{
public: // メンバ関数
	void Initialize(WinApp* winApp);
	void DeviceInitialize(Microsoft::WRL::ComPtr<ID3D12Device> device, Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory);
	void CommandInitialize();
	void CreateSwapchain();
	void CreateDepthBuffer();
	void CreateDescriptorHeap();
	void RenderTargetviewInitialize();
	void DepthStencilViewInitialize();
	void FenceInitialize();
	void ViewportInitialize();
	void ScissorRectInitialize();
	void DXCcompilerInitialize();
	void ImGuiInitialize();

	// 描画前処理
	void PreDraw();
	// 描画後処理
	void PostDraw();


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	
	//std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> *swapChainResources_;

private: // メンバ変数
	WinApp* winApp_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

	ID3D12CommandAllocator* commandAllocator_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;
	ID3D12CommandQueue* commandQueue_ = nullptr;

	IDXGISwapChain4* swapChain_ = nullptr;

	int32_t width_ = 1280;
	int32_t height_ = 720;

	ID3D12Resource* depthStencilResource_ = nullptr;

	ID3D12DescriptorHeap* rtvHeap_ = nullptr;
	ID3D12DescriptorHeap* dsvHeap_ = nullptr;
	ID3D12DescriptorHeap* srvHeap_ = nullptr;

	uint32_t rtvDescriptorSize_ = 0;
	uint32_t dsvDescriptorSize_ = 0;
	uint32_t srvDescriptorSize_ = 0;

	ID3D12Resource* swapChainResources_[2];
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	IDxcUtils *dxcUtils_ = nullptr;
	IDxcCompiler3 *dxcCompiler_ = nullptr;
	IDxcIncludeHandler *includeHandler_ = nullptr;

	ID3D12Fence *fence_ = nullptr;
	UINT64 fenceVal_ = 0;
	HANDLE fenceEvent_ = {};
};

