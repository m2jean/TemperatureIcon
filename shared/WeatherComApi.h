#pragma once

#include "CTempDownloader.h"

class WeatherComApi : CTempDownloader {
public:
    WeatherComApi(void(*currentTempCallback)(float));
    bool DownloadTemperature();

    /// <summary>
    /// https://weather.com/swagger-docs/ui/sun/v3/sunV3CurrentsOnDemand.json
    /// </summary>
    void DownloadTemperature(float latitue, float longitude, WCHAR unit, const WCHAR* language, const WCHAR* apiKey);
protected:
    virtual bool GetCurrentTemp(nlohmann::json, float* currentTemp);
private:
    const WCHAR* m_accepts[2] = { L"application/json", NULL };
};
