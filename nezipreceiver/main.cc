#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "mainwindow.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
	// Get Exe Path
	WCHAR szIniFilePath[MAX_PATH];
	GetModuleFileName(NULL, szIniFilePath, MAX_PATH);

	size_t nCh = 0;
	char szLogFilePath[MAX_PATH];
	wcstombs_s(&nCh, szLogFilePath, szIniFilePath, MAX_PATH);

	WCHAR *pEnd = wcsrchr(szIniFilePath, L'\\');
	wcscpy_s(pEnd + 1, 18, L"nezipreceiver.ini");

	char* pSep = strrchr(szLogFilePath, '\\');
	strcpy_s(pSep + 1, 18, "nezipreceiver.log");

	// Create color multi threaded logger
	// and set as the default logger
	auto file_logger = spdlog::basic_logger_mt("NeZipReceiver", szLogFilePath);
	spdlog::set_default_logger(file_logger);

	// change log pattern
	spdlog::set_pattern("[%m-%d %H:%M:%S] [%n] [%^-%L-%$] %v");

	MainWindow win;
	win.LoadConfig(szIniFilePath);
	win.StartWorkers();

	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	int nWidth = 300, nHeight = 200;
	if (!win.Create(hInstance, L"NeZipReceiver", dwStyle, 0, CW_USEDEFAULT, CW_USEDEFAULT, nWidth, nHeight)) {
		return 0;
	}

	ShowWindow(win.hWnd(), nCmdShow);

	// Run the message loop.
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
