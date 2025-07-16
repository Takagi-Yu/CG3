#include <Windows.h>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <DbgHelp.h>
#include <strsafe.h>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <fstream>
#include <sstream>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <vector>

#define M_PI 3.141592f  

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")

struct Vector2 {
  float x;
  float y;
}; 

struct Vector3 {
  float x;
  float y;
  float z;
};

struct Vector4 {
  float x;
  float y;
  float z;
  float w;
};

struct Matrix3x3 {
  float m[3][3];
};

struct Matrix4x4 {
  float m[4][4];
};

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

struct ModelData {
  std::vector<VertexData> vertices;
};

Matrix4x4 MakeIdentity4x4() {
  Matrix4x4 result = {
      1, 0, 0, 0, 
      0, 1, 0, 0, 
      0, 0, 1, 0, 
      0, 0, 0, 1
  };
  return result;
}

Matrix4x4 Multiply(const Matrix4x4 &m1, const Matrix4x4 m2) {
  Matrix4x4 result{};

  result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] +
                   m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
  result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] +
                   m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
  result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] +
                   m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
  result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] +
                   m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];
  result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] +
                   m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
  result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] +
                   m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
  result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] +
                   m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
  result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] +
                   m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];
  result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] +
                   m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
  result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] +
                   m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
  result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] +
                   m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
  result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] +
                   m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];
  result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] +
                   m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
  result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] +
                   m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
  result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] +
                   m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
  result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] +
                   m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];

  return result;
}

Matrix4x4 MakeRotateXMatrix(float radian) {
  Matrix4x4 result{
      1, 0, 0, 0,
      0, cosf(radian), sinf(radian), 0, 
      0, -sinf(radian), cosf(radian), 0,
      0, 0, 0, 1
  };
  return result;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
  Matrix4x4 result{
      cosf(radian), 0, -sinf(radian), 0, 
      0, 1, 0, 0,
      sinf(radian), 0, cosf(radian),  0, 
      0, 0, 0, 1
  };
  return result;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
  Matrix4x4 result{
      cosf(radian), sinf(radian), 0, 0,
      -sinf(radian), cosf(radian), 0, 0,
      0, 0, 1, 0, 
      0, 0, 0, 1
  };
  return result;
}

Matrix4x4 MakeScaleMatrix(Vector3 scale) { 
    Matrix4x4 result{
        scale.x, 0.0f, 0.0f, 0.0f, 
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f, 
        0.0f, 0.0f, 0.0f, 1.0f
    };
  return result;
}

Matrix4x4 MakeTranslateMatrix(Vector3 translate) {
  Matrix4x4 result{
      1.0f, 0.0f, 0.0f, 0.0f, 
      0.0f, 1.0f, 0.0f, 0.0f,        
      0.0f, 0.0f, 1.0f, 0.0f,
      translate.x, translate.y, translate.z, 1.0f
  };
  return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3 &scale, const Vector3 &rotate,
                           const Vector3 &translate) {
  Matrix4x4 result;

  Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);

  Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);

  Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);

  Matrix4x4 rotateXYZMatrix =
      Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));

  for (int i = 0; i < 3; i++) {
    result.m[0][i] = scale.x * rotateXYZMatrix.m[0][i];
  }
  result.m[0][3] = 0;

  for (int i = 0; i < 3; i++) {
    result.m[1][i] = scale.y * rotateXYZMatrix.m[1][i];
  }
  result.m[1][3] = 0;

  for (int i = 0; i < 3; i++) {
    result.m[2][i] = scale.z * rotateXYZMatrix.m[2][i];
  }
  result.m[2][3] = 0;

  result.m[3][0] = translate.x;
  result.m[3][1] = translate.y;
  result.m[3][2] = translate.z;
  result.m[3][3] = 1;

  return result;
}

// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio,
                                   float nearClip, float farClip) {
  Matrix4x4 result;

  result.m[0][0] = (1.0f / aspectRatio) * (1.0f / tanf(fovY / 2.0f));
  result.m[0][1] = 0.0f;
  result.m[0][2] = 0.0f;
  result.m[0][3] = 0.0f;
  result.m[1][0] = 0.0f;
  result.m[1][1] = 1.0f / tanf(fovY / 2.0f);
  result.m[1][2] = 0.0f;
  result.m[1][3] = 0.0f;
  result.m[2][0] = 0.0f;
  result.m[2][1] = 0.0f;
  result.m[2][2] = farClip / (farClip - nearClip);
  result.m[2][3] = 1.0f;
  result.m[3][0] = 0.0f;
  result.m[3][1] = 0.0f;
  result.m[3][2] = -nearClip * farClip / (farClip - nearClip);
  result.m[3][3] = 0.0f;

  return result;
}

Matrix4x4 MakeOrthographicMatrix(float left, float top, float right,
                                 float bottom, float nearClip, float farClip) {
  Matrix4x4 result;

  result.m[0][0] = 2.0f / (right - left);
  result.m[0][1] = 0.0f;
  result.m[0][2] = 0.0f;
  result.m[0][3] = 0.0f;
  result.m[1][0] = 0.0f;
  result.m[1][1] = 2.0f / (top - bottom);
  result.m[1][2] = 0.0f;
  result.m[1][3] = 0.0f;
  result.m[2][0] = 0.0f;
  result.m[2][1] = 0.0f;
  result.m[2][2] = 1.0f / (farClip - nearClip);
  result.m[2][3] = 0.0f;
  result.m[3][0] = (left + right) / (left - right);
  result.m[3][1] = (top + bottom) / (bottom - top);
  result.m[3][2] = nearClip / (nearClip - farClip);
  result.m[3][3] = 1.0f;

  return result;
}

