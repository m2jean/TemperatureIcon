#pragma once

#include "CTempDownloader.h"

class OpenWeatherApi : CTempDownloader {
public:
    OpenWeatherApi(void(*currentTempCallback)(float));
    bool DownloadTemperature();

    /// <summary>
    /// https://openweathermap.org/current#zip
    /// </summary>
    void DownloadTemperature(int zip, const WCHAR* country, const WCHAR* unit, const WCHAR* language, const WCHAR* apiKey);
protected:
    virtual bool GetCurrentTemp(nlohmann::json, float* currentTemp);
};
