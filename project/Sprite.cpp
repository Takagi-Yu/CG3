#include "Sprite.h"

void Sprite::Initialize(SpriteCommon *spriteCommon, DirectXCommon *dxCommon)
{
	spriteCommon_ = spriteCommon;
    dxCommon_ = dxCommon;

    vertexResource_ =
        dxCommon_->CreateBufferResource(sizeof(VertexData) * 6);

    indexResource_ =
        dxCommon_->CreateBufferResource(sizeof(uint32_t) * 6);

    // 頂点バッファビューを作成する
    // リソースの先頭のアドレスから使う
    vertexBufferView_.BufferLocation =
        vertexResource_->GetGPUVirtualAddress();
    // 使用するリソースのサイズは頂点6つ分のサイズ
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    // 1頂点あたりのサイズ
    vertexBufferView_.StrideInBytes = sizeof(VertexData);

    // リソースの先頭のアドレスから使う
    indexBufferView_.BufferLocation =
        indexResource_->GetGPUVirtualAddress();
    // 使用するリソースのサイズはインデックス6つ分のサイズ
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    // インデックスはuint32_tとする
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

    vertexResource_->Map(0, nullptr,
        reinterpret_cast<void **>(&vertexData_));

    // 1枚目の三角形
    vertexData_[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
    vertexData_[0].texcoord = { 0.0f, 1.0f };
    vertexData_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
    vertexData_[1].texcoord = { 0.0f, 0.0f };
    vertexData_[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
    vertexData_[2].texcoord = { 1.0f, 1.0f };
    // 2枚目の三角形
    vertexData_[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
    vertexData_[3].texcoord = { 1.0f, 0.0f };

    vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };



    // インデックスリソースにデータを書き込む
    indexResource_->Map(0, nullptr,
        reinterpret_cast<void **>(&indexData_));

    indexData_[0] = 0;
    indexData_[1] = 1;
    indexData_[2] = 2;
    indexData_[3] = 1;
    indexData_[4] = 3;
    indexData_[5] = 2;

    // マテリアル用のリソースを作る
    materialResource_ = dxCommon_->CreateBufferResource(sizeof(Material));
    // 書き込むためのアドレスを取得
    materialResource_->Map(0, nullptr, reinterpret_cast<void **>(&materialData_));
    // マテリアルデータの初期値を書き込む
    materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData_->enableLighting = false;
    materialData_->uvTransform = MakeIdentity4x4();

    transformationMatrixResource_ =
        dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
    // 書き込むためのアドレスを取得
    transformationMatrixResource_->Map(
        0, nullptr, reinterpret_cast<void **>(&transformationMatrixData_));

    transformationMatrixData_->WVP = MakeIdentity4x4();
    transformationMatrixData_->World = MakeIdentity4x4();
}

void Sprite::Update() {




    //textureSrvHandleGPU_ =
    //    dxCommon_->GetGPUDescriptorHandle(srvHeap_, srvDescriptorSize_, 2);

    Transform transform{
    {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
    Transform cameraTransform{
    {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} };

    //Matrix4x4 worldMatrix = MakeAffineMatrix(
    //    transform.scale, transform.rotate, transform.translate);
    //Matrix4x4 cameraMatrix =
    //    MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate,
    //        cameraTransform.translate);
    //Matrix4x4 viewMatrix = Inverse(cameraMatrix);
    //Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
    //    0.45f, float(kClientWidth_) / float(kClientheight_), 0.1f, 100.0f);
    //Matrix4x4 worldViewProjectionMatrix =
    //    Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
    //transformationMatrixData_->WVP = worldViewProjectionMatrix;
    //transformationMatrixData_->World = worldMatrix;
      // Sprite用のWorldProjectionMatrixを作る
    Transform transformSprite{
    {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

    Matrix4x4 worldMatrixSprite =
        MakeAffineMatrix(transformSprite.scale, transformSprite.rotate,
            transformSprite.translate);
    Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
    Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(
        0.0f, 0.0f, float(WinApp::kClientWidth_), float(WinApp::kClientHeight_), 0.0f, 100.0f);
    Matrix4x4 worldViewProjectionMatrixSprite =
        Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));

    transformationMatrixData_->WVP = worldViewProjectionMatrixSprite;
    transformationMatrixData_->World = worldMatrixSprite;
}

void Sprite::Draw()
{
    commandList_ = dxCommon_->GetCommandList();
    // マテリアルCBufferの場所を設定
    commandList_->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
    commandList_->IASetIndexBuffer(&indexBufferView_); // VBVを設定
    // Material
    commandList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    // TransformationMatrix
    commandList_->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
    // Texture
    commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
    // DrawCall
    commandList_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
