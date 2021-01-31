# TemperatureIcon
A Win32 program that shows current temperature as tray icon

![Example](/docs/images/trayIcon.png)

### Instruction
* Register on https://openweathermap.org/api
* Put the api key into a file with the name `apiKey` in the working directory

### Features
* Format and render text to a Windows Imaging Component bitmap using Direct Write (and Direct 2D)
* Convert and combine multiple bitmaps of different sizes into one Windows icon file (ICO) using ImageMagick
* Load icon resource in memory and set tray icon (See [How to Load Icons from Memory](docs/images/LoadIconFromMemory.md))
* (optional) Render the bitmaps to the window using Direct2D

### Depeendencies
* Direct2D
* DirectWrite
* Windows Imaging Component
* ImageMagick
* [WinHTTPWrappers](http://www.naughter.com/winhttpwrappers.html)
* [nlohmann/json](https://github.com/nlohmann/json)
* [Open Weather](https://openweathermap.org/) API
* (decommissioned) [The Weather Channel](https://weather.com/) API
* (unused) [National Weather Service API](https://www.weather.gov/documentation/services-web-api)
