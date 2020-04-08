#pragma once

#include <windows.h>

#define IDT_TIMER0 1


template <typename TWindow>
class BaseWindow
{

public:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		TWindow* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (TWindow*)pCreate->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hWnd = hWnd;

			SetTimer(hWnd, IDT_TIMER0, 1000/* ms */, NULL);
		}
		else
		{
			pThis = (TWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}

		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow() : m_hWnd(NULL) { }
	virtual ~BaseWindow() {}

	BOOL Create(
		HINSTANCE hInstance,
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		WNDCLASS wc = { 0 };

		wc.lpfnWndProc = TWindow::WindowProc;
		wc.hInstance = hInstance;
		wc.lpszClassName = ClassName();

		RegisterClass(&wc);

		this->m_hWnd = CreateWindowEx(
			dwExStyle, ClassName(), lpWindowName, dwStyle,
			x, y, nWidth, nHeight,
			hWndParent, hMenu, hInstance, this
		);

		return (m_hWnd ? TRUE : FALSE);
	}

	HWND hWnd() const { return m_hWnd; }

protected:

	virtual PCWSTR  ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	HWND m_hWnd;
};
