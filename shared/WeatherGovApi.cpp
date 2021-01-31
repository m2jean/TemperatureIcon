#include "WeatherGovApi.h"

#include <atltime.h>
#include <codecvt>

WeatherGovApi::WeatherGovApi(void (*callback)(float)) : CTempDownloader(callback) {
}

bool WeatherGovApi::DownloadTemperature() {
    m_connection.Close();
    Close();

    m_connection.Initialize(m_session, L"api.weather.gov", INTERNET_DEFAULT_HTTPS_PORT);
    Initialize(m_connection, L"/gridpoints/LOT/74,72", L"GET", NULL, WINHTTP_NO_REFERER, static_cast<LPCWSTR*>(m_accepts), WINHTTP_FLAG_REFRESH | WINHTTP_FLAG_SECURE);
    SendRequest();
    return true;
}

bool GetCurrentTemperature(const nlohmann::json& weatherGovResponse, float* pTemperature);
bool WeatherGovApi::GetCurrentTemp(nlohmann::json json, float* currentTemp) {
    return GetCurrentTemperature(json, currentTemp);
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