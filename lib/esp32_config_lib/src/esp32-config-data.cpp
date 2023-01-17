#include "esp32-config-lib.hpp"


esp32config::Entry::Entry(const std::string& title, esp32config::EntryType type, const std::string& key, const std::string& defaultValue) :
	title(title), type(type), key(key), value(""), defaultValue(defaultValue) {}

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

std::string esp32config::Entry::getTitle() {
	return this->title;
}
std::string esp32config::Entry::getValue() {
	return this->value;
}

esp32config::Namespace::Namespace(const std::string& title, const std::string& name, std::vector<esp32config::Entry> entries) :
	title(title), name(name), entries(entries) {}

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

std::string esp32config::Namespace::getTitle() {
	return this->title;
}

std::string esp32config::Namespace::getName() {
	return this->name;
}

esp32config::Configuration::Configuration(const std::string& title, std::vector<Namespace> namespaces, const std::string& partition) :
	title(title), namespaces(namespaces), partition(partition) {}

std::vector<esp32config::Namespace> esp32config::Configuration::getNamespaces() {
	return this->namespaces;
}