#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <d3d12.h>
#include <DbgHelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <sstream>
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);
#include "externals/DirectXTex/d3dx12.h"

#include "engine/io/input.h"
#include "engine/base/WinApp.h"
#include "engine/base/DirectXCommon.h"
#include "engine/base/Logger.h"
#include "engine/base/StringUtility.h"
#include "engine/2d/SpriteCommon.h"
#include "engine/2d/Sprite.h"
#include "engine/base/MyMath.h"
#include "engine/base/TextureManager.h"
#include "engine/base/D3DResourceLeakChecker.h"

using namespace StringUtility;
using namespace Logger;

#define M_PI 3.141592f  

#pragma comment(lib, "Dbghelp.lib")

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

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

static LONG WINAPI ExportDump(EXCEPTION_POINTERS *exception) {
	// 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d.dmp", time.wYear,
		time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle =
		CreateFile(filePath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processID(このexeのID)とクラッシュ(例外)の発生したthreadIDを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION maindumpInformation{ 0 };
	maindumpInformation.ThreadId = threadId;
	maindumpInformation.ExceptionPointers = exception;
	maindumpInformation.ClientPointers = TRUE;
	// Dumpを出力。MiniDumpNomalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle,
		MiniDumpNormal, &maindumpInformation, nullptr, nullptr);
	// ほかに関連付けられているSEH例外ハンドラがあれば実行。通常はプロセスする
	return EXCEPTION_EXECUTE_HANDLER;
}

MaterialData LoadMaterialTemplateFile(const std::string &directoryPath, const std::string &filename) {
	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

ModelData LoadObjFile(const std::string &directoryPath, const std::string &filename) {
	ModelData modelData; // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 長短の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];


				VertexData vertex = { position, texcoord, normal };
				modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position, texcoord, normal };
			}
			// 頂点を逆順で登録することで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		} else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;

			modelData.material =
				LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;

	CoInitializeEx(0, COINIT_MULTITHREADED);
	// 誰も捕捉しなかった場合に(Unhedled),補足する関数を登録
	// main関数始まってすぐに登録すると良い
	SetUnhandledExceptionFilter(ExportDump);

	std::filesystem::create_directory("logs");

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

	std::chrono::zoned_time localtime{ std::chrono::current_zone(), nowSeconds };

	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localtime);

	std::string logFilePath = std::string("logs/") + dateString + ".log";

	std::ofstream logStream(logFilePath);

	WinApp *winApp = nullptr;
	winApp = new WinApp();
	winApp->Initialize();

	DirectXCommon *dxCommon = nullptr;
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = dxCommon->GetCommandList();
	Microsoft::WRL::ComPtr<ID3D12Device> device = dxCommon->GetDevice();

	TextureManager::GetInstance()->Initialize(dxCommon);

	TextureManager::GetInstance()->LoadTexture("Resources/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("Resources/monsterBall.png");

	SpriteCommon *spriteCommon = nullptr;
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	std::vector<Sprite *>sprites;
	for (uint32_t i = 0; i < 5; ++i) {
		Sprite *sprite = new Sprite();
		if (i % 2) {
			sprite->Initialize(spriteCommon, dxCommon, "Resources/monsterBall.png");
		} else {
			sprite->Initialize(spriteCommon, dxCommon, "Resources/uvChecker.png");
		}

		Vector2 position[5];
		position[i] = { i * 100.0f, 300.0f };
		sprite->SetPosition(position[i]);
		//sprite->SetSize({ 100, 100 });
		sprite->SetIsFlipX(true);
		sprites.push_back(sprite);
	}

	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource =
		dxCommon->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	Material *materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void **>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource =
		dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight *directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr,
		reinterpret_cast<void **>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = { 1.0f, 0.0f, 0.0f };
	directionalLightData->intensity = 1.0f;

	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width =
		sizeof(VertexData) * 3; // リソースのサイズ。今回はVector4を3頂点分
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

#pragma region Model
	// モデル読み込み
	ModelData modelData = LoadObjFile("Resources", "axis.obj");
	// 頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> modelVertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// 1頂点当たりのサイズ
	D3D12_VERTEX_BUFFER_VIEW modelVertexBufferView{};
	modelVertexBufferView.StrideInBytes = sizeof(VertexData);
	modelVertexBufferView.BufferLocation = modelVertexResource->GetGPUVirtualAddress();			// リソースの先頭のアドレスから使う
	modelVertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size());	// 使用するリソースのサイズは頂点のサイズ
	modelVertexBufferView.StrideInBytes = sizeof(VertexData);									// 1頂点当たりのサイズ

	VertexData *modelVertexData = nullptr;
	modelVertexResource->Map(0, nullptr, reinterpret_cast<void **>(&modelVertexData));			// 書き込むためのアドレスを取得

	std::memcpy(modelVertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size()); // 頂点リソースにコピー

