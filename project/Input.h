#pragma once
#include "WinApp.h"
#include <windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#define DIRECTINPUT_VERSION    0x0800
#include <dinput.h>

class Input {
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

  void Initialize(WinApp* winApp);
  void update();

  bool PushKey(BYTE keyNumber);
  bool TriggerKey(BYTE keyNumber);

private:
	WinApp *winApp_ = nullptr;

  // キーボードのデバイス
  ComPtr<IDirectInputDevice8> devkeyboard_;

  BYTE key_[256] = {};
  BYTE keyPre_[256] = {};
};
