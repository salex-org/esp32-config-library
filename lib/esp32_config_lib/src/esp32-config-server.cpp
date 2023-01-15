#include "esp32-config-lib.hpp"

esp32config::Entry::Entry(const std::string& title, esp32config::EntryType type, const std::string& key, const std::string& defaultValue)
{
    this->title = title;
    this->type = type;
    this->key = key;
    this->value = "";
    this->defaultValue = defaultValue;
}

void esp32config::Entry::update(const std::string& key, const std::string& value)
{
    if (this->key == key)
    {
        this->value = value;
    }
}

void esp32config::Entry::load(const nvs_handle_t& nvs)
{
	size_t required_size;
	if(nvs_get_str(nvs, this->key.c_str(), NULL, &required_size) == ESP_OK) {
		char* buffer = new char[required_size];
		if(nvs_get_str(nvs, this->key.c_str(), buffer, &required_size) == ESP_OK) {
			this->value = std::string(buffer);
		}
		delete[] buffer;
	}
}

void esp32config::Entry::save(const nvs_handle_t& nvs)
{
    switch (this->type)
    {
    case INTEGER:
		nvs_set_i32(nvs, this->key.c_str(), std::strtol(this->value.c_str(), NULL, 10));
        break;
    case TEXT:
    case PASSWORD:
		nvs_set_str(nvs, this->key.c_str(), this->value.c_str());
        break;
    }
}

std::string esp32config::Entry::html()
{
	std::string content = "<p>";
	content += this->title;
	content += "</p>";
    return content;
}

esp32config::Namespace::Namespace(const std::string& title, const std::string& name, std::vector<esp32config::Entry> entries)
{
    this->title = title;
    this->name = name;
    this->entries = entries;
}

void esp32config::Namespace::update(const std::string& key, const std::string& value)
{
    for (int i = 0; i < this->entries.size(); i++)
    {
        this->entries[i].update(key, value);
    }
}

void esp32config::Namespace::save()
{
	nvs_handle_t nvs;
	if(nvs_open(this->name.c_str(), NVS_READWRITE, &nvs) == ESP_OK) {
		if(nvs_erase_all(nvs) == ESP_OK) {
			if(nvs_commit(nvs) == ESP_OK) {
				for (esp32config::Entry& entry : this->entries)
				{
					entry.save(nvs);
				}
				nvs_commit(nvs);
			}
		}
		nvs_close(nvs);
	}
}

void esp32config::Namespace::load()
{
	nvs_handle_t nvs;
	if(esp_err_t result = nvs_open(this->name.c_str(), NVS_READONLY, &nvs) == ESP_OK) {
        for (esp32config::Entry& entry : this->entries)
        {
            entry.load(nvs);
        }
		nvs_close(nvs);
	} 
}

std::string esp32config::Namespace::html()
{
	char* content = new char[65536];
    const char* template1 = "<h2>%s<h2>";
	std::snprintf(content, sizeof(content), template1, this->title);
/*	
    for (int i = 0; i < this->entries.size(); i++)
    {
        content.concat(this->entries[i].html(this->name));
    }
    content.concat("</table>"); */
    return content;
}

esp32config::Server::Server(std::vector<esp32config::Namespace> namespaces, std::string (*styleHandler)(), int serverPort)
{
    this->namespaces = namespaces;
    this->styleHandler = styleHandler;
	this->serverPort = serverPort;
}

void esp32config::Server::begin(const std::string& ssid, const std::string& password, IPAddress ip) {
	// Load preference value
	load();

	// Start WiFi soft access point
	WiFi.softAP(ssid.c_str(), password.c_str());
	WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
	Serial.println("WiFi access point started");
	delay(100); // Wait 0.1 seconds for the wifi to be up and running

	// Start web server
	this->webServer.on("/", HTTP_GET, std::bind(&esp32config::Server::handle_get_request, this));
	this->webServer.on("/", HTTP_POST, std::bind(&esp32config::Server::handle_post_request, this));
	this->webServer.begin(serverPort);
}

void esp32config::Server::loop() {
	this->webServer.handleClient();
}

std::string esp32config::Server::create_html()
{
    std::string content = "<html><head><title>Samrt Home Agent</title>";
    if (this->styleHandler != 0)
    {
        content += "<style>";
        content += this->styleHandler();
        content += "</style>";
    }
    content += "</head><body><h1>Configuration</h1>";
    content += "<form action=\"/\" method=\"post\">";
    for (esp32config::Namespace& ns : this->namespaces)
    {
        content += ns.html();
    }
    content += "<p><input type=\"submit\" name=\"save\" value=\"Save\" /></p>";
    content += "</form></body></html>";
    return content;
}

void esp32config::Server::load()
{
    for (esp32config::Namespace& ns : this->namespaces)
    {
        ns.load();
    }
}

void esp32config::Server::handle_get_request()
{
	this->webServer.send(200, "text/html", create_html().c_str());
}

void esp32config::Server::handle_post_request()
{
  if (this->webServer.hasArg("general.timeserver"))
  {
    std::string timeserver = this->webServer.arg("general.timeserver").c_str();
    Serial.printf("Updating configuration, new timeserver value is '%s'\n", timeserver);
    this->webServer.send(201, "text/html", this->create_html().c_str());
  }
  else
  {
    Serial.println("ERROR: Received post call without data");
    this->webServer.send(400, "text/html", "missing data");
  }
}
