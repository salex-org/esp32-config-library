#ifndef ESP32_CONFIG_LIB_HPP
#define ESP32_CONFIG_LIB_HPP

#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include "nvs_flash.h"

namespace esp32config
{

	enum EntryType
	{
		TEXT,
		PASSWORD,
		INTEGER
	};

	class Entry
	{
	private:
		std::string title;
		EntryType type;
		std::string key;
		std::string value;
		std::string defaultValue;

	public:
		Entry(const std::string& title, EntryType type, const std::string& key, const std::string& defaultValue = "");
		void update(const std::string& key, const std::string& value);
		void save(const nvs_handle_t& nvs);
		void load(const nvs_handle_t& nvs);
		std::string html();
	};

	class Namespace
	{
	private:
		std::string title;
		std::string name;
		std::vector<Entry> entries;

	public:
		Namespace(const std::string& title, const std::string& name, std::vector<Entry> entries);
		void update(const std::string& key, const std::string& value);
		void save();
		void load();
		std::string html();
	};

	class Server
	{
	private:
		std::vector<Namespace> namespaces;
		std::string (*styleHandler)();
		int serverPort;
		WebServer webServer;
		void load();
		std::string create_html();
		void handle_get_request();
		void handle_post_request();
	public:
		Server(std::vector<Namespace> namespaces, std::string (*styleHandler)() = 0, int port = 80);
		void begin(const std::string& ssid = "ESP32 Config Server", const std::string& password = "esp32secret", IPAddress ip = IPAddress(192,168,1,1));
		void loop();
	};

	class Cli
	{
	private:
		Stream *console;

	public:
		Cli(std::vector<Namespace> namespaces);
		void begin(Stream *console);
		void loop();
	};

}

#endif