// 逆行列
Matrix4x4 Inverse(const Matrix4x4 &m) {
  Matrix4x4 result;
  float A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3] +
            m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1] +
            m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2] - // 3
            m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1] -
            m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3] -
            m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2] - // 6
            m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3] -
            m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1] -
            m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2] + // 9
            m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1] +
            m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3] +
            m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2] + // 12
            m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3] +
            m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1] +
            m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2] - // 15
            m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1] -
            m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3] -
            m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2] - // 18
            m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0] -
            m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0] -
            m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0] + // 21
            m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0] +
            m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0] +
            m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0]; // 24

  result.m[0][0] =
      1 / A *
      (m.m[1][1] * m.m[2][2] * m.m[3][3] + m.m[1][2] * m.m[2][3] * m.m[3][1] +
       m.m[1][3] * m.m[2][1] * m.m[3][2] - m.m[1][3] * m.m[2][2] * m.m[3][1] -
       m.m[1][2] * m.m[2][1] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2]);
  result.m[0][1] =
      1 / A *
      (-m.m[0][1] * m.m[2][2] * m.m[3][3] - m.m[0][2] * m.m[2][3] * m.m[3][1] -
       m.m[0][3] * m.m[2][1] * m.m[3][2] + m.m[0][3] * m.m[2][2] * m.m[3][1] +
       m.m[0][2] * m.m[2][1] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2]);
  result.m[0][2] =
      1 / A *
      (m.m[0][1] * m.m[1][2] * m.m[3][3] + m.m[0][2] * m.m[1][3] * m.m[3][1] +
       m.m[0][3] * m.m[1][1] * m.m[3][2] - m.m[0][3] * m.m[1][2] * m.m[3][1] -
       m.m[0][2] * m.m[1][1] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2]);
  result.m[0][3] =
      1 / A *
      (-m.m[0][1] * m.m[1][2] * m.m[2][3] - m.m[0][2] * m.m[1][3] * m.m[2][1] -
       m.m[0][3] * m.m[1][1] * m.m[2][2] + m.m[0][3] * m.m[1][2] * m.m[2][1] +
       m.m[0][2] * m.m[1][1] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2]);

  result.m[1][0] =
      1 / A *
      (-m.m[1][0] * m.m[2][2] * m.m[3][3] - m.m[1][2] * m.m[2][3] * m.m[3][0] -
       m.m[1][3] * m.m[2][0] * m.m[3][2] + m.m[1][3] * m.m[2][2] * m.m[3][0] +
       m.m[1][2] * m.m[2][0] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2]);
  result.m[1][1] =
      1 / A *
      (m.m[0][0] * m.m[2][2] * m.m[3][3] + m.m[0][2] * m.m[2][3] * m.m[3][0] +
       m.m[0][3] * m.m[2][0] * m.m[3][2] - m.m[0][3] * m.m[2][2] * m.m[3][0] -
       m.m[0][2] * m.m[2][0] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2]);
  result.m[1][2] =
      1 / A *
      (-m.m[0][0] * m.m[1][2] * m.m[3][3] - m.m[0][2] * m.m[1][3] * m.m[3][0] -
       m.m[0][3] * m.m[1][0] * m.m[3][2] + m.m[0][3] * m.m[1][2] * m.m[3][0] +
       m.m[0][2] * m.m[1][0] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2]);
  result.m[1][3] =
      1 / A *
      (m.m[0][0] * m.m[1][2] * m.m[2][3] + m.m[0][2] * m.m[1][3] * m.m[2][0] +
       m.m[0][3] * m.m[1][0] * m.m[2][2] - m.m[0][3] * m.m[1][2] * m.m[2][0] -
       m.m[0][2] * m.m[1][0] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2]);

  result.m[2][0] =
      1 / A *
      (m.m[1][0] * m.m[2][1] * m.m[3][3] + m.m[1][1] * m.m[2][3] * m.m[3][0] +
       m.m[1][3] * m.m[2][0] * m.m[3][1] - m.m[1][3] * m.m[2][1] * m.m[3][0] -
       m.m[1][1] * m.m[2][0] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1]);
  result.m[2][1] =
      1 / A *
      (-m.m[0][0] * m.m[2][1] * m.m[3][3] - m.m[0][1] * m.m[2][3] * m.m[3][0] -
       m.m[0][3] * m.m[2][0] * m.m[3][1] + m.m[0][3] * m.m[2][1] * m.m[3][0] +
       m.m[0][1] * m.m[2][0] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1]);
  result.m[2][2] =
      1 / A *
      (m.m[0][0] * m.m[1][1] * m.m[3][3] + m.m[0][1] * m.m[1][3] * m.m[3][0] +
       m.m[0][3] * m.m[1][0] * m.m[3][1] - m.m[0][3] * m.m[1][1] * m.m[3][0] -
       m.m[0][1] * m.m[1][0] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1]);
  result.m[2][3] =
      1 / A *
      (-m.m[0][0] * m.m[1][1] * m.m[2][3] - m.m[0][1] * m.m[1][3] * m.m[2][0] -
       m.m[0][3] * m.m[1][0] * m.m[2][1] + m.m[0][3] * m.m[1][1] * m.m[2][0] +
       m.m[0][1] * m.m[1][0] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1]);

  result.m[3][0] =
      1 / A *
      (-m.m[1][0] * m.m[2][1] * m.m[3][2] - m.m[1][1] * m.m[2][2] * m.m[3][0] -
       m.m[1][2] * m.m[2][0] * m.m[3][1] + m.m[1][2] * m.m[2][1] * m.m[3][0] +
       m.m[1][1] * m.m[2][0] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1]);
  result.m[3][1] =
      1 / A *
      (m.m[0][0] * m.m[2][1] * m.m[3][2] + m.m[0][1] * m.m[2][2] * m.m[3][0] +
       m.m[0][2] * m.m[2][0] * m.m[3][1] - m.m[0][2] * m.m[2][1] * m.m[3][0] -
       m.m[0][1] * m.m[2][0] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1]);
  result.m[3][2] =
      1 / A *
      (-m.m[0][0] * m.m[1][1] * m.m[3][2] - m.m[0][1] * m.m[1][2] * m.m[3][0] -
       m.m[0][2] * m.m[1][0] * m.m[3][1] + m.m[0][2] * m.m[1][1] * m.m[3][0] +
       m.m[0][1] * m.m[1][0] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1]);
  result.m[3][3] =
      1 / A *
      (m.m[0][0] * m.m[1][1] * m.m[2][2] + m.m[0][1] * m.m[1][2] * m.m[2][0] +
       m.m[0][2] * m.m[1][0] * m.m[2][1] - m.m[0][2] * m.m[1][1] * m.m[2][0] -
       m.m[0][1] * m.m[1][0] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1]);

  return result;
}

static LONG WINAPI ExportDump(EXCEPTION_POINTERS *exception) {
  // 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpディレクトリ以下に出力
  SYSTEMTIME time;
  GetLocalTime(&time);
  wchar_t filePath[MAX_PATH] = {0};
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
  MINIDUMP_EXCEPTION_INFORMATION maindumpInformation{0};
  maindumpInformation.ThreadId = threadId;
  maindumpInformation.ExceptionPointers = exception;
  maindumpInformation.ClientPointers = TRUE;
  // Dumpを出力。MiniDumpNomalは最低限の情報を出力するフラグ
  MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle,
                    MiniDumpNormal, &maindumpInformation, nullptr, nullptr);
  // ほかに関連付けられているSEH例外ハンドラがあれば実行。通常はプロセスする
  return EXCEPTION_EXECUTE_HANDLER;
}

std::wstring ConvertString(const std::string &str) {
  if (str.empty()) {
    return std::wstring();
  }

  auto sizeNeeded =
      MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&str[0]),
                          static_cast<int>(str.size()), NULL, 0);
  if (sizeNeeded == 0) {
    return std::wstring();
  }
  std::wstring result(sizeNeeded, 0);
  MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&str[0]),
                      static_cast<int>(str.size()), &result[0], sizeNeeded);
  return result;
}

std::string ConvertString(const std::wstring &str) {
  if (str.empty()) {
    return std::string();
  }

  auto sizeNeeded =
      WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
                          NULL, 0, NULL, NULL);
  if (sizeNeeded == 0) {
    return std::string();
  }
  std::string result(sizeNeeded, 0);
  WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
                      result.data(), sizeNeeded, NULL, NULL);
  return result;
}

//void Log(const std::string &message) {
//    OutputDebugStringA(message.c_str());
//}

void Log(std::ostream &os, const std::string &message) {
  os << message << std::endl;
  OutputDebugStringA(message.c_str());
}

//ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
    return true;
  }
  //メッセージに応じてゲーム固有の処理を行う
  switch (msg) {
        //ウィンドウが破棄された
  case WM_DESTROY:
      //OSに対して、アプリの終了を伝える
    PostQuitMessage(0);
    return 0;
  }
    //標準のメッセージ処理を行う
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device,
    D3D12_DESCRIPTOR_HEAP_TYPE heapType,
    UINT numDescriptors,
    bool shaderVisible) {
  ID3D12DescriptorHeap *descriptorHeap = nullptr;
  D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
  descriptorHeapDesc.Type = heapType;
  descriptorHeapDesc.NumDescriptors = numDescriptors;
  descriptorHeapDesc.Flags = shaderVisible
                                 ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                 : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc,
                                            IID_PPV_ARGS(&descriptorHeap));
  assert(SUCCEEDED(hr));
  return descriptorHeap;
}

