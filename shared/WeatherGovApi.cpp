#include "WeatherGovApi.h"

#include <atltime.h>
#include <codecvt>
#include <fstream>

WeatherGovApi::WeatherGovApi(void (*callback)(float)) {
    m_callback = callback;
    m_session.Initialize(L"TrayTemperature", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
}

void WeatherGovApi::DownloadTemperature() {
    m_connection.Close();
    Close();

    m_connection.Initialize(m_session, L"api.weather.gov", INTERNET_DEFAULT_HTTPS_PORT);
    Initialize(m_connection, L"/gridpoints/LOT/74,72", L"GET", NULL, WINHTTP_NO_REFERER, static_cast<LPCWSTR*>(m_accepts), WINHTTP_FLAG_REFRESH | WINHTTP_FLAG_SECURE);
    SendRequest();
}

bool GetCurrentTemperature(const nlohmann::json& weatherGovResponse, float* pTemperature) {
    if (pTemperature == NULL)
        return false;

    CTime cCurrent = CTime::GetCurrentTime();

    for (auto item : weatherGovResponse["properties"]["temperature"]["values"])
    {
        float temp = item["value"];
        std::string timeStr = item["validTime"];

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wTimeStr = converter.from_bytes(timeStr);

        SYSTEMTIME time{};
        short offsetHour = 0;
        unsigned short offsetMinute = 0;
        unsigned short durationHour = 0;
        swscanf_s(wTimeStr.c_str(), L"%hu-%hu-%huT%hu:%hu:%hu%hd:%hu/PT%huH", &time.wYear, &time.wMonth, &time.wDay, &time.wHour, &time.wMinute, &time.wSecond, &offsetHour, &offsetMinute, &durationHour);

        FILETIME fileTime;
        SystemTimeToFileTime(&time, &fileTime);
        CTime cTime(fileTime);
        CTimeSpan cDuration(0, durationHour, 0, 0);

        if (cTime < cCurrent && cCurrent < cTime + cDuration) {
            *pTemperature = temp;
            return true;
        }
    }

    return false;
}

HRESULT WeatherGovApi::OnReadCompleteCallback(_In_ HINTERNET hInternet, _In_ DWORD dwInternetStatus, _In_opt_ LPVOID lpvStatusInformation, _In_ DWORD dwStatusInformationLength)
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
        std::ofstream fout("E:\\projects\\TemperatureIcon\\temp.json");
        fout << j;
        fout.close();
#endif

        float temp;
        if (GetCurrentTemperature(j, &temp))
            m_callback(temp);
    }

    return hr;
}