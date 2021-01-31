// TemperatureTrayIcon.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TemperatureTrayIcon.h"
#include <combaseapi.h>
#include <math.h>
#include "../shared/OpenWeatherApi.h"
#include "../shared/TemperatureIcon.h"
#include "../shared/notifyIcon.h"
#include "../shared/dbprintf.h"

#define MAX_LOADSTRING 100
const UINT WM_TEMP_UPDATED = WM_USER + 1;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND g_mainWnd;

void DowloadTemperature();
void OnTemperatureUpdated(float);

int g_smallIconSize;
HICON g_icon;
OpenWeatherApi g_tempDownloader(OnTemperatureUpdated);
TemperatureIcon g_tempIcon;
UINT_PTR g_uTimer;
CTrayIcon g_trayIcon;
HMENU g_menu;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    CoInitialize(NULL);
    g_smallIconSize = GetSystemMetrics(SM_CXSMICON);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TEMPERATURETRAYICON, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TEMPERATURETRAYICON));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TEMPERATURETRAYICON));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TEMPERATURETRAYICON);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CREATE:
        {
            g_mainWnd = hWnd;
            g_menu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_TEMPERATURETRAYICON));
            HMENU subMenu = GetSubMenu(g_menu, 0);

            LoadIconMetric(hInst, MAKEINTRESOURCE(IDI_SMALL), LIM_SMALL, &g_icon);
            g_trayIcon.Initialize(hWnd, g_icon, 0, L"TemperatureTrayIcon::TrayIcon0", subMenu);

            SetTimer(hWnd, g_uTimer, 1000 * 60 * 10/*10min*/, NULL);
            DowloadTemperature();
        }
        break;
    case WM_TIMER:
        DowloadTemperature();
        break;
    case WM_TEMP_UPDATED:
        {
            float temp = reinterpret_cast<float&>(wParam);

            HICON icon = g_tempIcon.CreateTemperatureIcon(roundf(temp), L'C', g_smallIconSize);
            g_trayIcon.Update(icon);

            DestroyIcon(g_icon);
            g_icon = icon;
        }
        break;
    case WM_DESTROY:
        g_trayIcon.Uninitialize();
        DestroyMenu(g_menu);
        DestroyIcon(g_icon);

        PostQuitMessage(0);
        break;
    default:
        if (g_trayIcon.HandleMessage(message, wParam, lParam))
            break;

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void DowloadTemperature() {
    if (!g_tempDownloader.DownloadTemperature())
    {
        MessageBox(g_mainWnd, TEXT("Cannot find file \"apiKey\""), TEXT("apiKey Error"), MB_OK);
        DestroyWindow(g_mainWnd);
    }
}

void OnTemperatureUpdated(float temp) {
    dbwprintf_s(L"Temperature updated: %f", temp);
    SendMessage(g_mainWnd, WM_TEMP_UPDATED, reinterpret_cast<WPARAM&>(temp), NULL);
}
