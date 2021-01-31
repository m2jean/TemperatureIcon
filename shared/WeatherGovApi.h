#pragma once

#include "CTempDownloader.h"

class WeatherGovApi : CTempDownloader {
public:
    WeatherGovApi(void(*currentTempCallback)(float));
    bool DownloadTemperature();
protected:
    virtual bool GetCurrentTemp(nlohmann::json, float* currentTemp);
private:
    const WCHAR* m_accepts[2] = { L"application/geo+json", NULL };
};
