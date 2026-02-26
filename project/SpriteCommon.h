#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include "DirectXCommon.h"
#include "Logger.h"

using namespace Logger;
class DirectXCommon;

class SpriteCommon
{
public:
	void Initialize(DirectXCommon *dxCommon);

	void PreDraw();

public:
	DirectXCommon *GetDxCommonPtr()const { return mDxCommonptr_; }

private:
	void CreateRootSignature();

	void CreatePipeLineStateObject();

private:
	DirectXCommon *mDxCommonptr_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>graphicsPipelineState_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob>vertexShaderBlob_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcBlob>pixelShaderBlob_ = nullptr;

	ID3D12Device *device_;
	ID3D12GraphicsCommandList *commandList_;

};

