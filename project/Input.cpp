#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
  HRESULT result;
  // DirectInputのインスタンス生成
  ComPtr<IDirectInput8> directInput = nullptr;
  result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
  assert(SUCCEEDED(result));
  // キーボードデバイス生成
  result = directInput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);
  assert(SUCCEEDED(result));
  // 入力データ形式のセット
  result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);
  // 排他制御レベルのセット
  result = devkeyboard->SetCooperativeLevel(
      hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
  assert(SUCCEEDED(result));
}

void Input::update() {
  HRESULT result;

  // 前回のキー入力を保存
  memcpy(keyPre, key, sizeof(key));
  // キーボード情報の取得開始
  result = devkeyboard->Acquire();
  // 全キーの入力情報を取得する
  result = devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber) { 
    // 指定キーを押していればtrueに返す
  if (key[keyNumber]) {
      return true;
  }

    return false; 
}

bool Input::TriggerKey(BYTE keyNumber)
{
    return (key[keyNumber] && !keyPre[keyNumber]);
}
