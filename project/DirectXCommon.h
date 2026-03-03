#pragma once
#include "WinApp.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <dxcapi.h>
#include <string>
#include <chrono>

#include "externals/DirectXTex/DirectXTex.h"

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

	void ExecuteCommandList();
	void WaitForSignal();
	void CommandReset();

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &descriptorHeap, uint32_t descriptorSize, uint32_t index);
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &descriptorHeap, uint32_t descriptorSize, uint32_t index);
	Microsoft::WRL::ComPtr<IDxcBlob>CompileShader(const std::wstring &filePath, const wchar_t *profile, std::ostream &os);
	Microsoft::WRL::ComPtr<ID3D12Resource>CreateBufferResource(size_t sizeInBytes);
	Microsoft::WRL::ComPtr<ID3D12Resource>CreateTextureResource(const DirectX::TexMetadata &metadata);

	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource> &texture, const DirectX::ScratchImage &mipImages);
	static DirectX::ScratchImage LoadTexture(const std::string &filePath);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	// getter
	ID3D12Device *GetDevice()const { return device_.Get(); }
	ID3D12GraphicsCommandList *GetCommandList()const { return commandList_.Get(); }
	
	static const uint32_t kMaxSRVCount;

private: // メンバ変数
	WinApp* winApp_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;

	int32_t width_ = 1280;
	int32_t height_ = 720;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;

	uint32_t rtvDescriptorSize_ = 0;
	uint32_t dsvDescriptorSize_ = 0;
	uint32_t srvDescriptorSize_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	UINT64 fenceVal_ = 0;
	HANDLE fenceEvent_ = {};

	// FPS固定初期化
	void InitializeFixFPS();
	// FPS固定更新
	void UpdateFixFPS();
	std::chrono::steady_clock::time_point referrence_;
};

