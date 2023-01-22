#include "esp32-config-lib.hpp"
#include "uri/UriBraces.h"

const std::string DEFAULT_STYLE = ""
"body {\n"
"	color: black;\n"
"	background-color: #2098c4;\n"
"	font-size: 2vw;\n"
"	font-family: sans-serif;\n"
"	font-style: normal;\n"
"	font-weight: normal;\n"
"}\n"
".entry {\n"
"	position: relative;\n"
"	width: 100%;\n"
"	height: 3.5em;\n"
"}\n"
".entry div {\n"
"	position: relative;\n"
"	height: 1.4em;\n"
"	border-style: solid;\n"
"	font-size: 3vw;\n"
"	padding: 0.2em 0.5em;\n"
"	vertical-align: middle;\n"
"	line-height: 1.4em;\n"
"}\n"
".entry .key {\n"
"	background-color: lightgray;\n"
"	float: left;\n"
"	width: calc(20% - 4px - 1em);\n"
"	border-width: 2px;\n"
"	border-radius: 0.7em 0 0 0.7em;\n"
"}\n"
".entry .value {\n"
"	background-color: white;\n"
"	float: right;\n"
"	width: calc(80% - 2px - 1em);\n"
"	border-width: 2px 2px 2px 0px;\n"
"	border-radius: 0 0.7em 0.7em 0;\n"
"}\n"
".entry .value input {\n"
"	border: none;\n"
"	width: calc(100% - 1.2em);\n"
"	height: 1.4em;\n"
"	font-size: 3vw;\n"
"	vertical-align: middle;\n"
"}\n"
".buttons button {\n"
"	font-size: 3vw;\n"
"	height: 2em;\n"
"	background-color: lightgray;\n"
"	border-width: 2px;\n"
"	border-radius: 0.7em;\n"
"	padding: 0.2em 0.5em;\n"
"	vertical-align: middle;\n"
"	margin-right: 0.5em;\n"
"	margin-top: 0.5em;\n"
"}\n"
".buttons button:hover {\n"
"	background-color: #ec9c3e;\n"
"}\n"
".namespaces button {\n"
"	float: left;\n"
"	font-size: 3vw;\n"
"	background-color: lightgray;\n"
"	height: 2em;\n"
"	width: calc(100% - 4px);\n"
"	border-width: 2px;\n"
"	border-radius: 0.7em;\n"
"	padding: 0.2em 0.5em;\n"
"	vertical-align: middle;\n"
"	text-align: left;\n"
"	margin-right: 0.5em;\n"
"	margin-top: 0.5em;\n"
"}\n"
".namespaces button:hover {\n"
"	background-color: #ec9c3e;\n"
"}\n"
".messages {\n"
"	color: black;\n"
"	background-color: lightgray;\n"
"	font-size: 3vw;\n"
"	padding: 0.2em 0.5em;\n"
"	border-width: 2px;\n"
"	border-radius: 0.7em;\n"
"	border-style: solid;\n"
"}\n"
".error {\n"
"	color: red !important;\n"
"	border-color: red !important;\n"
"}\n"
".error input {\n"
""	"color: red !important;\n"
"}\n";

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
	this->webServer.on(UriBraces("/{}"), HTTP_POST, std::bind(&esp32config::Server::handle_post_request, this));
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
	content += "<div class=\"namespaces\">";
    for (Namespace* each : this->configuration.getNamespaces())
    {
		content += "<button onclick=\"window.location.href='/" + each->getName() + "'\">" + each->getTitle() + "</button>";
    }
	content += "</div>";
    return create_html(title, content);
}

std::string esp32config::Server::create_namespace_html(esp32config::Configuration& config, esp32config::Namespace& ns, const std::string& message)
{
	std::string title = config.title + TITLE_SPLITTER + ns.getTitle();
	std::string content = "";
	if(message != "") {
		content += "<div class=\"messages\"><p>" + message + "</p></div>";
	}
	std::vector<ValidationError*>& errors = ns.getValidationErrors();
	if(!errors.empty()) {
		content += "<div class=\"messages error\">";
		for(ValidationError* v : errors) {
			content += "<p>" + v->getMessage() + "</p>";
		}
		content += "</div>";
	}
    content += "<form action=\"/" + ns.getName() + "\" method=\"post\">";
    for (Entry* each : ns.getEntries()) {
		std::string eachKey = each->getKey();
		content += "<div class=\"entry";
		ns.isEntryInvalid(eachKey)?" error":"";
		content += ns.isEntryInvalid(eachKey)?" error":"";
		content += "\">";
		content += "<div class=\"key\">" + each->getTitle() + "</div>";
		content += "<div class=\"value\"><input type=\"text\" name=\"" + ns.getName() + "." + each->getKey() + "\" value=\"" + each->getValue() + "\" /></div>";
		content += "</div>";
    }
    content += "<div class=\"buttons\">";
	content += "<button type=\"button\" onclick=\"window.location.href='/'\">&cross;&nbsp;Cancel</button>";
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
	std::string nsName = this->webServer.pathArg(0).c_str();
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
}
