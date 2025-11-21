#pragma once
#include <windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;

#define DIRECTINPUT_VERSION    0x0800
#include <dinput.h>

class Input {
public:
  template <class T> using ComPtr=Microsoft::WRL::ComPtr<T>;

  void Initialize(HINSTANCE hInstance,HWND hwnd);
  void update();

private:
  // キーボードのデバイス
  ComPtr<IDirectInputDevice8> keyboard;

};
