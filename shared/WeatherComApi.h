#pragma once

#include <windows.h>
#include "json.hpp"
#include "WinHTTPWrappers.h"

class WeatherComApi : WinHTTPWrappers::CAsyncDownloader {
public:
    WeatherComApi(void(*currentTempCallback)(float));
    bool DownloadTemperature();

    /// <summary>
    /// https://weather.com/swagger-docs/ui/sun/v3/sunV3CurrentsOnDemand.json
    /// </summary>
    void DownloadTemperature(float latitue, float longitude, WCHAR unit, const WCHAR* language, const WCHAR* apiKey);
private:
    WinHTTPWrappers::CSession m_session;
    WinHTTPWrappers::CConnection m_connection;
    const WCHAR* m_accepts[2] = { L"application/json", NULL };

    void(*m_callback)(float);

    virtual HRESULT OnReadCompleteCallback(_In_ HINTERNET, _In_ DWORD, _In_opt_ LPVOID, _In_ DWORD) override;
};
