#ifndef ESP32_CONFIG_LIB_HPP
#define ESP32_CONFIG_LIB_HPP

#include <Arduino.h>
#include <WebServer.h>
#include <vector>
#include "nvs_flash.h"

namespace esp32config
{
	class Entry;
	class ValidationError;
	class Namespace;
	class Configuration;
	class Server;
	class Cli;

	enum EntryType
	{
		TEXT,
		PASSWORD,
		INTEGER
	};

	enum Status
	{
		OK,
		INVALID_VALUES,
		READ_FAILURE,
		WRITE_FAILURE
	};

	class ValidationError
	{
	private:
		std::string message;
		std::vector<Entry*> invalidEntries;

	public:
		ValidationError(const std::string& message,  std::vector<Entry*> invalidEntries);
		ValidationError(const std::string& message,  Entry* invalidEntry);
		std::string getMessage();
		std::vector<Entry*>& getInvalidEntries();
	};

	class Entry
	{
	friend Namespace;
	private:
		std::string title;
		EntryType type;
		std::string key;
		std::string value;
		std::string defaultValue;
		boolean nullable;
		void validate(std::vector<ValidationError*>& errorList);
		void save(const nvs_handle_t& nvs);
		void load(const nvs_handle_t& nvs);

	public:
		Entry(const std::string& title, EntryType type, const std::string& key, boolean nullable = false, const std::string& defaultValue = "");
		void update(const std::string& value);
		std::string getTitle();
		std::string getValue();
	};

	class Namespace
	{
	friend Configuration;
	private:
		std::string title;
		std::string name;
		std::vector<Entry*> entries;
		std::vector<ValidationError*> validationErrors;
		void (*customValidation)(const std::vector<Entry*>& entries, std::vector<ValidationError*>& errors);

	public:
		Namespace(const std::string& title, const std::string& name, std::vector<Entry*> entries, void (*customValidation)(const std::vector<Entry*>& entries, std::vector<ValidationError*>& errors) = 0);
		void update(const std::string& entryKey, const std::string& value);
		Status save();
		Status load();
		std::string getTitle();
		std::string getName();
		std::vector<Entry*>& getEntries();
		Entry* getEntry(std::string& key);
		Status validate();
	};

	class Configuration
	{
	friend Server;
	friend Cli;
	private:
		std::string title;
		std::string partition;
		std::vector<Namespace*> namespaces;

	public:
		Configuration(const std::string& title, std::vector<Namespace*> namespaces, const std::string& partition = "nvs");
		std::vector<Namespace*>& getNamespaces();
		Namespace* getNamespace(std::string& name);
	};

	class Server
	{
	private:
		Configuration configuration;
		std::string (*styleHandler)();
		int serverPort;
		WebServer webServer;
		void load();
		std::string create_html(std::string& title, std::string& body);
		std::string create_root_html(esp32config::Configuration& config);
		std::string create_namespace_html(esp32config::Configuration& config, esp32config::Namespace& ns);
		std::string create_entry_html(esp32config::Configuration& config, esp32config::Namespace& ns, esp32config::Entry& entry);
		void handle_get_root_request();
		void handle_get_namespace_request();
		void handle_get_entry_request();
		void handle_post_request();
	public:
		Server(const Configuration& configuration, std::string (*styleHandler)() = 0, int port = 80);
		void begin(const std::string& ssid = "ESP32 Config Server", const std::string& password = "esp32secret", IPAddress ip = IPAddress(192,168,1,1));
		void loop();
	};

	class Cli
	{
	private:
		Stream *console;
		Configuration configuration;

	public:
		Cli(const Configuration& configuration);
		void begin(Stream *console);
		void loop();
	};

}

#endif