IDxcBlob *CompileShader(
    const std::wstring &filePath, // CompilerするShaderファイルへのパス
    const wchar_t *profile,       // Compilerに使用するProfile
    IDxcUtils *dxcUtils,          // 初期化で生成したものを3つ
    IDxcCompiler3 *dxcCompiler, IDxcIncludeHandler *includeHandler,
    std::ostream &os) {
  // これからシェーダーをコンパイルする旨をログに出す
  Log(os,
      ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n",
                                filePath, profile)));
  // hlslファイルを読む
  IDxcBlobEncoding *shaderSource = nullptr;
  HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
  // 読めなかったら止める
  assert(SUCCEEDED(hr));
  // 読み込んだファイルの内容を設定する
  DxcBuffer shaderSourceBuffer;
  shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
  shaderSourceBuffer.Size = shaderSource->GetBufferSize();
  shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF8の文字コードであることを通知

  LPCWSTR arguments[] = {
      filePath.c_str(), // コンパイル対象のhlslファイル名
      L"-E",
      L"main", // エントリーポイントの指定。基本的にmain以外にはしない
      L"-T",
      profile, // ShaderProfileの設定
      L"-Zi",
      L"-Qembed_debug", // デバッグ用の情報を埋め込む
      L"-Od",           // 最適化を外しておく
      L"-Zpr",          // メモリレイアウトは行優先
  };
  // 実際にShaderをコンパイルする
  IDxcResult *shaderResult = nullptr;
  hr = dxcCompiler->Compile(&shaderSourceBuffer, arguments, _countof(arguments),
                            includeHandler, IID_PPV_ARGS(&shaderResult));
  // コンパイラエラーではなくdxcが起動できないなど致命的な状況
  assert(SUCCEEDED(hr));

  // 警告・エラーが出てたらログに出して止める
  IDxcBlobUtf8 *shaderError = nullptr;
  shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
  if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
    Log(os, shaderError->GetStringPointer());
    // 警告・エラーダメゼッタイ
    assert(false);
  }
  // コンパイル結果から実行用のバイナリ部分を取得
  IDxcBlob *shaderBlob = nullptr;
  hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob),
                               nullptr);
  assert(SUCCEEDED(hr));
  // 成功したログを出す
  Log(os, ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n",
                                    filePath, profile)));
  // もう使わないリソースを解放
  shaderSource->Release();
  shaderResult->Release();
  // 実行用のバイナリを返却
  return shaderBlob;
}

ID3D12Resource *CreateBufferResource(ID3D12Device *device, size_t sizeInBytes) {
  // 頂点リソース用のヒープの設定
  D3D12_HEAP_PROPERTIES uploadHeapProperties{};
  uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
  // 頂点リソースの設定
  D3D12_RESOURCE_DESC vertexResourceDesc{};
  // バッファリソース。テクスチャの場合はまた別の設定をする
  vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  vertexResourceDesc.Width =
      sizeInBytes; // リソースのサイズ。今回はVector4を3頂点分
  // バッファの場合はこれらは1にする決まり
  vertexResourceDesc.Height = 1;
  vertexResourceDesc.DepthOrArraySize = 1;
  vertexResourceDesc.MipLevels = 1;
  vertexResourceDesc.SampleDesc.Count = 1;
  // バッファの場合はこれにする決まり
  vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  // 実際に頂点リソースを作る
  // ID3D12Resource *vertexResource =
  //    CreateBufferResource(device, sizeof(Vector4) * 3);
  ID3D12Resource *vertexResource = nullptr;
  HRESULT hr = device->CreateCommittedResource(
      &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
      IID_PPV_ARGS(&vertexResource));
  assert(SUCCEEDED(hr));

  return vertexResource;
}

DirectX::ScratchImage LoadTexture(const std::string& filepath) {
  // テクスチャファイルを読んでプログラムで扱えるようにする
  DirectX::ScratchImage image{};
  std::wstring filepathW=ConvertString(filepath);
  HRESULT hr = DirectX::LoadFromWICFile(
      filepathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
  assert(SUCCEEDED(hr));

  // ミニマップの作成
  DirectX::ScratchImage mipImage{};
  hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(),
                                image.GetMetadata(), DirectX::TEX_FILTER_SRGB,
                                0, mipImage);
  assert(SUCCEEDED(hr));

  // ミニマップ付きのデータを返す
  return mipImage;
}

