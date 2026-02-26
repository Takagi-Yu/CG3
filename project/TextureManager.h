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

	void Filalize();
private:
	static uint32_t kSRVIndexTop;
	static TextureManager *nInstance;

	DirectXCommon *DxCommonPtr = nullptr;

	struct TextureData {
		std::string filePath;
		DirectX::TexMetadata metadata{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU{};
	};

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager &) = default;
	TextureManager &operator=(TextureManager &) = default;

	std::vector<TextureData> textureDatas;
};

