#include "esp32-config-lib.hpp"

esp32config::Entry::Entry(String title, esp32config::EntryType type, String key, String defaultValue)
{
    this->title = title;
    this->type = type;
    this->key = key;
    this->value = "";
    this->defaultValue = defaultValue;
}

void esp32config::Entry::update(String key, String value)
{
    if (this->key == key)
    {
        this->value = value;
    }
}

void esp32config::Entry::save(Preferences *prefs)
{
    switch (this->type)
    {
    case INTEGER:
        prefs->putInt(this->key.c_str(), this->value.toInt());
        break;
    case TEXT:
    case PASSWORD:
        prefs->putString(this->key.c_str(), this->value.c_str());
        break;
    }
}

void esp32config::Entry::load(Preferences *prefs)
{
    if (prefs->isKey(this->key.c_str()))
    {
        switch (this->type)
        {
        case INTEGER:
            this->value = prefs->getInt(this->key.c_str());
            break;
        case TEXT:
        case PASSWORD:
            this->value = prefs->getString(this->key.c_str());
            break;
        }
    }
    else
    {
        this->value = this->defaultValue;
    }
}

String esp32config::Entry::html(String ns)
{
    String content = "<tr><td>";
    content.concat(this->title);
    content.concat("</td><td><input type=\"");
    content.concat(this->type);
    content.concat("\" name=\"");
    content.concat(ns);
    content.concat(".");
    content.concat(this->key);
    content.concat("\" value=\"");
    content.concat(this->value);
    content.concat("\" /></td></tr>");
    return content;
}

esp32config::Namespace::Namespace(String title, String name, std::vector<esp32config::Entry> entries)
{
    this->title = title;
    this->name = name;
    this->entries = entries;
}

void esp32config::Namespace::update(String key, String value)
{
    for (int i = 0; i < this->entries.size(); i++)
    {
        this->entries[i].update(key, value);
    }
}

void esp32config::Namespace::save()
{
    Preferences prefs;
    if (prefs.begin(this->name.c_str(), false))
    {
        if (prefs.clear())
        {
            for (int i = 0; i < this->entries.size(); i++)
            {
                this->entries[i].save(&prefs);
            }
            prefs.end();
            // TODO: return OK;
        }
        else
        {
            // TODO: return ERROR;
        }
    }
    else
    {
        // TODO: return ERROR;
    }
}

void esp32config::Namespace::load()
{
    Preferences prefs;
    if (prefs.begin(this->name.c_str(), false))
    {
        for (int i = 0; i < this->entries.size(); i++)
        {
            this->entries[i].load(&prefs);
        }
        prefs.end();
    }
}

String esp32config::Namespace::html()
{
    String content = "<h2>";
    content.concat(this->title);
    content.concat("</h2><table border=\"0\">");
    for (int i = 0; i < this->entries.size(); i++)
    {
        content.concat(this->entries[i].html(this->name));
    }
    content.concat("</table>");
    return content;
}

esp32config::Server::Server(std::vector<esp32config::Namespace> namespaces, String (*styleHandler)(), int serverPort)
{
    this->namespaces = namespaces;
    this->styleHandler = styleHandler;
	this->serverPort = serverPort;
}

void esp32config::Server::begin(std::string ssid, std::string password, IPAddress ip) {
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

String esp32config::Server::create_html()
{
    String content = "<html><head><title>Samrt Home Agent</title>";
    if (this->styleHandler != 0)
    {
        content.concat("<style>");
        content.concat(this->styleHandler());
        content.concat("</style>");
    }
    content.concat("</head><body><h1>Configuration</h1>");
    content.concat("<form action=\"/\" method=\"post\">");
    for (int i = 0; i < this->namespaces.size(); i++)
    {
        content.concat(this->namespaces[i].html());
    }
    content.concat("<p><input type=\"submit\" name=\"save\" value=\"Save\" /></p>");
    content.concat("</form></body></html>");
    return content;
}

void esp32config::Server::load()
{
    for (int i = 0; i < this->namespaces.size(); i++)
    {
        this->namespaces[i].load();
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
    String timeserver = this->webServer.arg("general.timeserver").c_str();
    Serial.printf("Updating configuration, new timeserver value is '%s'\n", timeserver);
    this->webServer.send(201, "text/html", this->create_html().c_str());
  }
  else
  {
    Serial.println("ERROR: Received post call without data");
    this->webServer.send(400, "text/html", "missing data");
  }
}