#pragma endregion

#pragma region Sphere
	//// 頂点リソースにデータを書き込む
	//// =======
	//// 球
	//// =======
	const uint32_t kSubdivision = 8; // 分割数
	uint32_t kSphereVertexNum = (kSubdivision + 1) * (kSubdivision + 1);
	uint32_t kSphereIndexNum = kSubdivision * kSubdivision * 6;

	Microsoft::WRL::ComPtr<ID3D12Resource> sphereVertexResource =
		dxCommon->CreateBufferResource(sizeof(VertexData) * kSphereVertexNum);

	VertexData *sphereVertexData = nullptr;
	sphereVertexResource->Map(0, nullptr, reinterpret_cast<void **>(&sphereVertexData)); // 書き込むためのアドレスを取得
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW sphereVertexBufferView{};
	// リソースの先頭のアドレスから使う
	sphereVertexBufferView.BufferLocation = sphereVertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	sphereVertexBufferView.SizeInBytes = sizeof(VertexData) * kSphereVertexNum;
	// 経度分割1つ分の角度
	const float kLonEvery = 2.0f * M_PI / float(kSubdivision);
	// 緯度分割1つ分の角度
	const float kLatEvery = M_PI / float(kSubdivision);

	for (uint32_t latIndex = 0; latIndex < kSubdivision + 1; ++latIndex) {
		// 緯度(南北)
		float lat = -M_PI / 2.0f + kLatEvery * latIndex;
		float nextLat = lat + kLatEvery;

		float v0 = 1.0f - float(latIndex) / float(kSubdivision);
		float v1 = 1.0f - float(latIndex + 1) / float(kSubdivision);
		// 経度の方向に分割しながら線を引く
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision + 1; ++lonIndex) {
			uint32_t start = (latIndex * (kSubdivision + 1) + lonIndex);
			// 経度(東西)
			float lon = lonIndex * kLonEvery; // 現在
			float nextLon = lon + kLonEvery;  // 次

			float u0 = float(lonIndex) / float(kSubdivision);
			float u1 = float(lonIndex + 1) / float(kSubdivision);

			VertexData vertA{};
			vertA.position = { cos(lat) * cos(lon), sin(lat), cos(lat) * sin(lon),
							  1.0f };
			vertA.texcoord = { u0, v0 };
			vertA.normal = {
				vertA.position.x,
				vertA.position.y,
				vertA.position.z,
			};

			// 頂点にデータを入力する。基準点a
			sphereVertexData[start + 0] = vertA;
		}
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> sphereIndexResource =
		dxCommon->CreateBufferResource(sizeof(uint32_t) * kSphereIndexNum);

	D3D12_INDEX_BUFFER_VIEW sphereIndexBufferView{};
	sphereIndexBufferView.BufferLocation =
		sphereIndexResource->GetGPUVirtualAddress();
	sphereIndexBufferView.SizeInBytes = sizeof(uint32_t) * kSphereIndexNum;
	sphereIndexBufferView.Format = DXGI_FORMAT_R32_UINT;

	uint32_t *sphereIndexData = nullptr;
	sphereIndexResource->Map(0, nullptr,
		reinterpret_cast<void **>(&sphereIndexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			uint32_t lb =
				latIndex * (kSubdivision + 1) + lonIndex; // 左下のインデックス
			uint32_t rb = lb + 1;                         // 右下のインデックス
			uint32_t lt =
				(latIndex + 1) * (kSubdivision + 1) + lonIndex; // 左上のインデックス
			uint32_t rt = lt + 1;                               // 右上のインデックス

			sphereIndexData[start + 0] = lb;
			sphereIndexData[start + 1] = lt;
			sphereIndexData[start + 2] = rb;
			sphereIndexData[start + 3] = rb;
			sphereIndexData[start + 4] = lt;
			sphereIndexData[start + 5] = rt;
		}
	}

	// WVP用のリソースを作る。MAtrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixSphere = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix *transformationMatrixDataSphere = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixSphere->Map(0, nullptr,
		reinterpret_cast<void **>(&transformationMatrixDataSphere));
	// 単位行列を書き込んでおく
	transformationMatrixDataSphere->WVP = MakeIdentity4x4();
	transformationMatrixDataSphere->World = MakeIdentity4x4();
