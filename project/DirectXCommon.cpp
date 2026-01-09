#include "DirectXCommon.h"
#include <cassert>
#include <format>
#include <windows.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

void DirectXCommon::Initialize(WinApp* winApp) {
	assert(winApp);
	this->winApp_ = winApp;
}

void DirectXCommon::DeviceInitialize(Microsoft::WRL::ComPtr<ID3D12Device> device_, Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_)
{
	HRESULT hr;

#ifdef _DEBUG

	ID3D12Debug1 *debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

	// DXGIファクトリーの生成
	hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか
	// どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	IDXGIAdapter4 *useAdapter = nullptr;
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(
		i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND;
		++i) {
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できないのは一大事
		// ソフトウェアアダプタ出なければ採用!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。wstringの方なので注意
			break;
		}
		useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
	}
	// 適切なアダプタが見つからなっかったので起動できない
	assert(useAdapter != nullptr);

	D3D_FEATURE_LEVEL featureLevels[] = {
	D3D_FEATURE_LEVEL_12_2,
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0
	};

	const char *featureLevelStrings[] = {
		"12.2", "12.1", "12.0"
	};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(
			useAdapter,
			featureLevels[i],
			IID_PPV_ARGS(device_.ReleaseAndGetAddressOf())
		);

		if (SUCCEEDED(hr)) {
			// ログ出力（例）
			OutputDebugStringA(
				std::format("FeatureLevel : {}\n", featureLevelStrings[i]).c_str()
			);
			break;
		}
	}

	// 生成失敗は致命的
	assert(device_ != nullptr);
	OutputDebugStringA("Complete create D3D12Device!!!\n");

#ifdef _DEBUG

	ID3D12InfoQueue *infoQueue = nullptr;	
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {

		infoQueue->SetBreakOnSeverity(
			D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(
			D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(
			D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		D3D12_MESSAGE_SEVERITY severities[] = {
			D3D12_MESSAGE_SEVERITY_INFO
		};

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);
		infoQueue->Release();
	}

#endif
}

void DirectXCommon::CommandInitialize()
{
	HRESULT hr;

	// コマンドアロケータを生成する
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator_, nullptr,
		IID_PPV_ARGS(&commandList_));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device_->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

}

void DirectXCommon::SwapchainInitialize()
{
	HRESULT hr;

	// スワップチェーンを生成する
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width =
		WinApp::kClientWidth_; // 画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height =
		WinApp::kClientHeight_; // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
	swapChainDesc.SampleDesc.Count = 1;                // マルチサンプルしない
	swapChainDesc.BufferUsage =
		DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;       // ダブルバッファ
	swapChainDesc.SwapEffect =
		DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタにうつしたら、中身を破棄
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory_->CreateSwapChainForHwnd(
		commandQueue_, winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain_));
	assert(SUCCEEDED(hr));
}

void DirectXCommon::depthBufferInitialize()
{

}
