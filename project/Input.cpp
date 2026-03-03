#include "Input.h"
#include <cassert>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(WinApp* winApp) {
  HRESULT result;
  this->winApp_ = winApp;
  // DirectInputのインスタンス生成
  ComPtr<IDirectInput8> directInput = nullptr;
  result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&directInput, nullptr);
  assert(SUCCEEDED(result));
  // キーボードデバイス生成
  result = directInput->CreateDevice(GUID_SysKeyboard, &devkeyboard_, NULL);
  assert(SUCCEEDED(result));
  // 入力データ形式のセット
  result = devkeyboard_->SetDataFormat(&c_dfDIKeyboard);
  // 排他制御レベルのセット
  result = devkeyboard_->SetCooperativeLevel(
      winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
  assert(SUCCEEDED(result));
}

void Input::update() {
  HRESULT result;

  // 前回のキー入力を保存
  memcpy(keyPre_, key_, sizeof(key_));
  // キーボード情報の取得開始
  result = devkeyboard_->Acquire();
  // 全キーの入力情報を取得する
  result = devkeyboard_->GetDeviceState(sizeof(key_), key_);
}

bool Input::PushKey(BYTE keyNumber) { 
    // 指定キーを押していればtrueに返す
  if (key_[keyNumber]) {
      return true;
  }
    return false; 
}

bool Input::TriggerKey(BYTE keyNumber)
{
    return (key_[keyNumber] && !keyPre_[keyNumber]);
}