ID3D12Resource* CreateTextureResource(ID3D12Device* device,
    const DirectX::TexMetadata& metadata) {
  // matadataを基にResourceの設定
  D3D12_RESOURCE_DESC resourceDesc{};
  resourceDesc.Width = UINT(metadata.width); // Textureの幅
  resourceDesc.Height = UINT(metadata.height); // Textureの高さ
  resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
  resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行き or 配列Textureの配列数
  resourceDesc.Format = metadata.format; //TextureのFormat
  resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。普段使っているのは2次元

  // 利用するHeapの設定。非常に特殊な運用。02_04exで一般的なケース版がある
  D3D12_HEAP_PROPERTIES heapProperties{};
  heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う


  // Resourceの生成
  ID3D12Resource *resource = nullptr;
  HRESULT hr = device->CreateCommittedResource(
      &heapProperties, // Heapの設定
      D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定
      &resourceDesc, // Resourceの設定
      D3D12_RESOURCE_STATE_COPY_DEST, // しょかいのResourceState。Textureは基本読むだけ
      nullptr, // Clear最適値。使わないのでnullptr
      IID_PPV_ARGS(&resource) // 作成するResourceポインタへのポインタ
  );
  assert(SUCCEEDED(hr));
  return resource;
}

ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages,ID3D12Device* device,ID3D12GraphicsCommandList*commandList) {
  std::vector<D3D12_SUBRESOURCE_DATA> subresources;
  DirectX::PrepareUpload(device, mipImages.GetImages(),
                         mipImages.GetImageCount(), mipImages.GetMetadata(),
                         subresources);
  uint64_t intermediateSize =
      GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
  ID3D12Resource *intermediateResource =
      CreateBufferResource(device, intermediateSize);
  UpdateSubresources(commandList, texture, intermediateResource, 0, 0,
                     UINT(subresources.size()), subresources.data());

  D3D12_RESOURCE_BARRIER barrier{};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = texture;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
  commandList->ResourceBarrier(1, &barrier);
  return intermediateResource;
}

ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device,
    int32_t width,
    int32_t height) {
    // 生成するResourceの設定
  D3D12_RESOURCE_DESC resourceDesc{};
  resourceDesc.Width = width; // Textureの幅
  resourceDesc.Height = height; // Textureの高さ
  resourceDesc.MipLevels = 1; // mipmapの数
  resourceDesc.DepthOrArraySize = 1; // 奥行き or 配列Textureの配列数
  resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // DepthStencilとして利用可能なフォーマット
  resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定。
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; // 2次元
  resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; // DepthStencilとして使う通知

  // 利用するHeapの設定
  D3D12_HEAP_PROPERTIES heapProperties{};
  heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る

  // 深度値のクリア
  D3D12_CLEAR_VALUE depthClearValue{};
  depthClearValue.DepthStencil.Depth = 1.0f;
  depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

  // Rsourceの生成
  ID3D12Resource *resource = nullptr;
  HRESULT hr = device->CreateCommittedResource(
      &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
      D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue,
      IID_PPV_ARGS(&resource));
  assert(SUCCEEDED(hr));
  return resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE
GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap,
    uint32_t descriptorSize, uint32_t index) {
  D3D12_CPU_DESCRIPTOR_HANDLE handleCPU =
      descriptorHeap->GetCPUDescriptorHandleForHeapStart();
  handleCPU.ptr += (descriptorSize * index);
  return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE
GetGPUDescriptorHandle(ID3D12DescriptorHeap *descriptorHeap,
                       uint32_t descriptorSize, uint32_t index) {
  D3D12_GPU_DESCRIPTOR_HANDLE handleGPU =
      descriptorHeap->GetGPUDescriptorHandleForHeapStart();
  handleGPU.ptr += (descriptorSize * index);
  return handleGPU;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {
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
      position.w = 1.0f;
      positions.push_back(position);
    } else if (identifier == "vt") {
      Vector2 texcoord;
      s >> texcoord.x >> texcoord.y;
      texcoords.push_back(texcoord);
    } else if (identifier == "vn") {
      Vector3 normal;
      s >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    } else if (identifier == "f") {
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
        VertexData vertex = {position, texcoord, normal};
        modelData.vertices.push_back(vertex);
      }
    }
  }

}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  CoInitializeEx(0, COINIT_MULTITHREADED);
  // 誰も捕捉しなかった場合に(Unhedled),補足する関数を登録
  // main関数始まってすぐに登録すると良い
  SetUnhandledExceptionFilter(ExportDump);

  /*uint32_t *p = nullptr;
   *p = 100;*/

  std::filesystem::create_directory("logs");

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

  std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
      nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);

  std::chrono::zoned_time localtime{std::chrono::current_zone(), nowSeconds};

  std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localtime);

  std::string logFilePath = std::string("logs/") + dateString + ".log";

  std::ofstream logStream(logFilePath);

  WNDCLASS wc{};
  // ウィンドウプロシージャ
  wc.lpfnWndProc = WindowProc;
  // うウィンドウクラス名
  wc.lpszClassName = L"CG2WindowClass";
  // インスタンスハンドル
  wc.hInstance = GetModuleHandle(nullptr);
  // カーソル
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

  // ウィンドウクラスを登録する
  RegisterClass(&wc);

  // クライアント領域のサイズ
  const int32_t kClientWidth = 1280;
  const int32_t kClientheight = 720;

  // ウィンドウサイズを表す構造体にクライアント領域を入れる
  RECT wrc = {0, 0, kClientWidth, kClientheight};

  // クライアント領域を元に実際のサイズにwrcに変更してもらう
  AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

  // ウィンドウの生成
  HWND hwnd = CreateWindow(wc.lpszClassName, // 利用するクラス
                           L"CG2",           // タイトルバーの文字
                           WS_OVERLAPPEDWINDOW, // よく見るウィンドウスタイル
                           CW_USEDEFAULT, // 表示X座標(Windowsに任せる)
                           CW_USEDEFAULT, // 表示Y座標(WindowOSに任せる)
                           wrc.right - wrc.left, // ウィンドウ横幅
                           wrc.bottom - wrc.top, // ウィンドウ縦幅
                           nullptr,      // 親ウィンドウハンドル
                           nullptr,      // メニューハンドル
                           wc.hInstance, // インスタンスハンドル
                           nullptr);     // オプション

  // ウィンドウを表示する
  ShowWindow(hwnd, SW_SHOW);

  // DXGIファクトリーの生成
  IDXGIFactory7 *dxgiFactory = nullptr;
  // HRESULTはWindows系エラーコードであり、
  // 関数が成功したかどうかをSUCCEEDEDマクロで判定できる
  HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
  // 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか
  // どうにもできない場合が多いのでassertにしておく
  assert(SUCCEEDED(hr));

  // 使用するアダプタ用の変数。最初にnullptrを入れておく
  IDXGIAdapter4 *useAdapter = nullptr;

  // 良い順にアダプタを頼む
  for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(
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
      Log(logStream, ConvertString(std::format(L"Use Adapter:{}\n",
                                               adapterDesc.Description)));
      break;
    }
    useAdapter = nullptr; // ソフトウェアアダプタの場合は見なかったことにする
  }
  // 適切なアダプタが見つからなっかったので起動できない
  assert(useAdapter != nullptr);

#ifdef _DEBUG
  ID3D12Debug1 *debugController = nullptr;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
    // デバッグレイヤーを有効化する
    debugController->EnableDebugLayer();
    // さらにGPU側でもチェックを行うようにする
    debugController->SetEnableGPUBasedValidation(TRUE);
  }
#endif

  ID3D12Device *device = nullptr;
  // 機能レベルとログ出力用の文字列
  D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};

  const char *featureLevelStrings[] = {"12.2", "12.1", "12.0"};
  // 高い順に生成できるか試していく
  for (size_t i = 0; i < _countof(featureLevels); ++i) {
    // 採用したアダプターでデバイスを生成
    hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
    // 指定した機能レベルでデバイスが生成できたかを確認
    if (SUCCEEDED(hr)) {
      // 生成できたのでログ出力を行ってループを抜ける
      Log(logStream,
          std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
      break;
    }
  }
  // デバイスの生成がうまくいかなかったので起動できない
  assert(device != nullptr);
  Log(logStream, "Complete create D3D12Device!!!\n");

  #ifdef _DEBUG
  ID3D12InfoQueue *infoQueue = nullptr;
  if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
    // やばいエラー時に止まる
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
    // エラー時に止まる
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    // 警告時に止まる
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    // 抑制するメッセージのID
    D3D12_MESSAGE_ID denyIds[] = {
        // Windows11でのDXGIデバッグレイヤーの相互作用バグによるエラーメッセージ
        D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE};
    D3D12_MESSAGE_SEVERITY severities[] = {D3D12_MESSAGE_SEVERITY_INFO};
    D3D12_INFO_QUEUE_FILTER filter{};
    filter.DenyList.NumIDs = _countof(denyIds);
    filter.DenyList.pIDList = denyIds;
    filter.DenyList.NumSeverities = _countof(severities);
    filter.DenyList.pSeverityList = severities;
    infoQueue->PushStorageFilter(&filter);
    // 解放
    infoQueue->Release();
  }
