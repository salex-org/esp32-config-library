#include "esp32-config-lib.hpp"

esp32config::Entry::Entry(const std::string& title, esp32config::EntryType type, const std::string& key, boolean nullable, const std::string& defaultValue) :
	title(title), type(type), key(key), value(""), nullable(nullable), defaultValue(defaultValue) {}

void esp32config::Entry::update(const std::string& value)
{
    this->value = value;
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

void esp32config::Entry::validate(std::vector<esp32config::ValidationError*>& errorList) {
	if(this->value.empty()) {
		if(!this->nullable) {
			errorList.push_back(new ValidationError(this->title + " must not be empy.", this));
		}
	} else {
	    switch (this->type)
		{
		case INTEGER:
			char* rest;
			std::strtol(this->value.c_str(), &rest, 10);
			if(strlen(rest) > 0) {
				errorList.push_back(new ValidationError(this->value + " is not an integer.", this));
			}
			break;
		}
	}
}

std::string esp32config::Entry::getTitle() {
	return this->title;
}
std::string esp32config::Entry::getValue() {
	return this->value;
}
std::string esp32config::Entry::getKey() {
	return this->key;
}
