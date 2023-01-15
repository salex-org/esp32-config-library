#ifndef ESP32_CONFIG_LIB_HPP
#define ESP32_CONFIG_LIB_HPP

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <vector>

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
		String title;
		EntryType type;
		String key;
		String value;
		String defaultValue;

	public:
		Entry(String title, EntryType type, String key, String defaultValue = "");
		void update(String key, String value);
		void save(Preferences *prefs);
		void load(Preferences *prefs);
		String html(String ns);
	};

	class Namespace
	{
	private:
		String title;
		String name;
		std::vector<Entry> entries;

	public:
		Namespace(String title, String name, std::vector<Entry> entries);
		void update(String key, String value);
		void save();
		void load();
		String html();
	};

	class Server
	{
	private:
		std::vector<Namespace> namespaces;
		String (*styleHandler)();
		int serverPort;
		WebServer webServer;
		void load();
		String create_html();
		void handle_get_request();
		void handle_post_request();
	public:
		Server(std::vector<Namespace> namespaces, String (*styleHandler)() = 0, int port = 80);
		void begin(std::string ssid = "ESP32 Config Server", std::string password = "esp32secret", IPAddress ip = IPAddress(192,168,1,1));
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