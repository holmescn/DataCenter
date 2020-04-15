#include <Windows.h>
#include <shellapi.h>
#include <sstream>
#include <spdlog/spdlog.h>
#include "mainwindow.h"
#include "stockdrv.h"

void MainWindow::PaintWindow(HDC& hdc, RECT& rect)
{
	std::wstringstream wcs;
	switch (m_state)
	{
	case MainWindow::State::Start:
		wcs << L"NeZip Receiver 启动";
		break;
	case MainWindow::State::StartNeZip:
		wcs << L"正在启动 NeZip";
		break;
	case MainWindow::State::StartDrv:
		wcs << L"正在启动 StockDrv";
		break;
	case MainWindow::State::ReceiveData:
	{
		wcs << L"正在接收数据：" << std::endl
			<< L"已接收 Tick 数据 " << m_nTotalReceived << std::endl
			<< L"已转发 Tick 数据 " << m_nTotalSent << std::endl;
	} break;
	case MainWindow::State::Error:
		wcs << L"发生错误:" << std::endl << m_wcsError;
		break;
	default:
		wcs << L"未知状态，请检查代码！";
		break;
	}

	std::wstring txt = wcs.str();
	DrawText(hdc, txt.c_str(), txt.length(), &rect, DT_LEFT | DT_TOP);
}

void MainWindow::StartNeZip()
{
	WCHAR szExeFile[MAX_PATH];
	wsprintf(szExeFile, L"%s\\Nezip.exe", m_szNeZipPath);

	auto rc = (int)ShellExecute(NULL, NULL, szExeFile, NULL, m_szNeZipPath, SW_NORMAL);
	if (rc > 32) {
		m_state = State::StartNeZip;
		return;
	}

	m_state = State::Error;
	switch (rc) {
	case ERROR_FILE_NOT_FOUND:
		m_wcsError = L"没找到 ";
		m_wcsError += szExeFile;
		break;
	case ERROR_PATH_NOT_FOUND:
		m_wcsError = L"没找到 ";
		m_wcsError += m_szNeZipPath;
		break;
	default:
		m_wcsError = L"启动 NeZip 时发生错误。";
		break;
	}
}

void MainWindow::KillTimer()
{
	::KillTimer(m_hWnd, IDT_TIMER0);
}

bool MainWindow::LoadDll()
{
	m_hStockDll = NULL;
	Stock_Init = NULL;
	Stock_Quit = NULL;
	GetStockDrvInfo = NULL;

	WCHAR szDllFile[MAX_PATH] = { 0 };
	wsprintf(szDllFile, L"%s\\System\\Stockdrv.dll", m_szNeZipPath);
	m_hStockDll = LoadLibrary(szDllFile);
	if (m_hStockDll == NULL) {
		m_wcsError = L"载入 Dll 文件失败";
		m_state = State::Error;
		return false;
	}

	Stock_Init = (fnStockInit)GetProcAddress(m_hStockDll, "Stock_Init");
	if (Stock_Init == NULL) {
		m_wcsError = L"载入 Stock_Init 函数失败";
		m_state = State::Error;
		return false;
	}

	Stock_Quit = (fnStockQuit)GetProcAddress(m_hStockDll, "Stock_Quit");
	if (Stock_Quit == NULL) {
		m_wcsError = L"载入 Stock_Quit 函数失败";
		m_state = State::Error;
		return false;
	}

	/* 暂时没用，先注释掉
	GetStockDrvInfo = (fnGetStockDrvInfo)GetProcAddress(m_hStockDll, "GetStockDrvInfo");
	if (GetStockDrvInfo == NULL) {
		m_state = State::Error;
		return false;
	}
	*/

	return true;
}

void MainWindow::MoveWindow()
{
	RECT rect = { 0 };
	GetWindowRect(m_hWndNeZip, &rect);
	LONG newX = rect.left - 5;
	LONG newY = rect.top;

	GetWindowRect(m_hWnd, &rect);
	newY -= rect.bottom - rect.top - 5;

	SetWindowPos(m_hWnd, m_hWndNeZip, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void MainWindow::Fatal(const WCHAR *message, int rv)
{
	char mbsString[KB];
	sprintf_s(mbsString, "[%d] %s", rv, nng_strerror(rv));
	
	size_t nCh = 0;
	WCHAR wcsString[KB];
	mbstowcs_s(&nCh, wcsString, mbsString, KB);

	std::wstringstream wcs;
	wcs << message << L" " << wcsString;
	m_wcsError = wcs.str();
	m_state = State::Error;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		if (m_hStockDll) {
			Stock_Quit(m_hWnd);
			FreeLibrary(m_hStockDll);
		}
		m_pExit.set_value();
		WaitWorkers();
		DestroyWindow(m_hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		this->StateMachine();
		RedrawWindow(this->m_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
		break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(m_hWnd, &ps);
		RECT rect = { 0 };
		GetClientRect(m_hWnd, &rect);
		FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW));

		rect.top += 5;
		rect.left += 10;
		// Black Text
		SetTextColor(hdc, RGB(0, 0, 0));
		this->PaintWindow(hdc, rect);
		EndPaint(m_hWnd, &ps);
	} break;

	case WM_RECV_DATA:
		OnRecvData(wParam, lParam);
		break;

	case WM_SENT_ONE_RECORD:
		m_nTotalSent += 1;
		break;

	default:
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}

	return TRUE;
}

