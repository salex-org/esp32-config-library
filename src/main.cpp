#include <Arduino.h>
#include "esp32-config-lib.hpp"

String configStyle()
{
  return "body { background-color: cyan; }";
}

esp32config::Server configServer({
	esp32config::Namespace("First namespace", "ns1", {
		esp32config::Entry("Text", esp32config::TEXT, "text", "Default text"),
		esp32config::Entry("Secret", esp32config::PASSWORD, "secret", "Default secret"),
		esp32config::Entry("Integer", esp32config::INTEGER, "num", "123")
	}),
    esp32config::Namespace("Second namespace", "ns2", {
		esp32config::Entry("Text", esp32config::TEXT, "text"),
		esp32config::Entry("Secret", esp32config::PASSWORD, "secret"),
		esp32config::Entry("Integer", esp32config::INTEGER, "num")
	})},
    &configStyle
);

esp32config::Cli configCLI({
	esp32config::Namespace("First namespace", "ns1", {
		esp32config::Entry("Text", esp32config::TEXT, "text", "Default text"),
		esp32config::Entry("Secret", esp32config::PASSWORD, "secret", "Default secret"),
		esp32config::Entry("Integer", esp32config::INTEGER, "num", "123")
	}),
    esp32config::Namespace("Second namespace", "ns2", {
		esp32config::Entry("Text", esp32config::TEXT, "text"),
		esp32config::Entry("Secret", esp32config::PASSWORD, "secret"),
		esp32config::Entry("Integer", esp32config::INTEGER, "num")
	})}
);

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
