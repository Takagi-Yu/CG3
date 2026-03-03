#pragma once
#include <wrl.h>
#include <string>
#include <d3d12.h>
#include <vector>

#include "externals/DirectXTex/DirectXTex.h"


class DirectXCommon;

class TextureManager
{
public:
	static TextureManager *GetInstance();

	void Initialize(DirectXCommon *dxCommon);

	void LoadTexture(const std::string &filePath);

	uint32_t GetTextureIndexByFilePath(const std::string &filePath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	const DirectX::TexMetadata &GetMetaData(uint32_t textureIndex);

	void Filalize();

private:
	static uint32_t kSRVIndexTop_;
	static TextureManager *instance_;

	DirectXCommon *dxCommonPtr_ = nullptr;

	struct TextureData {
		std::string filePath_;
		DirectX::TexMetadata metadata_{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU_{};
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU_{};
	};

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager &) = default;
	TextureManager &operator=(TextureManager &) = default;

	std::vector<TextureData> textureDatas_;

	ID3D12Device *device_;
	ID3D12GraphicsCommandList *commandList_;
};

