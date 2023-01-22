#include "esp32-config-lib.hpp"
#include "uri/UriBraces.h"

const std::string DEFAULT_STYLE = ""
"body {"
"   color: black;"
"   background-color: #2098c4;"
"	font-size: 2vw;"
"   font-family: sans-serif;"
"   font-style: normal;"
"   font-weight: normal;"
"}"
".entry {"
"	position: relative;"
"	width: 100%;"
"	height: 3.5em;"
"}"
".entry div {"
"	position: relative;"
"	height: 1.4em;"
"	border-style: solid;"
"	font-size: 3vw;"
"	padding: 0.2em 0.5em;"
"	vertical-align: middle;"
"	line-height: 1.4em;"
"}"
".entry .key {"
"	background-color: lightgray;"
"	float: left;"
"	width: calc(20% - 4px - 1em);"
"	border-width: 2px;"
"	border-radius: 0.7em 0 0 0.7em;"
"}"
".entry .value {"
"	background-color: white;"
"	float: right;"
"	width: calc(80% - 2px - 1em);"
"	border-width: 2px 2px 2px 0px;"
"	border-radius: 0 0.7em 0.7em 0;"
"}"
".entry .value input {"
"	border: none;"
"	width: calc(100% - 1.2em);"
"	height: 1.4em;"
"	font-size: 3vw;"
"	vertical-align: middle;"
"}"
".buttons button {"
"	font-size: 3vw;"
"	height: 2em;"
"	background-color: lightgray;"
"	border-width: 2px;"
"	border-radius: 0.7em;"
"	padding: 0.2em 0.5em;"
"	vertical-align: middle;"
"	margin-right: 0.5em;"
"	margin-top: 0.5em;"
"}"
".buttons button:hover {"
"	background-color: #ec9c3e;"
"}"
".namespaces button {"
"	float: left;"
"	font-size: 3vw;"
"	background-color: lightgray;"
"	height: 2em;"
"	width: calc(100% - 4px);"
"	border-width: 2px;"
"	border-radius: 0.7em;"
"	padding: 0.2em 0.5em;"
"	vertical-align: middle;"
"	text-align: left;"
"	margin-right: 0.5em;"
"	margin-top: 0.5em;"
"}"
".namespaces button:hover {"
"	background-color: #ec9c3e;"
"}"
".messages {"
"	color: black;"
"	background-color: lightgray;"
"	font-size: 3vw;"
"	padding: 0.2em 0.5em;"
"	border-width: 2px;"
"	border-radius: 0.7em;"
"	border-style: solid;"
"}"
".error {"
"	color: red !important;"
"	border-color: red !important;"
"}"
".error input {"
"	color: red !important;"
"}";


const std::string TITLE_SPLITTER = "&nbsp;&raquo;&nbsp;";

esp32config::Server::Server(const Configuration& configuration, const std::string& style, int serverPort) :
	configuration(configuration), serverPort(serverPort) {
	if(style == "") {
		this->style = DEFAULT_STYLE;
	} else {
		this->style;
	}
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
	this->webServer.on("/style.css", HTTP_GET, std::bind(&esp32config::Server::handle_get_style_request, this));
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
	content += "<link rel=\"stylesheet\" href=\"/style.css\">";
    content += "</head><body><h1>";
	content += title;
	content += "</h1>";
	content += body;
    content += "</body></html>";
    return content;
}

std::string esp32config::Server::create_root_html(esp32config::Configuration& config)
{
	std::string title = config.title;
    std::string content;
	content += "<div class=\"namespace\">";
    for (Namespace* each : this->configuration.getNamespaces())
    {
		content += "<button onclick=\"window.location.href='/" + each->getName() + "'\">" + each->getTitle() + "</button>";
    }
	content += "</div>";
    return create_html(title, content);
}

std::string esp32config::Server::create_namespace_html(esp32config::Configuration& config, esp32config::Namespace& ns)
{
	std::string title = config.title + TITLE_SPLITTER + ns.getTitle();
    std::string content = "<form action=\"/\" method=\"post\">";
	content += "<input type=\nhidden\n id=\"namespacce\" value=\"" + ns.getName() + "\" />";
    for (Entry* each : ns.getEntries()) {
		content += "<div class=\"line entry\">";
		content += "<div class=\"linecomponent key\">" + each->getTitle() + "</div>";
		content += "<div class=\"linecomponent value\"><input type=\"text\" id=\"" + ns.getName() + "." + each->getKey() + "\" value=\"" + each->getValue() + "\" /></div>";
		content += "</div>";
    }
    content += "<div class=\"buttons\">";
	content += "<button onclick=\"window.location.href='/'\">&cross;&nbsp;Cancel</button>";
	content += "<button type=\"send\">&check; &nbsp;Save</button>";
	content += "</div>";
    content += "</form>";
    return create_html(title, content);

}

std::string esp32config::Server::create_entry_html(esp32config::Configuration& config, esp32config::Namespace& ns, esp32config::Entry& entry)  {
	std::string title = config.title + TITLE_SPLITTER + ns.getTitle() + TITLE_SPLITTER + entry.getTitle();
    std::string content;
	// TODO: Create content for entry value selection
	return create_html(title, content);
}

void esp32config::Server::load()
{
    for (Namespace* each : this->configuration.getNamespaces())
    {
        each->load();
    }
}

void esp32config::Server::handle_get_style_request() {
	this->webServer.send(200, "text/html", this->style.c_str());
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
		this->webServer.send(200, "text/html", create_namespace_html(this->configuration, *ns).c_str());
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
			this->webServer.send(200, "text/html", create_entry_html(this->configuration, *ns, *entry).c_str());
		} else {
			this->webServer.send(404, "text/html", ("Entry " + entryKey + " not found in Namespace " + nsName).c_str());
		}
	} else {
		this->webServer.send(404, "text/html", ("Namespace " + nsName + " not found").c_str());
	}
}

void esp32config::Server::handle_post_request()
{
  if (this->webServer.hasArg("namespace")) {
	std::string nsName = this->webServer.arg("namespace").c_str();
	Namespace* ns = this->configuration.getNamespace(nsName);
	if(ns != nullptr) {
		for(Entry* e : ns->getEntries()) {
			std::string argName = nsName + "." + e->getKey();
			if(this->webServer.hasArg(argName.c_str())) {
				std::string argValue = this->webServer.arg(argName.c_str()).c_str();
				e->update(argValue);
			}
		}
		if(ns->validate() == OK) {
			ns->save();
			this->webServer.send(200, "text/html", this->create_root_html(this->configuration).c_str());
			this->webServer.sendHeader("Location", "/", true);  
			this->webServer.send(303, "text/plain", "");
		} else {
			this->webServer.send(400, "text/html", this->create_namespace_html(this->configuration, *ns).c_str());
		}
		std::string timeserver = this->webServer.arg("namespace").c_str();

	} else {
		this->webServer.send(404, "text/html", ("Namespace " + nsName + " not found").c_str());
	}
  } else {
    this->webServer.send(400, "text/html", "missing data");
  }
}
