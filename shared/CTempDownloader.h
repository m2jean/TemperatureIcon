#pragma once

#include <windows.h>
#include "json.hpp"
#include "WinHTTPWrappers.h"

class CTempDownloader : protected WinHTTPWrappers::CAsyncDownloader {
public:
    CTempDownloader(void(*currentTempCallback)(float));
    virtual bool DownloadTemperature() = 0;
protected:
    WinHTTPWrappers::CSession m_session;
    WinHTTPWrappers::CConnection m_connection;

    void(*m_callback)(float);

    virtual bool GetCurrentTemp(nlohmann::json, float* currentTemp) = 0;
private:
    virtual HRESULT OnReadCompleteCallback(_In_ HINTERNET, _In_ DWORD, _In_opt_ LPVOID, _In_ DWORD) override;

};