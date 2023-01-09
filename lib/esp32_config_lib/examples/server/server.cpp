#include <Arduino.h>
#include "esp32-config-lib.hpp"

ConfigServer configServer({
	ConfigNamespace("First namespace", "ns1", {
		ConfigEntry("Text", TEXT, "text", "Default text"),
		ConfigEntry("Secret", PASSWORD, "secret", "Default secret"),
		ConfigEntry("Integer", INTEGER, "num", "123")
	}),
    ConfigNamespace("Second namespace", "ns2", {
		ConfigEntry("Text", TEXT, "text"),
		ConfigEntry("Secret", PASSWORD, "secret"),
		ConfigEntry("Integer", INTEGER, "num")
	})}
);

void setup()
{
	Serial.begin(115200);
	Serial.println();
	configServer.begin();
}

void loop()
{
	configServer.loop();
}
