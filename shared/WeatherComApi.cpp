#include "WeatherComApi.h"
#include <fstream>

WeatherComApi::WeatherComApi(void (*callback)(float)) : CTempDownloader(callback) {
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

bool WeatherComApi::GetCurrentTemp(nlohmann::json json, float* currentTemp) {
    *currentTemp = json["temperature"];
    return true;
}