#pragma endregion

#pragma region Sprite
	// Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxCommon->CreateBufferResource(sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData *vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr,
		reinterpret_cast<void **>(&vertexDataSprite));
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxCommon->CreateBufferResource(sizeof(uint32_t) * 6);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t *indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void **>(&indexDataSprite));
	indexDataSprite[0] = 0;
	indexDataSprite[1] = 1;
	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;
	indexDataSprite[4] = 3;
	indexDataSprite[5] = 2;

	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite =
		dxCommon->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix *transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(
		0, nullptr, reinterpret_cast<void **>(&transformationMatrixDataSprite));
	// 単位行列を書き込んでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();

	// sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite =
		dxCommon->CreateBufferResource(sizeof(Material));
	Material *materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr,
		reinterpret_cast<void **>(&materialDataSprite));
	materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// SpriteはLightingしないのでfalseを設定する
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();

	Transform uvTransformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};
#pragma endregion 

	// Transform変数を作る
	Transform transform{
		{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	Transform cameraTransform{
		{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f} };
	Transform transformSprite{
		{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxCommon->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxCommon->GetSRVGPUDescriptorHandle(1);

	for (uint32_t i = 0; i < 5; ++i) {
		sprites[i]->SetSRVHandleGPU(textureSrvHandleGPU);
	}

	// DescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV =
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV =
		device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxCommon->GetSRVCPUDescriptorHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxCommon->GetSRVGPUDescriptorHandle(2);
	bool texture = true;

	MSG msg{};
	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (winApp->ProcessMessage()) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			//} else {
			break;
		}
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// ゲームの処理

		for (uint32_t i = 0; i < 5; ++i) {
			sprites[i]->SetColor(Vector4(1, 0, 0, 1));
			//sprites[i]->SetRotation(0.77f);
			sprites[i]->Update();
		}

		// Sprite用のWorldProjectionMatrixを作る
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth_), float(WinApp::kClientHeight_), 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		transformationMatrixDataSprite->World = worldMatrixSprite;

		//// ImGuiの内部コマンドを生成する
		ImGui::Render();

		dxCommon->PreDraw();

		spriteCommon->PreDraw();

#pragma region Sphere Draw
		commandList->IASetVertexBuffers(0, 1, &sphereVertexBufferView); // VBVを設定
		commandList->IASetIndexBuffer(&sphereIndexBufferView); // IBVを設定
		// マテリアルCBufferの場所を設定
		commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// wvp用のCBufferの場所を設定
		commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixSphere->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

		commandList->DrawIndexedInstanced(kSphereIndexNum, 1, 0, 0, 0);
#pragma endregion
#pragma region Sprite Draw

		for (uint32_t i = 0; i < 5; ++i) {
			sprites[i]->Draw();
		}

#pragma endregion

		// 実際のcommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

		dxCommon->PostDraw();
	}

	// こういうもんである。初期化と逆順に行う
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Log(logStream, "Hello,DirectX!\n");
	Log(logStream, ConvertString(std::format(L"clientSize:{},{}\n", WinApp::kClientWidth_,
		WinApp::kClientHeight_)));


	for (uint32_t i = 0; i < 5; ++i) {
		delete sprites[i];
	}
	delete spriteCommon;
	TextureManager::GetInstance()->Filalize();
	delete dxCommon;

	winApp->Finalize();
	delete winApp;
	return 0;
}