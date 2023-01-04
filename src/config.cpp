#include "config.h"

ConfigEntry::ConfigEntry(String title, ConfigEntryType type, String key) {
    this->title = title;
    this->type = type;
    this->key = key;
    this->value = "";
    this->defaultValue = "";
}

ConfigEntry::ConfigEntry(String title, ConfigEntryType type, String key, String defaultValue) {
    this->title = title;
    this->type = type;
    this->key = key;
    this->value = "";
    this->defaultValue = defaultValue;
}

void ConfigEntry::update(String key, String value) {
    if(this->key == key) {
        this->value = value;
    }
}

void ConfigEntry::save(Preferences *prefs) {
    switch(this->type) {
        case INTEGER:
            prefs->putInt(this->key.c_str(), this->value.toInt());
            break;
        case TEXT:
        case PASSWORD:
            prefs->putString(this->key.c_str(), this->value.c_str());
            break;
    }
}

void ConfigEntry::load(Preferences *prefs) {
    if(prefs->isKey(this->key.c_str())) {
        switch(this->type) {
            case INTEGER:
                this->value = prefs->getInt(this->key.c_str());
                break;
            case TEXT:
            case PASSWORD:
                this->value = prefs->getString(this->key.c_str());
                break;
        }    
    } else {
        this->value = this->defaultValue;
    }
}

String ConfigEntry::html(String ns) {
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


ConfigNamespace::ConfigNamespace(String title, String name, std::vector<ConfigEntry> entries) {
    this->title = title;
    this->name = name;
    this->entries = entries;
}

void ConfigNamespace::update(String key, String value) {
    for(int i = 0 ; i < this->entries.size() ; i++) {
        this->entries[i].update(key, value);
    }
}

void ConfigNamespace::save() {
  Preferences prefs;
  if(prefs.begin(this->name.c_str(), false)) {
    if(prefs.clear()) {
      for(int i = 0 ; i < this->entries.size() ; i++) {
        this->entries[i].save(&prefs);
      }
      prefs.end();
      // TODO: return OK;
    } else {
      // TODO: return ERROR;
    }
  } else {
    // TODO: return ERROR;
  }
}

void ConfigNamespace::load() {
    Preferences prefs;
    if(prefs.begin(this->name.c_str(), false)) {
        for(int i = 0 ; i < this->entries.size() ; i++) {
            this->entries[i].load(&prefs);
        }
        prefs.end();
    } 
}

String ConfigNamespace::html() {
    String content = "<h2>";
    content.concat(this->title);
    content.concat("</h2><table border=\"0\">");
    for(int i = 0 ; i < this->entries.size() ; i++) {
        content.concat(this->entries[i].html(this->name));
    }
    content.concat("</table>");
    return content;
}

ConfigServer::ConfigServer(std::vector<ConfigNamespace> namespaces) {
    this->namespaces = namespaces;
}

String ConfigServer::html() {
  String content = "<html><head><title>Samrt Home Agent</title></head><body><h1>Configuration</h1>";
  content.concat("<form action=\"/\" method=\"post\">");
  for(int i = 0 ; i < this->namespaces.size() ; i++) {
    content.concat(this->namespaces[i].html());
  }
  content.concat("<p><input type=\"submit\" name=\"save\" value=\"Save\" /></p>");
  content.concat("</form></body></html>");
  return content;
}

void ConfigServer::load() {
    for(int i = 0 ; i < this->namespaces.size() ; i++) {
        this->namespaces[i].load();
    }
}