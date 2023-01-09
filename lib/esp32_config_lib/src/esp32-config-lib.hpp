#ifndef ESP32_CONFIG_LIB_HPP
#define ESP32_CONFIG_LIB_HPP

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <vector>

enum ConfigEntryType
{
    TEXT,
    PASSWORD,
    INTEGER
};

class ConfigEntry
{
private:
	String title;
	ConfigEntryType type;
	String key;
	String value;
	String defaultValue;

public:
	ConfigEntry(String title, ConfigEntryType type, String key, String defaultValue = "");
	void update(String key, String value);
	void save(Preferences *prefs);
	void load(Preferences *prefs);
	String html(String ns);
};

class ConfigNamespace
{
private:
	String title;
	String name;
	std::vector<ConfigEntry> entries;

public:
	ConfigNamespace(String title, String name, std::vector<ConfigEntry> entries);
	void update(String key, String value);
	void save();
	void load();
	String html();
};

class ConfigServer
{
private:
	std::vector<ConfigNamespace> namespaces;
	String (*styleHandler)();
	int serverPort;
	WebServer *webServer;
	void load();
	String html();
public:
	ConfigServer(std::vector<ConfigNamespace> namespaces, String (*styleHandler)() = 0, int port = 80);
	void begin(std::string ssid = "ESP32 Config Server", std::string password = "esp32secret", IPAddress ip = IPAddress(192,168,1,1));
	void loop();
};

class ConfigCli
{
private:
	Stream *console;

public:
	ConfigCli(std::vector<ConfigNamespace> namespaces);
	void begin(Stream *console);
	void loop();
};

#endif