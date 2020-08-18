#include "WeatherComApi.h"

#include <atltime.h>
#include <codecvt>
#include <fstream>

WeatherComApi::WeatherComApi(void (*callback)(float)) {
    m_callback = callback;
    m_session.Initialize(L"TrayTemperature", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
}

bool WeatherComApi::DownloadTemperature() {
    std::ifstream fin("apiKey");
    char buf[64] = { 0 };
    fin >> buf;
    fin.close();
    if (buf[0] == 0)
        return false;

    WCHAR wbuf[64];
    int size = MultiByteToWideChar(CP_UTF8, 0, buf, 64, wbuf, 64);

    DownloadTemperature(41.8786f, -87.6251f, L'm', L"en-US", wbuf);
    return true;
}

void WeatherComApi::DownloadTemperature(float latitue, float longitude, WCHAR unit, const WCHAR *language, const WCHAR *apiKey) {
    m_connection.Close();
    Close();

    WCHAR buf[1024];
    swprintf_s(buf, L"/v3/wx/observations/current?geocode=%.2f%%2C%.2f&units=%c&language=%s&format=json&apiKey=%s", latitue, longitude, unit, language, apiKey);

    m_connection.Initialize(m_session, L"api.weather.com", INTERNET_DEFAULT_HTTPS_PORT);
    Initialize(m_connection, buf, L"GET", NULL, WINHTTP_NO_REFERER, static_cast<LPCWSTR*>(m_accepts), WINHTTP_FLAG_REFRESH | WINHTTP_FLAG_SECURE);
    SendRequest();
}

HRESULT WeatherComApi::OnReadCompleteCallback(_In_ HINTERNET hInternet, _In_ DWORD dwInternetStatus, _In_opt_ LPVOID lpvStatusInformation, _In_ DWORD dwStatusInformationLength)
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
        std::ofstream fout("currentOnDemand.json");
        fout << j;
        fout.close();
#endif

        float temp = j["temperature"];
        m_callback(temp);
    }

    return hr;
}