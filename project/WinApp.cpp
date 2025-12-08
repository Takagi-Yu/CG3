#include "WinApp.h"

void WinApp::Initialize() {
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"CGWindowClass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

}

void WinApp::Update() {

}
