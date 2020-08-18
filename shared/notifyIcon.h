#pragma once

#include <Windows.h>
#include <shellapi.h>
#include <windowsx.h>

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
// necessary for Shell_NotifyIcon
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "dbprintf.h"

struct CTrayIcon {
public:
	~CTrayIcon() {
		Uninitialize();
	}

	void Initialize(const HWND hWnd, HICON icon, UINT iconId, const WCHAR* windowMessage, HMENU hContextMenu) {
		if (m_added)
			return;

		m_hWnd = hWnd;
		m_iconId = iconId;
		m_iconMsg = RegisterWindowMessage(windowMessage);
		m_menu = hContextMenu;

		NOTIFYICONDATA notifyIconData = {};
		notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData.hWnd = hWnd;
		notifyIconData.uID = iconId;
		notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE;
		notifyIconData.hIcon = icon;
		notifyIconData.uCallbackMessage = m_iconMsg;

		bool result = Shell_NotifyIcon(NIM_ADD, &notifyIconData);
		if (!result)
			dbwprintf_s(L"Adding tray icon failed.");

		notifyIconData.uVersion = NOTIFYICON_VERSION_4;
		result = Shell_NotifyIcon(NIM_SETVERSION, &notifyIconData);

		m_added = true;
	}

	void Update(HICON newIcon) {
		NOTIFYICONDATA notifyIconData = {};
		notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData.hWnd = m_hWnd;
		notifyIconData.uID = m_iconId;
		notifyIconData.uFlags = NIF_ICON;
		notifyIconData.hIcon = newIcon;

		bool result = Shell_NotifyIcon(NIM_MODIFY, &notifyIconData);
		if (!result)
			dbwprintf_s(L"Updating tray icon failed.");
	}

	void Uninitialize() {
		if (!m_added)
			return;

		NOTIFYICONDATA notifyIconData = {};
		notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
		notifyIconData.hWnd = m_hWnd;
		notifyIconData.uID = 0;
		notifyIconData.guidItem = GUID();

		bool result = Shell_NotifyIcon(NIM_DELETE, &notifyIconData);

		m_added = false;
	}

	bool HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (uMsg != m_iconMsg)
			return false;

		WORD event = LOWORD(lParam);
		if (event == WM_CONTEXTMENU)
		{
			float x = GET_X_LPARAM(wParam);
			float y = GET_Y_LPARAM(wParam);

			// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-trackpopupmenuex
			// To display a context menu for a notification icon, the current window must be the foreground window before the application calls TrackPopupMenu or TrackPopupMenuEx.
			// Otherwise, the menu will not disappear when the user clicks outside of the menu or the window that created the menu (if it is visible).
			// If the current window is a child window, you must set the (top-level) parent window as the foreground window.
			SetForegroundWindow(m_hWnd);

			TrackPopupMenuEx(m_menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON, x, y, m_hWnd, NULL);
		}

		return true;
	}

private:
	bool m_added = false;
	HWND m_hWnd;
	UINT m_iconId;
	UINT m_iconMsg = 0;
	HMENU m_menu;
};
