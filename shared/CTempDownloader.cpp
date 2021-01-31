
#include <fstream>
#include "CTempDownloader.h"

CTempDownloader::CTempDownloader(void (*callback)(float)) {
    m_callback = callback;
    m_session.Initialize(L"TrayTemperature", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
}

HRESULT CTempDownloader::OnReadCompleteCallback(_In_ HINTERNET hInternet, _In_ DWORD dwInternetStatus, _In_opt_ LPVOID lpvStatusInformation, _In_ DWORD dwStatusInformationLength)
{
    HRESULT hr = ERROR_SUCCESS;
    if (dwStatusInformationLength > 0) { // response incomplete, call base class to buffer the response
        hr = WinHTTPWrappers::CAsyncDownloader::OnReadCompleteCallback(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
    }
    else {
        const char* data = reinterpret_cast<const char*>(m_Response.data());
        int output_size = MultiByteToWideChar(CP_UTF8, 0, data, m_Response.size(), NULL, 0);
        wchar_t* converted_buf = new wchar_t[output_size];
        int size = MultiByteToWideChar(CP_UTF8, 0, data, m_Response.size(), converted_buf, output_size);

        nlohmann::json j = nlohmann::json::parse(converted_buf, converted_buf + size);
        delete[] converted_buf;

        m_connection.Close();
        Close();
        ReleaseResources();

#ifdef DEBUG
        std::ofstream fout("lastDownload.json");
        fout << j;
        fout.close();
#endif

        float temp;
        if (GetCurrentTemp(j, &temp))
            m_callback(temp);
    }

    return hr;
}