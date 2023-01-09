#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <vector>
#include <algorithm>
#include "nvs_flash.h"
#include <config.h>

WebServer webServer(80);

String configStyle()
{
  return "body { background-color: cyan; }";
}

ConfigServer configServer({
	ConfigNamespace("General", "general", {
		ConfigEntry("NTP Hostname", TEXT, "ntp-host", "pool.ntp.org"),
		ConfigEntry("Display Timeout (ms)", INTEGER, "display-timeout", "60000")
	}),
    ConfigNamespace("WiFi", "wifi", {
		ConfigEntry("SSID", TEXT, "ssid"),
		ConfigEntry("Password", PASSWORD, "password")
	}),
    ConfigNamespace("MQTT", "mqtt", {
		ConfigEntry("Hostname", TEXT, "hostname"),
		ConfigEntry("Username", TEXT, "username"),
		ConfigEntry("Password", PASSWORD, "password")
	})},
    &configStyle
);

void handle_config_page_request()
{
  webServer.send(200, "text/html", configServer.html().c_str());
}

void handle_config_update_request()
{
  if (webServer.hasArg("general.timeserver"))
  {
    String timeserver = webServer.arg("general.timeserver").c_str();
    Serial.printf("Updating configuration, new timeserver value is '%s'\n", timeserver);
    webServer.send(201, "text/html", configServer.html().c_str());
  }
  else
  {
    Serial.println("ERROR: Received post call without data");
    webServer.send(400, "text/html", "missing data");
  }
}

ConfigCli CLI;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  CLI.begin(&Serial);

  configServer.load();

  WiFi.softAP("salex.org Smart Home Agent", "chicago2011");
  WiFi.softAPConfig(IPAddress(192, 168, 10, 1), IPAddress(192, 168, 10, 1), IPAddress(255, 255, 255, 0));
  Serial.println("WiFi access point started");
  delay(100); // Wait 0.1 seconds for the wifi to be up and running

  webServer.on("/", HTTP_GET, handle_config_page_request);
  webServer.on("/", HTTP_POST, handle_config_update_request);
  webServer.begin();
  Serial.println("Web server started");

  Serial.println("Connect to WiFi 'salex.org Smart Home Agent' and open 'http://192.168.10.1' to access the configuration");
}

void loop()
{
  webServer.handleClient();
}