void MainWindow::StateMachine()
{
	switch (m_state)
	{
	case MainWindow::State::Start:
		StartNeZip();
		break;
	case MainWindow::State::StartNeZip:
		EnumWindows(MainWindow::CheckNeZipStarted, (LPARAM)this);
		break;
	case MainWindow::State::StartDrv:
	{
		if (LoadDll()) {
			StartDrv();
		}
	}   break;
	case MainWindow::State::ReceiveData:
		break;
	case MainWindow::State::Error:
		this->KillTimer();
		break;
	default:
		break;
	}
}

void MainWindow::StartDrv()
{
	int rc = Stock_Init(m_hWnd, WM_RECV_DATA, RCV_WORK_SENDMSG);
	if (rc == -1) {
		m_wcsError = L"Stock_Init 失败";
		m_state = State::Error;
	}
	else {
		m_state = State::ReceiveData;
	}
}

BOOL MainWindow::CheckNeZipStarted(HWND hWnd, LPARAM lParam)
{
	MainWindow* pThis = reinterpret_cast<MainWindow*>(lParam);
	const WCHAR* pTitle = pThis->m_wcsNeZipTitle.c_str();

	size_t nTextLen = GetWindowTextLength(hWnd);
	if (nTextLen > 0) {
		nTextLen = nTextLen + 1;
		WCHAR* pText = (WCHAR*)_malloca(sizeof(WCHAR) * nTextLen);
		GetWindowText(hWnd, pText, nTextLen);
		if (wcsstr(pText, pTitle)) {
			pThis->m_hWndNeZip = hWnd;
			pThis->MoveWindow();
			pThis->m_state = MainWindow::State::StartDrv;
			return FALSE;
		}
	}

	return TRUE;
}

void MainWindow::LoadConfig(LPCWSTR szIniFilePath)
{
	GetPrivateProfileString(
		L"NeZip",
		L"Path",
		NULL,
		m_szNeZipPath,
		MAX_PATH,
		szIniFilePath);

	if (wcslen(m_szNeZipPath) == 0) {
		m_wcsError = L"配置文件中没有设置 NeZip 的路径。";
		m_state = State::Error;
		return;
	}

	WCHAR wcsBuffer[1 * KB];
	GetPrivateProfileString(
		L"NeZip",
		L"Title",
		L"",
		wcsBuffer,
		KB,
		szIniFilePath);

	m_wcsNeZipTitle = wcsBuffer;

	if (m_wcsNeZipTitle.length() == 0) {
		m_wcsError = L"配置文件中没有设置 NeZip 的标题。";
		m_state = State::Error;
		return;
	}

	GetPrivateProfileString(
		L"NeZipReceiver",
		L"Server",
		L"tcp://127.0.0.1:8008",
		wcsBuffer,
		1*KB,
		szIniFilePath);

	size_t nCh = 0;
	char mbsBuffer[KB];
	wcstombs_s(&nCh, mbsBuffer, wcsBuffer, KB);
	m_uri = mbsBuffer;
}

void MainWindow::StartWorkers()
{
	int rv;

	/*  Create the socket. */
	rv = nng_req0_open(&m_sock);
	if (rv != 0) {
		Fatal(L"创建 req socket 错误：", rv);
		return;
	}

	std::shared_future<void> fExit = m_pExit.get_future();
	for (size_t i = 0; i < PARALLEL; i++) {
		m_workers.emplace_front(m_bufferQueue, m_hWnd, fExit);
		Worker& w = m_workers.front();

		if ((rv = nng_aio_alloc(&w.aio, Worker::aio_cb, &w)) != 0) {
			Fatal(L"nng_aio_alloc 失败：", rv);
			return;
		}

		if ((rv = nng_ctx_open(&w.ctx, m_sock)) != 0) {
			Fatal(L"nng_ctx_open 错误：", rv);
			return;
		}
	}

	bool connected = false;
	do {
		if ((rv = nng_dial(m_sock, m_uri.c_str(), NULL, 0)) == 0) {
			m_state = State::Start;
			connected = true;
		}
		else {
			Fatal(L"连接到服务器失败：", rv);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	} while (!connected);

	for (auto &w : m_workers) {
		// this starts them going (INIT state)
		Worker::aio_cb(&w);
	}
}

void MainWindow::WaitWorkers()
{
	for (auto& w : m_workers) {
		nng_aio_wait(w.aio);
	}

	m_workers.clear();
}

void MainWindow::OnRecvData(WPARAM wParam, LPARAM lParam)
{
	auto pRcvData = (RCV_DATA*)lParam;
	switch (wParam) {
	case RCV_REPORT:
		OnRecvReport(pRcvData);
		break;
	case RCV_FILEDATA:
		OnRecvFileData(pRcvData);
		break;
	default:
		m_wcsError = L"无效的 RecvData 类型";
		spdlog::error("Invalid wParam for OnRecvData: {}", wParam);
		break;
	}
}

void MainWindow::OnRecvReport(RCV_DATA* pRcvData)
{
	RCV_REPORT_STRUCTEx *pData = pRcvData->m_pReport;
	size_t nPacketNum = pRcvData->m_nPacketNum;
	m_bufferQueue.enqueue_bulk(pData, nPacketNum);
	m_nTotalReceived += nPacketNum;
}

void MainWindow::OnRecvFileData(RCV_DATA* pRcvData)
{
	//
}

MainWindow::MainWindow()
	: m_hWndNeZip(NULL), m_state(State::Start),
	m_nTotalReceived(0), m_nTotalSent(0)
{
	ZeroMemory(m_szNeZipPath, sizeof m_szNeZipPath);
}

MainWindow::~MainWindow()
{
	nng_close(m_sock);
}
