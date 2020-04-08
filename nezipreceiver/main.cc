#include "winsock_initializer.h"
#include "mainwindow.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// 不使用 Redis 暂时没用
	// winsock_initializer winsock_init;

	// Get Exe Path
	WCHAR szIniFilePath[MAX_PATH];
	GetModuleFileName(NULL, szIniFilePath, MAX_PATH);
	WCHAR *pEnd = wcsrchr(szIniFilePath, L'\\');
	wcscpy_s(pEnd + 1, 18, L"nezipreceiver.ini");

	MainWindow *win = new MainWindow();
	win->LoadConfig(szIniFilePath);
	win->StartWorkers();

	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	int nWidth = 300, nHeight = 200;
	if (!win->Create(hInstance, L"NeZipReceiver", dwStyle, 0, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight)) {
		return 0;
	}

	ShowWindow(win->hWnd(), nCmdShow);

	// Run the message loop.
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete win;

	return 0;
}
