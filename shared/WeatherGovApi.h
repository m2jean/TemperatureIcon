#pragma once

#include <windows.h>
#include "json.hpp"
#include "WinHTTPWrappers.h"

class WeatherGovApi : WinHTTPWrappers::CAsyncDownloader {
public:
    WeatherGovApi(void(*currentTempCallback)(float));
    void DownloadTemperature();
private:
    WinHTTPWrappers::CSession m_session;
    WinHTTPWrappers::CConnection m_connection;
    const WCHAR* m_accepts[2] = { L"application/geo+json", NULL };

    void(*m_callback)(float);

    virtual HRESULT OnReadCompleteCallback(_In_ HINTERNET, _In_ DWORD, _In_opt_ LPVOID, _In_ DWORD) override;
};
