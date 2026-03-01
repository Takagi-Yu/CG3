#include "TextureManager.h"
#include "DirectXCommon.h"
#include "StringUtility.h"

using namespace StringUtility;

TextureManager *TextureManager::instance_ = nullptr;

uint32_t TextureManager::kSRVIndexTop_ = 1;

TextureManager *TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new TextureManager;
	}
	return instance_;
}

void TextureManager::Initialize(DirectXCommon *dxCommon) {
	textureDatas_.reserve(DirectXCommon::kMaxSRVCount);
}

void TextureManager::LoadTexture(const std::string &filePath) {
	auto it = std::find_if(textureDatas_.begin(), textureDatas_.end(), [&](TextureData &textureData)
		{return textureData.filePath_ == filePath; });
	if (it != textureDatas_.end()) {
		return;
	}

	assert(textureDatas_.size() + kSRVIndexTop_ < DirectXCommon::kMaxSRVCount);

	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filepathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(
		filepathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	// ミニマップの作成
	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(),
		image.GetMetadata(), DirectX::TEX_FILTER_SRGB,
		0, mipImage);
	assert(SUCCEEDED(hr));

	textureDatas_.reserve(textureDatas_.size() + 1);
	TextureData &textureData = textureDatas_.back();
	textureData.filePath_ = filePath;
	textureData.metadata_ = mipImage.GetMetadata();
	textureData.resource_ = dxCommonPtr_->CreateTextureResource(textureData.metadata_);

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas_.size() - 1) + kSRVIndexTop_;

	textureData.srvHandleCPU_ = dxCommonPtr_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU_ = dxCommonPtr_->GetSRVGPUDescriptorHandle(srvIndex);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	// CG2参考の1559行目付近を参考に続きを書く
	srvDesc.Format = textureData.metadata_.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata_.mipLevels);

	device_ = dxCommonPtr_->GetDevice();
	commandList_ = dxCommonPtr_->GetCommandList();
	device_->CreateShaderResourceView(textureData.resource_.Get(), &srvDesc, textureData.srvHandleCPU_);
	
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommonPtr_->UploadTextureData(textureData.resource_, mipImage);
	dxCommonPtr_->ExecuteCommandList();
	dxCommonPtr_->WaitForSignal();
	dxCommonPtr_->CommandReset();

	intermediateResource->Release();
}

uint32_t TextureManager::GetTextureIndexByFilePath(const std::string &filePath)
{
	auto it = std::find_if(textureDatas_.begin(), textureDatas_.end(), [&](TextureData &textureData)
		{return textureData.filePath_ == filePath; });

	if (it != textureDatas_.end()) {
		uint32_t textureIndex = 
			static_cast<uint32_t>(std::distance(textureDatas_.begin(), it));
	}
	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex)
{
	assert(textureIndex);

	TextureData &textureData = textureDatas_[textureIndex];
	return textureData.srvHandleGPU_;
}

void TextureManager::Filalize(){
	delete instance_;
	instance_ = nullptr;
}
