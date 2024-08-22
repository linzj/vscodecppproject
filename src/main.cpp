#include <windows.h>

int main() {
  WNDCLASS wc = {};
  HMODULE hInstance = GetModuleHandle(nullptr);
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = L"Win32WindowClass";

  if (!RegisterClass(&wc)) {
    MessageBox(nullptr, L"Failed to register window class.", L"Error", MB_OK);
    return 1;
  }

  HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"Win32 Window",
                             WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                             640, 480, nullptr, nullptr, hInstance, nullptr);

  if (hwnd == nullptr) {
    MessageBox(nullptr, L"Failed to create window.", L"Error", MB_OK);
    return 1;
  }

  ShowWindow(hwnd, SW_SHOWDEFAULT);
  UpdateWindow(hwnd);

  MSG msg = {};
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
