#pragma once
#include "DirectXCommon.h"
#include "MyMath.h"
#include "TextureManager.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>

class SpriteCommon;

class Sprite
{
public:
	struct Transform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};

	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};

	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

public:
	void Initialize(SpriteCommon *spriteCommon,DirectXCommon *dxCommon, std::string textureFilePath);

	void Update();

	void Draw();

public:
	void SetUseTexture(const std::string &filePath);
	void SetUseTexture(uint32_t textureIndex) { textureIndex_ = textureIndex; }
	void SetSRVHandleGPU(D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU) { textureSrvHandleGPU_ = textureSrvHandleGPU; }
	
	const Vector2 &GetPosition()const { return position_; }
	void SetPosition(const Vector2 &position) { position_ = position; }
	float GetRotation()const { return rotation_; }
	void SetRotation(float rotation) { rotation_ = rotation; }
	const Vector4 &GetColor()const { return materialData_->color; }
	void SetColor(const Vector4 &color) { materialData_->color = color; }
	const Vector2 &GetSize()const { return size_; }
	void SetSize(const Vector2 &size) { size_ = size; }
	const Vector2 &GetAnchorPoint()const { return anchorPoint_; }
	void SetAnchorPoint(const Vector2 &anchorPoint) { anchorPoint_ = anchorPoint; }
	const bool &GetIsFlipX()const { return isFlipX_; }
	void SetIsFlipX(const bool &isFlipX) { isFlipX_ = isFlipX; }
	const bool &GetIsFlipY()const { return isFlipY_; }
	void SetIsFlipY(const bool &isFlipY) { isFlipY_ = isFlipY; }

	const Vector2 &GetTextureLeftTop()const { return textureLeftTop_; }
	void SetTextureLeftTop(const Vector2 &textureLeftTop) { textureLeftTop_ = textureLeftTop; }
	const Vector2 &GetTextureSize()const { return textureSize_; }
	void SetTextureSize(const Vector2 &textureSize) { textureSize_ = textureSize; }

private:
	void AdjustTextureSize();

private:
	DirectXCommon *dxCommon_ = nullptr;
	SpriteCommon *spriteCommon_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>indexResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>materialResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>transformationMatrixResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>textureResource_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	VertexData *vertexData_ = nullptr;
	uint32_t *indexData_ = nullptr;
	Material *materialData_ = nullptr;
	TransformationMatrix *transformationMatrixData_ = nullptr;

	uint32_t textureIndex_ = 0;

	Transform transform_{};
	Transform uvTransform_{};

	const int32_t kClientWidth_ = 1280;
	const int32_t kClientheight_ = 720;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap_ = nullptr;
	uint32_t srvDescriptorSize_ = 0;

	Vector2 position_ = { 0.0f,0.0f };
	float rotation_ = 0.0f;
	Vector2 size_ = { 180.0f,180.0f };

	Vector2 anchorPoint_ = { 0.5f,0.5f };

	bool isFlipX_ = false;
	bool isFlipY_ = false;

	Vector2 textureLeftTop_ = { 0.0f,0.0f };
	Vector2 textureSize_ = { 100.0f,100.0f };
};

