#include "TextureManager.h"
#include "DirectXCommon.h"
#include "StringUtility.h"

using namespace StringUtility;

TextureManager *TextureManager::instance_ = nullptr;

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

	uint32_t srvIndex = static_cast<uint32_t>(textureDatas_.size() - 1);

	textureData.srvHandleCPU_ = dxCommonPtr_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU_ = dxCommonPtr_->GetSRVGPUDescriptorHandle(srvIndex);
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	// CG2参考の1559行目付近を参考に続きを書く

}

void TextureManager::Filalize(){
	delete instance_;
	instance_ = nullptr;
}
