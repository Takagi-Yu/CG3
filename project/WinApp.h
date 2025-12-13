#include <Windows.h>
#include <stdint.h>

#pragma once
class WinApp
{
public: // 静寂メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:

	void Initialize();
	void Update();
	void Finalize();

	// getter
	HWND GetHwnd()const { return hwnd_; }
	HINSTANCE GetHInstance()const { return wc_.hInstance; }

	bool ProcessMessage();

public: // 定数

	static const int32_t kClientWidth_ = 1280;
	static const int32_t kClientHeight_ = 720;

private:

	HWND hwnd_ = nullptr;
	WNDCLASS wc_{};
};

