#include "OpenWeatherApi.h"
#include <fstream>

OpenWeatherApi::OpenWeatherApi(void (*callback)(float)) : CTempDownloader(callback) {
}

bool OpenWeatherApi::DownloadTemperature() {
    std::ifstream fin("apiKey");
    char buf[64] = { 0 };
    fin >> buf;
    fin.close();
    if (buf[0] == 0)
        return false;

    WCHAR wbuf[64];
    int size = MultiByteToWideChar(CP_UTF8, 0, buf, 64, wbuf, 64);

    DownloadTemperature(60601, L"us", L"metric", L"en", wbuf);
    return true;
}

void OpenWeatherApi::DownloadTemperature(int zip, const WCHAR* country, const WCHAR* unit, const WCHAR *language, const WCHAR *apiKey) {
    m_connection.Close();
    Close();

    WCHAR buf[1024];
    swprintf_s(buf, L"/data/2.5/weather?zip=%d,%s&units=%s&lang=%s&appid=%s", zip, country, unit, language, apiKey);

    m_connection.Initialize(m_session, L"api.openweathermap.org", INTERNET_DEFAULT_HTTPS_PORT);
    Initialize(m_connection, buf, L"GET", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_REFRESH | WINHTTP_FLAG_SECURE);
    SendRequest();
}

bool OpenWeatherApi::GetCurrentTemp(nlohmann::json json, float* currentTemp) {
    *currentTemp = json["main"]["temp"];
    return true;
}

/*
{"coord":{"lon":-87.6181,"lat":41.8858},"weather":[{"id":701,"main":"Mist","description":"mist","icon":"50d"},{"id":601,"main":"Snow","description":"snow","icon":"13d"}],"base":"stations","main":{"temp":-0.92,"feels_like":-8.7,"temp_min":-1.11,"temp_max":-0.56,"pressure":1015,"humidity":86},"visibility":1609,"wind":{"speed":7.72,"deg":110},"snow":{"1h":0.8},"clouds":{"all":90},"dt":1612046210,"sys":{"type":1,"id":4861,"country":"US","sunrise":1612011883,"sunset":1612047766},"timezone":-21600,"id":0,"name":"Chicago","cod":200}
*/