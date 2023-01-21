#include "esp32-config-lib.hpp"
#include "uri/UriBraces.h"

esp32config::Server::Server(const Configuration& configuration, std::string (*styleHandler)(), int serverPort) :
	configuration(configuration), styleHandler(styleHandler), serverPort(serverPort) {}

void esp32config::Server::begin(const std::string& ssid, const std::string& password, IPAddress ip) {
	// Load preference value
	load();

	// Start WiFi soft access point
	WiFi.softAP(ssid.c_str(), password.c_str());
	WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
	Serial.println("WiFi access point started");
	delay(100); // Wait 0.1 seconds for the wifi to be up and running

	// Start web server
	this->webServer.on("/", HTTP_GET, std::bind(&esp32config::Server::handle_get_root_request, this));
	this->webServer.on(UriBraces("/{}"), HTTP_GET, std::bind(&esp32config::Server::handle_get_namespace_request, this));
	this->webServer.on(UriBraces("/{}/{}"), HTTP_GET, std::bind(&esp32config::Server::handle_get_entry_request, this));
	this->webServer.on("/", HTTP_POST, std::bind(&esp32config::Server::handle_post_request, this));
	this->webServer.begin(serverPort);
}

void esp32config::Server::loop() {
	this->webServer.handleClient();
}

std::string esp32config::Server::create_html(std::string& title, std::string& body) {
    std::string content = "<html><head><title>" + title + "</title>";
    if (this->styleHandler != 0)
    {
        content += "<style>";
        content += this->styleHandler();
        content += "</style>";
    }
    content += "</head><body><h1>";
	content += title;
	content += "</h1>";
	content += "body";
    content += "</body></html>";
    return content;
}

std::string esp32config::Server::create_root_html(esp32config::Configuration& config)
{
	std::string title = config.title;
    std::string content;
    for (Namespace& each : this->configuration.getNamespaces())
    {
        content += "<p><a href=\"" + each.getName() + "\">" + each.getTitle() + "</a></p>";
    }
    return create_html(title, content);
}

std::string esp32config::Server::create_namespace_html(esp32config::Namespace& ns)
{
	std::string title = ns.getTitle();
    std::string content = "<form action=\"/\" method=\"post\">";
    for (Entry& each : ns.getEntries()) {
		content += "<p>";
		content += each.getTitle();
		content += "</p>";
    }
    content += "<p><a href=\"..\" />Back</a><input type=\"submit\" name=\"save\" value=\"Save\" /></p>";
    content += "</form>";
    return create_html(title, content);

}

std::string esp32config::Server::create_entry_html(esp32config::Namespace& ns, esp32config::Entry& entry)  {
	std::string title = ns.getTitle() + "&nbsp" + entry.getTitle();
    std::string content;
	// TODO: Create content for entry value selection
	return create_html(title, content);
}

void esp32config::Server::load()
{
    for (Namespace& each : this->configuration.getNamespaces())
    {
        each.load();
    }
}

void esp32config::Server::handle_get_root_request()
{
	this->webServer.send(200, "text/html", create_root_html(this->configuration).c_str());
}

void esp32config::Server::handle_get_namespace_request()
{
	std::string nsName = this->webServer.pathArg(0).c_str();
	Namespace* ns = this->configuration.getNamespace(nsName);
	if(ns != nullptr) {
		this->webServer.send(200, "text/html", create_namespace_html(*ns).c_str());
	} else {
		this->webServer.send(404, "text/html", ("Namespace " + nsName + " not found").c_str());
	}
}

void esp32config::Server::handle_get_entry_request()
{
	std::string nsName = this->webServer.pathArg(0).c_str();
	Namespace* ns = this->configuration.getNamespace(nsName);
	if(ns != nullptr) {
		std::string entryKey = this->webServer.pathArg(1).c_str();
		Entry* entry = ns->getEntry(entryKey);
		if(entry != nullptr) {
			this->webServer.send(200, "text/html", create_entry_html(*ns, *entry).c_str());
		} else {
			this->webServer.send(404, "text/html", ("Entry " + entryKey + " not found in Namespace " + nsName).c_str());
		}
	} else {
		this->webServer.send(404, "text/html", ("Namespace " + nsName + " not found").c_str());
	}
}

void esp32config::Server::handle_post_request()
{
  if (this->webServer.hasArg("general.timeserver"))
  {
    std::string timeserver = this->webServer.arg("general.timeserver").c_str();
    Serial.printf("Updating configuration, new timeserver value is '%s'\n", timeserver);
    this->webServer.send(201, "text/html", this->create_root_html(this->configuration).c_str());
  }
  else
  {
    Serial.println("ERROR: Received post call without data");
    this->webServer.send(400, "text/html", "missing data");
  }
}
