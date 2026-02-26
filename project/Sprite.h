#pragma once

#include "MyMath.h"
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
	void Initialize(SpriteCommon *spriteCommon,const std::string& filePath);

	void Update();

	void Draw();

public:
	void SetUseTexture(const std::string &filePath);
	void SetUseTexture(uint32_t textureIndex) { useTextureIndex = textureIndex; }

private:
	SpriteCommon *spriteCommonPtr = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>indexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>materialResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>transformationMatrixResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>textureResource = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};

	VertexData *vertexDataPtr = nullptr;
	uint32_t *indexDataPtr = nullptr;
	Material *materialDataPtr = nullptr;
	TransformationMatrix *transformationMatrix = nullptr;

	uint32_t useTextureIndex = 0;

	Transform transform{};
	Transform uvTransform{};
};

