#include <Arduino.h>
#include "esp32-config-lib.hpp"

std::string configStyle()
{
  return "body { background-color: cyan; }";
}

esp32config::Configuration config("Example Config",{
	new esp32config::Namespace("First namespace", "ns1", {
		new esp32config::Entry("Text", esp32config::TEXT, "text", "Default text"),
		new esp32config::Entry("Secret", esp32config::PASSWORD, "secret", "Default secret"),
		new esp32config::Entry("Integer", esp32config::INTEGER, "num", "123")
	}),
    new esp32config::Namespace("Second namespace", "ns2", {
		new esp32config::Entry("Text", esp32config::TEXT, "text"),
		new esp32config::Entry("Secret", esp32config::PASSWORD, "secret"),
		new esp32config::Entry("Integer", esp32config::INTEGER, "num")
	})}
);

esp32config::Server configServer(config, &configStyle);

esp32config::Cli configCLI(config);

void setup()
{
	Serial.begin(115200);
	Serial.println();
	configCLI.begin(&Serial);
	configServer.begin();
}

void loop()
{
	configCLI.loop();
	configServer.loop();
}