#endif

  // コマンドキューを生成する
  ID3D12CommandQueue *commandQueue = nullptr;
  D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
  hr = device->CreateCommandQueue(&commandQueueDesc,
                                  IID_PPV_ARGS(&commandQueue));
  // コマンドキューの生成がうまくいかなかったので起動できない
  assert(SUCCEEDED(hr));

  // コマンドアロケータを生成する
  ID3D12CommandAllocator *commandAllocator = nullptr;
  hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                      IID_PPV_ARGS(&commandAllocator));
  // コマンドアロケータの生成がうまくいかなかったので起動できない
  assert(SUCCEEDED(hr));

  // コマンドリストを生成する
  ID3D12GraphicsCommandList *commandList = nullptr;
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 commandAllocator, nullptr,
                                 IID_PPV_ARGS(&commandList));
  // コマンドリストの生成がうまくいかなかったので起動できない
  assert(SUCCEEDED(hr));

  // スワップチェーンを生成する
  IDXGISwapChain4 *swapChain = nullptr;
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
  swapChainDesc.Width =
      kClientWidth; // 画面の幅。ウィンドウのクライアント領域を同じものにしておく
  swapChainDesc.Height =
      kClientheight; // 画面の高さ。ウィンドウのクライアント領域を同じものにしておく
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
  swapChainDesc.SampleDesc.Count = 1;                // マルチサンプルしない
  swapChainDesc.BufferUsage =
      DXGI_USAGE_RENDER_TARGET_OUTPUT; // 描画のターゲットとして利用する
  swapChainDesc.BufferCount = 2;       // ダブルバッファ
  swapChainDesc.SwapEffect =
      DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタにうつしたら、中身を破棄
  // コマンドキュー、ウィンドウハンドル、設定を渡して生成する
  hr = dxgiFactory->CreateSwapChainForHwnd(
      commandQueue, hwnd, &swapChainDesc, nullptr, nullptr,
      reinterpret_cast<IDXGISwapChain1 **>(&swapChain));
  assert(SUCCEEDED(hr));

  // ディスクリプターヒープの生成
  // ID3D12DescriptorHeap *rtvDescriptorHeap = nullptr;
  // RTV用のヒープでディスクリプタの数は2。RTVはShader内で触れるものではないので、ShaderVisibleはfalse
  ID3D12DescriptorHeap *rtvDescriptorHeap =
      CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
  // SRV用のヒープでディスクリプタの数は128。SRVはShader内で触れるものなので、ShaderVisibleはtrue
  ID3D12DescriptorHeap *srvDescriptorHeap = CreateDescriptorHeap(
      device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
  D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
  rtvDescriptorHeapDesc.Type =
      D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // レンダーターゲットビュー用
  rtvDescriptorHeapDesc.NumDescriptors =
      2; // ダブルバッファ用に2つ。多くても別にかまわない

  // ディスクリプターヒープが作れなかったので起動できない
  assert(SUCCEEDED(hr));

  // SwapChainからResourceを引っ張ってくる
  ID3D12Resource *swapChainResources[2] = {nullptr};
  hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
  // うまく取得できなければ起動できない
  assert(SUCCEEDED(hr));
  hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
  assert(SUCCEEDED(hr));

  // RTVの設定
  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
  rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
  // ディスクリプタの戦闘を取得する
  D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle =
      rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
  // RTVを2つ作るのでディスクリプタを2つ用意
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
  // まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
  rtvHandles[0] = rtvStartHandle;
  device->CreateRenderTargetView(swapChainResources[0], &rtvDesc,
                                 rtvHandles[0]);
  // 2つ目のディスクリプタハンドルを得る(自力で)
  rtvHandles[1].ptr =
      rtvHandles[0].ptr +
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  // 2つ目を作る
  device->CreateRenderTargetView(swapChainResources[1], &rtvDesc,
                                 rtvHandles[1]);

  // 初期値0でFenceを作る
  ID3D12Fence *fence = nullptr;
  uint64_t fenceValue = 0;
  hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE,
                           IID_PPV_ARGS(&fence));
  assert(SUCCEEDED(hr));

  // FenceのSignalを待つためのイベントを作成する
  HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  assert(fenceEvent != nullptr);

  // dxcCompilerを初期化
  IDxcUtils *dxcUtils = nullptr;
  IDxcCompiler3 *dxcCompiler = nullptr;
  hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
  assert(SUCCEEDED(hr));
  hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
  assert(SUCCEEDED(hr));

  // 現時点でincludeはしないが、includeに対応するための設定を行っておく
  IDxcIncludeHandler *includeHandler = nullptr;
  hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
  assert(SUCCEEDED(hr));

  // RootSignature作成
  D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
  descriptionRootSignature.Flags =
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
  descriptorRange[0].BaseShaderRegister = 0;                      // 0から始まる
  descriptorRange[0].NumDescriptors = 1;                          // 数は1つ
  descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
  descriptorRange[0].OffsetInDescriptorsFromTableStart =
      D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // offsetを自動計算

  
  D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
  staticSamplers[0].Filter =
      D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
  staticSamplers[0].AddressU =
      D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
  staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
  staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
  staticSamplers[0].ShaderRegister = 0;         // レジスタ番号0を使う
  staticSamplers[0].ShaderVisibility =
      D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
  descriptionRootSignature.pStaticSamplers = staticSamplers;
  descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);



  // RootParamater作成。複数設定できるので配列。今回は結果1つだけなので長さ1の配列
  D3D12_ROOT_PARAMETER rootParameters[4] = {};
  rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
  rootParameters[0].ShaderVisibility =
      D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
  rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0を使う

  rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
  rootParameters[1].ShaderVisibility =
      D3D12_SHADER_VISIBILITY_VERTEX; // PixelShaderで使う
  rootParameters[1].Descriptor.ShaderRegister = 0; // レジスタ番号0を使う

  rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
  rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
  rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
  rootParameters[2].DescriptorTable.NumDescriptorRanges =
      _countof(descriptorRange); // Tableで利用する数
  rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
  rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
  rootParameters[3].Descriptor.ShaderRegister = 1; // レジスタ番号1を使う

  descriptionRootSignature.pParameters =
      rootParameters; // ルートパラメータ配列へのポインタ
  descriptionRootSignature.NumParameters =
      _countof(rootParameters); // 配列の長さ

  // マテリアル用のリソースを作る
  ID3D12Resource *materialResource =
      CreateBufferResource(device, sizeof(Material));
  // マテリアルにデータを書き込む
  Material *materialData = nullptr;
  // 書き込むためのアドレスを取得
  materialResource->Map(0, nullptr, reinterpret_cast<void **>(&materialData));
  // 今回は赤を書き込んでみる
  materialData->color = {1.0f,1.0f,1.0f,1.0f};
  materialData->enableLighting = true;
  materialData->uvTransform = MakeIdentity4x4();


  // sprite用のマテリアルリソースを作る
  ID3D12Resource *materialResourceSprite =
      CreateBufferResource(device, sizeof(Material));
  Material *materialDataSprite = nullptr;
  materialResourceSprite->Map(0, nullptr,
                              reinterpret_cast<void **>(&materialDataSprite));
  materialDataSprite->color = {1.0f, 1.0f, 1.0f, 1.0f};
  // SpriteはLightingしないのでfalseを設定する
  materialDataSprite->enableLighting = false;
  materialDataSprite->uvTransform = MakeIdentity4x4();

  Transform uvTransformSprite{
      {1.0f, 1.0f, 1.0f},
      {0.0f, 0.0f, 0.0f},
      {0.0f, 0.0f, 0.0f},
  };

  ID3D12Resource *directionalLightResource =
      CreateBufferResource(device, sizeof(DirectionalLight));
  DirectionalLight *directionalLightData = nullptr;
  directionalLightResource->Map(0, nullptr,
                        reinterpret_cast<void **>(&directionalLightData));
  directionalLightData->color = {1.0f, 1.0f, 1.0f, 1.0f};
  directionalLightData->direction = {1.0f, 0.0f, 0.0f};
  directionalLightData->intensity = 1.0f;

  // シリアライズしてバイナリにする
  ID3DBlob *signatureBlob = nullptr;
  ID3DBlob *errorBlob = nullptr;
  hr = D3D12SerializeRootSignature(&descriptionRootSignature,
                                   D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob,
                                   &errorBlob);
  if (FAILED(hr)) {
    Log(logStream, reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
    assert(false);
  }
  // バイナリを元に生成
  ID3D12RootSignature *rootSignature = nullptr;
  hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
                                   signatureBlob->GetBufferSize(),
                                   IID_PPV_ARGS(&rootSignature));
  assert(SUCCEEDED(hr));

  // InputLayout
  //D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
  //inputElementDescs[0].SemanticName = "POSITION";
  //inputElementDescs[0].SemanticIndex = 0;
  //inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  //inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
  //D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
  //inputLayoutDesc.pInputElementDescs = inputElementDescs;
  //inputLayoutDesc.NumElements = _countof(inputElementDescs);

  D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
  inputElementDescs[0].SemanticName = "POSITION";
  inputElementDescs[0].SemanticIndex = 0;
  inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
  inputElementDescs[1].SemanticName = "TEXCOORD";
  inputElementDescs[1].SemanticIndex = 0;
  inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
  inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
  inputElementDescs[2].SemanticName = "NORMAL";
  inputElementDescs[2].SemanticIndex = 0;
  inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
  D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
  inputLayoutDesc.pInputElementDescs = inputElementDescs;
  inputLayoutDesc.NumElements = _countof(inputElementDescs);

  // BlendStateの設定
  D3D12_BLEND_DESC blendDesc{};
  // すべての色要素を書き込む
  blendDesc.RenderTarget[0].RenderTargetWriteMask =
      D3D12_COLOR_WRITE_ENABLE_ALL;

  // RasiterzerStateの設定
  D3D12_RASTERIZER_DESC rasterizerDesc{};
  // 裏面(時計回り)を表示しない
  //rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
  rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
  // 三角形の中を塗りつぶす
  rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

  // Shaderをコンパイルする
  IDxcBlob *vertexShaderBlob =
      CompileShader(L"Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler,
                    includeHandler, logStream);
  assert(vertexShaderBlob != nullptr);

  IDxcBlob *pixelShaderBlob =
      CompileShader(L"Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler,
                    includeHandler, logStream);
  assert(pixelShaderBlob != nullptr);

  D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
  graphicsPipelineStateDesc.pRootSignature = rootSignature; // RootSignature
  graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;  // InputLayout
  graphicsPipelineStateDesc.VS = {
      vertexShaderBlob->GetBufferPointer(),
      vertexShaderBlob->GetBufferSize()}; // VertexShader
  graphicsPipelineStateDesc.PS = {
      pixelShaderBlob->GetBufferPointer(),
      pixelShaderBlob->GetBufferSize()};                      // PixelShader
  graphicsPipelineStateDesc.BlendState = blendDesc;           // BlendState
  graphicsPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
  // 書き込むRTVの情報
  graphicsPipelineStateDesc.NumRenderTargets = 1;
  graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  // 利用するトポロジ(形状)のタイプ。三角形
  graphicsPipelineStateDesc.PrimitiveTopologyType =
      D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  // どのように画面に色を打ち込むのかの設定(気にしなくていい)
  graphicsPipelineStateDesc.SampleDesc.Count = 1;
  graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

  // DepthDtencilStateの設定
  D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
  // depthの機能を有効化する
  depthStencilDesc.DepthEnable = true;
  // 書き込みします
  depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  // 比較関数はLessEqual。つまり、近ければ描画される
  depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

  // DepthStencilの設定
  graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
  graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

  // 実際に生成
  ID3D12PipelineState *graphicsPipelineState = nullptr;
  hr = device->CreateGraphicsPipelineState(
      &graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
  assert(SUCCEEDED(hr));

  const uint32_t kSubdivision = 16; // 分割数
  uint32_t kSphereVertexNum = kSubdivision * kSubdivision * 6;
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
  // 実際に頂点リソースを作る
  // ID3D12Resource* vertexResource = nullptr;

  // モデル読み込み
  ModelData modelData = LoadObjFile("Resource", "plane.obj");
  // 頂点リソースを作る
  ID3D12Resource *vertexResource = CreateBufferResource(
      device, sizeof(VertexData) * modelData.vertices.size());
  // 頂点バッファビューを作成する
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
  vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソースの先頭のアドレスから使う
  vertexBufferView.SizeInBytes =
      UINT(sizeof(VertexData) * modelData.vertices.size()); // 使用するリソースのサイズは頂点のサイズ
  vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点当たりのサイズ

  // 頂点リソースにデータを書き込む
  VertexData *vertexData = nullptr;
  vertexResource->Map(0, nullptr, reinterpret_cast<void **>(&vertexData)); // 書き込むためのアドレスを取得
  std::memcpy(vertexData, modelData.vertices.data(),
              sizeof(VertexData) * modelData.vertices.size()); // 頂点リソースにコピー
  // =======
  // 球
  // =======
  
  //ID3D12Resource *vertexResource =
  //    CreateBufferResource(device, sizeof(VertexData) * kSphereVertexNum);
  //
  //// 頂点バッファビューを作成する
  //D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
  //// リソースの先頭のアドレスから使う
  //vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
  //// 使用するリソースのサイズは頂点3つ分のサイズ
  //vertexBufferView.SizeInBytes = sizeof(VertexData) * kSphereVertexNum;
  //// 1頂点当たりのサイズ
  //vertexBufferView.StrideInBytes = sizeof(VertexData);
  //
  //// 頂点リソースにデータを書き込む
  //VertexData *vertexData = nullptr;
  //// 書き込むためのアドレスを取得
  //vertexResource->Map(0, nullptr, reinterpret_cast<void **>(&vertexData));

  // 左下
  vertexData[0].position = {-0.5f, -0.5f, 0.0f, 1.0f};
  vertexData[0].texcoord = {0.0f, 1.0f};
  // 上
  vertexData[1].position = {0.0f, 0.5f, 0.0f, 1.0f};
  vertexData[1].texcoord = {0.5f, 0.0f};
  // 右下
  vertexData[2].position = {0.5f, -0.5f, 0.0f, 1.0f};
  vertexData[2].texcoord = {1.0f, 1.0f};

   // 左下2
  vertexData[3].position = {-0.5f, -0.5f, 0.5f, 1.0f};
  vertexData[3].texcoord = {0.0f, 1.0f};
  // 上2
  vertexData[4].position = {0.0f, 0.0f, 0.0f, 1.0f};
  vertexData[4].texcoord = {0.5f, 0.0f};
  // 右下2
  vertexData[5].position = {0.5f, -0.5f, -0.5f, 1.0f};
  vertexData[5].texcoord = {1.0f, 1.0f};


  // 経度分割1つ分の角度
  const float kLonEvery = 2.0f * M_PI / float(kSubdivision);
  // 緯度分割1つ分の角度
  const float kLatEvery = M_PI / float(kSubdivision);

  for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
    // 緯度
    float lat = -M_PI / 2.0f + kLatEvery * latIndex;
    float nextLat = lat + kLatEvery;

    float v0 = 1.0f - float(latIndex) / float(kSubdivision);
    float v1 = 1.0f - float(latIndex + 1) / float(kSubdivision);
    // 経度の方向に分割しながら線を引く
    for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
      uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
      // 経度
      float lon = lonIndex * kLonEvery; // 現在
      float nextLon = lon + kLonEvery;  // 次

      float u0 = float(lonIndex) / float(kSubdivision);
      float u1 = float(lonIndex + 1) / float(kSubdivision);

      VertexData vertA{};
      vertA.position = {
          cos(lat) * cos(lon), 
          sin(lat), 
          cos(lat) * sin(lon),
          1.0f
      };
      vertA.texcoord = {u0, v0};
      vertA.normal = {
          vertA.position.x,
          vertA.position.y,
          vertA.position.z,
      };

      VertexData vertB{};
      vertB.position = {
          cos(nextLat) * cos(lon), 
          sin(nextLat),
          cos(nextLat) * sin(lon),
          1.0f
      };
      vertB.texcoord = {u0, v1};
      vertB.normal = {
          vertB.position.x,
          vertB.position.y,
          vertB.position.z,
      };

      VertexData vertC{};
      vertC.position = {
          cos(lat) * cos(nextLon), 
          sin(lat),
          cos(lat) * sin(nextLon),
          1.0f
      };
      vertC.texcoord = {u1, v0};
      vertC.normal = {
          vertC.position.x,
          vertC.position.y,
          vertC.position.z,
      };

      VertexData vertD{};
      vertD.position = {
          cos(nextLat) * cos(nextLon), 
          sin(nextLat),
          cos(nextLat) * sin(nextLon), 
          1.0f
      };
      vertD.texcoord = {u1, v1};
      vertD.normal = {
          vertD.position.x,
          vertD.position.y,
          vertD.position.z,
      };

      // 頂点にデータを入力する。基準点a
      vertexData[start + 0] = vertA;
      vertexData[start + 1] = vertB;
      vertexData[start + 2] = vertC;

      vertexData[start + 3] = vertD;
      vertexData[start + 4] = vertC;
      vertexData[start + 5] = vertB;
      
    }
  }


  // Sprite用の頂点リソースを作る
  ID3D12Resource *vertexResourceSprite =
      CreateBufferResource(device, sizeof(VertexData) * 6);
  
  
  // 頂点バッファビューを作成する
  D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
  // リソースの先頭のアドレスから使う
  vertexBufferViewSprite.BufferLocation =
      vertexResourceSprite->GetGPUVirtualAddress();
  // 使用するリソースのサイズは頂点6つ分のサイズ
  vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
  // 1頂点あたりのサイズ
  vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);


  VertexData *vertexDataSprite = nullptr;
  vertexResourceSprite->Map(0, nullptr,
                            reinterpret_cast<void **>(&vertexDataSprite));
  // 1枚目の三角形
  vertexDataSprite[0].position = {0.0f, 360.0f, 0.0f, 1.0f}; // 左下
  vertexDataSprite[0].texcoord = {0.0f, 1.0f};
  vertexDataSprite[1].position = {0.0f, 0.0f, 0.0f, 1.0f}; // 左上
  vertexDataSprite[1].texcoord = {0.0f, 0.0f};
  vertexDataSprite[2].position = {640.0f, 360.0f, 0.0f, 1.0f}; // 右下
  vertexDataSprite[2].texcoord = {1.0f, 1.0f};
  // 2枚目の三角形
  vertexDataSprite[3].position = {0.0f, 0.0f, 0.0f, 1.0f}; // 左下
  vertexDataSprite[3].texcoord = {0.0f, 0.0f};
  vertexDataSprite[4].position = {640.0f, 0.0f, 0.0f, 1.0f}; // 左上
  vertexDataSprite[4].texcoord = {1.0f, 0.0f};
  vertexDataSprite[5].position = {640.0f, 360.0f, 0.0f, 1.0f}; // 右下
  vertexDataSprite[5].texcoord = {1.0f, 1.0f};

  vertexDataSprite[0].normal = {0.0f, 0.0f, -1.0f};

  // ビューポート
  D3D12_VIEWPORT viewport{};
  // クライアント領域のサイズと一緒にして画面全体に表示
  viewport.Width = kClientWidth;
  viewport.Height = kClientheight;
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;

  // シザー矩形
  D3D12_RECT scissorRect{};
  // 基本的にビューポートと同じ矩形が構成されるようにする
  scissorRect.left = 0;
  scissorRect.right = kClientWidth;
  scissorRect.top = 0;
  scissorRect.bottom = kClientheight;


  // WVP用のリソースを作る。MAtrix4x4 1つ分のサイズを用意する
  ID3D12Resource *wvpResource = CreateBufferResource(device, sizeof(TransformationMatrix));
  // データを書き込む
  TransformationMatrix *transformationMatrixData = nullptr;
  // 書き込むためのアドレスを取得
  wvpResource->Map(0, nullptr,
                   reinterpret_cast<void **>(&transformationMatrixData));
  // 単位行列を書き込んでおく
  transformationMatrixData->WVP = MakeIdentity4x4();
  transformationMatrixData->World = MakeIdentity4x4();


  // Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
  ID3D12Resource *transformationMatrixResourceSprite =
      CreateBufferResource(device, sizeof(TransformationMatrix));
  // データを書き込む
  TransformationMatrix *transformationMatrixDataSprite = nullptr;
  // 書き込むためのアドレスを取得
  transformationMatrixResourceSprite->Map(
      0, nullptr, reinterpret_cast<void **>(&transformationMatrixDataSprite));
  // 単位行列を書き込んでおく
  transformationMatrixDataSprite->WVP = MakeIdentity4x4();
  transformationMatrixDataSprite->World = MakeIdentity4x4();


  // Transform変数を作る
  Transform transform{
      {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
  Transform cameraTransform{
      {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -5.0f}};
  Transform transformSprite{
      {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};


  // こういうもんである
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX12_Init(
      device, 
      swapChainDesc.BufferCount, 
      rtvDesc.Format,
      srvDescriptorHeap,
      srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
      srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());


  // Textureを読んで転送する
  DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
  const DirectX::TexMetadata &metadata = mipImages.GetMetadata();
  ID3D12Resource *textureResource = CreateTextureResource(device, metadata);
  ID3D12Resource *intermediateResource =
      UploadTextureData(textureResource, mipImages, device, commandList);


  // 2枚目のTextureを読んで転送する
  DirectX::ScratchImage mipImages2 = LoadTexture("resources/monsterBall.png");
  const DirectX::TexMetadata &metadata2 = mipImages2.GetMetadata();
  ID3D12Resource *textureResource2 = CreateTextureResource(device, metadata2);
  ID3D12Resource *intermediateResource2 =
      UploadTextureData(textureResource2, mipImages2, device, commandList);

  // mataDataを基にSRVの設定
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = metadata.format;
  srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
  srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

  // 2個目
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
  srvDesc2.Format = metadata2.format;
  srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
  srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

  // SRVを作成するDescriptorHeapの場所を決める
  D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU =
      srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
  D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU =
      srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
  // 戦闘はImGuiが使っているのでその次に使う
  textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  // SRVの生成
  device->CreateShaderResourceView(textureResource, &srvDesc,
                                   textureSrvHandleCPU);

  // DepthStencilTextureをウィンドウのサイズで作成
  ID3D12Resource *depthStencilResource =
      CreateDepthStencilTextureResource(device, kClientWidth, kClientheight);


  // DSV用のヒープでディスクリプタの数は1。DSVはShader内で触れるものではないので、ShaderVisibleはfalse
  ID3D12DescriptorHeap *dsvDescriptorHeap =
      CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

  // DSVの設定
  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
  dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // Format。基本的にはResourceに合わせる
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; // 2DTexture
  // DSVHeapの先頭にDSVを作る
  device->CreateDepthStencilView(
      depthStencilResource, &dsvDesc,
      dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());


  
  // DescriptorSizeを取得しておく
  const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  const uint32_t descriptorSizeRTV =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  const uint32_t descriptorSizeDSV =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


  // SRVを作成するDescriptorHeapの場所を決める
  D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 =
      GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
  D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 =
      GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
  // SRVの生成
  device->CreateShaderResourceView(textureResource2, &srvDesc2,
                                   textureSrvHandleCPU2);


  ID3D12Resource *indexResourceSprite =
      CreateBufferResource(device, sizeof(uint32_t) * 6);
  
  D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
  // リソースの先頭のアドレスから使う
  indexBufferViewSprite.BufferLocation =
      indexResourceSprite->GetGPUVirtualAddress();
  // 使用するリソースのサイズはインデックス6つ分のサイズ
  indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
  // インデックスはuint32_tとする
  indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

  // インデックスリソースにデータを書き込む
  uint32_t *indexDataSprite = nullptr;
  indexResourceSprite->Map(0, nullptr,
                           reinterpret_cast<void **>(&indexDataSprite));
  indexDataSprite[0] = 0;
  indexDataSprite[1] = 1;
  indexDataSprite[2] = 2;
  indexDataSprite[3] = 1;
  indexDataSprite[4] = 3;
  indexDataSprite[5] = 2;



  bool useMonsterBall = true;
  bool texture = true;


  MSG msg{};
  // ウィンドウの×ボタンが押されるまでループ
  while (msg.message != WM_QUIT) {
      // Windowにメッセージが来てたら最優先で処理させる
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      ImGui_ImplDX12_NewFrame();
      ImGui_ImplWin32_NewFrame();
      ImGui::NewFrame();
      // ゲームの処理
      // これから書き込むバックバッファのインデックスを取得
      UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

      // TransitionBarrierの設定
      D3D12_RESOURCE_BARRIER barrier{};
      // 今回のバリアはTransition
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      // Noneにしておく
      barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      // バリアを張る対象のリリース。現在のバックバッファに対して行う
      barrier.Transition.pResource = swapChainResources[backBufferIndex];
      // 遷移前(現在)のResourceState
      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
      // 遷移後のResourceState
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
      // TransitionBarrierを張る 
      commandList->ResourceBarrier(1, &barrier);


      transform.rotate.y += 0.03f;
      Matrix4x4 worldMatrix = MakeAffineMatrix(
          transform.scale, transform.rotate, transform.translate);
      Matrix4x4 cameraMatrix =
          MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate,
                           cameraTransform.translate);
      Matrix4x4 viewMatrix = Inverse(cameraMatrix);
      Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
          0.45f, float(kClientWidth) / float(kClientheight), 0.1f, 100.0f);
      Matrix4x4 worldViewProjectionMatrix =
          Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
      transformationMatrixData->WVP = worldViewProjectionMatrix;
      transformationMatrixData->World = worldMatrix;
      //*wvpData = worldMatrix;


      // Sprite用のWorldProjectionMatrixを作る
      Matrix4x4 worldMatrixSprite =
          MakeAffineMatrix(transformSprite.scale, transformSprite.rotate,
                           transformSprite.translate);
      Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
      Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(
          0.0f, 0.0f, float(kClientWidth), float(kClientheight), 0.0f, 100.0f);
      Matrix4x4 worldViewProjectionMatrixSprite =
          Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
      transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
      transformationMatrixDataSprite->World = worldMatrixSprite;


      Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
      uvTransformMatrix = Multiply(
          uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
      uvTransformMatrix = Multiply(
          uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
      materialDataSprite->uvTransform = uvTransformMatrix;


      ImGui::Begin("MaterialColor");
      ImGui::ColorEdit4("Color", &(*materialData).color.x);
      ImGui::Checkbox("useMonsterBall", &useMonsterBall);
      ImGui::Checkbox("texture", &texture);
      ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f,
                        -10.0f, 10.0f);
      ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f,
                        10.0f);
      ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
      ImGui::End();
      // 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
      ImGui::ShowDemoWindow();
      // ImGuiの内部コマンドを生成する
      ImGui::Render();






      // 描画先のRTVとDSVを設定する
      D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
          dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
      commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false,
                                        &dsvHandle);
      // 指定した深度で画面全体をクリアする
      commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH,
                                         1.0f, 0, 0, nullptr);

      // 描画用のDescriptorHeapの設定
      ID3D12DescriptorHeap *descriptorHeaps[] = {srvDescriptorHeap};
      commandList->SetDescriptorHeaps(1, descriptorHeaps);

      // 指定した色で画面全体をクリアする
      float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f};
      commandList->ClearRenderTargetView(rtvHandles[backBufferIndex],
                                           clearColor, 0, nullptr);

      commandList->RSSetViewports(1, &viewport); // Viewportを設定
      commandList->RSSetScissorRects(1, &scissorRect); // Scirssorを設定
      // RootSignatureを設定。PSOに設定しているけど別途設定が必要
      commandList->SetGraphicsRootSignature(rootSignature);
      commandList->SetPipelineState(graphicsPipelineState); // PSOを設定
      commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
      commandList->IASetIndexBuffer(&indexBufferViewSprite); // IBVを設定
      // 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけばいい
      commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

      // マテリアルCBufferの場所を設定
      commandList->SetGraphicsRootConstantBufferView(
          0, materialResource->GetGPUVirtualAddress());
      // wvp用のCBufferの場所を設定
      commandList->SetGraphicsRootConstantBufferView(
          1, wvpResource->GetGPUVirtualAddress());

      // SRVのDescriptorTableの先頭を設定。2はrootParamater[2]である
      commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

      commandList->SetGraphicsRootConstantBufferView(
          3, directionalLightResource->GetGPUVirtualAddress());


      // 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンスについては今後
      //commandList->DrawInstanced(kSphereVertexNum, 1, 0, 0);
      commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);
      commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

      commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

      // マテリアルCBufferの場所を設定
      commandList->SetGraphicsRootConstantBufferView(
          0, materialResourceSprite->GetGPUVirtualAddress());

      commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
      commandList->SetGraphicsRootConstantBufferView(
          1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
      commandList->DrawInstanced(6, texture, 0, 0);


      // 実際のcommandListのImGuiの描画コマンドを積む
      ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

      // 画面に描く処理はすべて終わり、画面映すので、状態を遷移
      // 今回はRenderTargetからPresentにする
      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
      // TransitionBarrierを張る
      commandList->ResourceBarrier(1, &barrier);

      // コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
      hr = commandList->Close();
      assert(SUCCEEDED(hr));

      // GPUにコマンドリストの実行を行わせる
      ID3D12CommandList *commandLists[] = {commandList};
      commandQueue->ExecuteCommandLists(1, commandLists);
      // GPUとOSに画面の交換を行うよう通知する
      swapChain->Present(1, 0);

      // Fenceの値を更新
      fenceValue++;
      // GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
      commandQueue->Signal(fence, fenceValue);

      // Fenceの値が指定したSignal値にたどり着いているか確認する
      // GetCompletwdValueの初期値はFence作成時に渡した初期値
      if (fence->GetCompletedValue() < fenceValue) {
        // 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
        fence->SetEventOnCompletion(fenceValue, fenceEvent);
        // イベント待つ
        WaitForSingleObject(fenceEvent, INFINITE);
      }

      // 次のフレーム用のコマンドリストを準備
      hr = commandAllocator->Reset();
      assert(SUCCEEDED(hr));
      hr = commandList->Reset(commandAllocator, nullptr);
      assert(SUCCEEDED(hr));
    }
  }

  CoUninitialize();

  // こういうもんである。初期化と逆順に行う
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  Log(logStream, "Hello,DirectX!\n");
  Log(logStream, ConvertString(std::format(L"clientSize:{},{}\n", kClientWidth,
                  kClientheight)));

  CloseHandle(fenceEvent);
  CloseWindow(hwnd);
  fence->Release();
  rtvDescriptorHeap->Release();
  swapChainResources[0]->Release();
  swapChainResources[1]->Release();
  swapChain->Release();
  commandList->Release();
  commandAllocator->Release();
  commandQueue->Release();

  useAdapter->Release();
  dxgiFactory->Release();
  #ifdef _DEBUG
  debugController->Release();
  #endif 

  srvDescriptorHeap->Release();
  dxcUtils->Release();
  dxcCompiler->Release();
  includeHandler->Release();
  vertexResource->Release();
  graphicsPipelineState->Release();
  signatureBlob->Release();
  if (errorBlob) {
    errorBlob->Release();
  }
  rootSignature->Release();
  pixelShaderBlob->Release();
  vertexShaderBlob->Release();
  materialResource->Release();
  materialResourceSprite->Release();
  directionalLightResource->Release();
  wvpResource->Release();
  textureResource->Release();
  textureResource2->Release();
  intermediateResource->Release();
  intermediateResource2->Release();
  depthStencilResource->Release();
  dsvDescriptorHeap->Release();
  vertexResourceSprite->Release();
  indexResourceSprite->Release();
  transformationMatrixResourceSprite->Release();


  device->Release();

  // リソースリークチェック
  IDXGIDebug1 *debug;
  if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
    debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
    debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
    debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
    debug->Release();
  }



  return 0;
}