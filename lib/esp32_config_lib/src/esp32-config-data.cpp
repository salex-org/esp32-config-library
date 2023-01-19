#include "esp32-config-lib.hpp"

esp32config::ValidationError::ValidationError(const std::string& message,  std::vector<Entry> invalidEntries) :
	message(message), invalidEntries(invalidEntries) {}

esp32config::ValidationError::ValidationError(const std::string& message,  Entry* invalidEntry) :
	message(message), invalidEntries( { *invalidEntry } ) {}

esp32config::Entry::Entry(const std::string& title, esp32config::EntryType type, const std::string& key, boolean nullable, const std::string& defaultValue) :
	title(title), type(type), key(key), value(""), nullable(nullable), defaultValue(defaultValue) {}

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

void esp32config::Entry::validate(std::vector<esp32config::ValidationError>& errorList) {
	if(this->value.empty()) {
		if(!this->nullable) {
			errorList.push_back(ValidationError(this->title + " must not be empy.", this));
		}
	}
}

std::string esp32config::Entry::getTitle() {
	return this->title;
}
std::string esp32config::Entry::getValue() {
	return this->value;
}

esp32config::Namespace::Namespace(const std::string& title, const std::string& name, std::vector<esp32config::Entry> entries, std::vector<ValidationError> (*customValidation)(std::vector<Entry> entries)) :
	title(title), name(name), entries(entries), validationErrors(std::vector<ValidationError>()), customValidation(customValidation) {}

void esp32config::Namespace::update(const std::string& key, const std::string& value)
{
    for (int i = 0; i < this->entries.size(); i++)
    {
        this->entries[i].update(key, value);
    }
}

esp32config::Status esp32config::Namespace::save()
{
	Status result = this->validate();
	if(result == OK) {
		nvs_handle_t nvs;
		if(nvs_open(this->name.c_str(), NVS_READWRITE, &nvs) == ESP_OK) {
			if(nvs_erase_all(nvs) == ESP_OK) {
				if(nvs_commit(nvs) == ESP_OK) {
					for (esp32config::Entry& entry : this->entries)
					{
						entry.save(nvs);
					}
					if(nvs_commit(nvs) != ESP_OK) {
						result = WRITE_FAILURE;
					}
				} else {
					result = WRITE_FAILURE;
				}
			} else {
				result = WRITE_FAILURE;
			}
			nvs_close(nvs);
		} else {
			result = WRITE_FAILURE;
		}
	}
	return result;
}

esp32config::Status esp32config::Namespace::load()
{
	nvs_handle_t nvs;
	if(esp_err_t result = nvs_open(this->name.c_str(), NVS_READONLY, &nvs) == ESP_OK) {
        for (esp32config::Entry& entry : this->entries)
        {
            entry.load(nvs);
        }
		nvs_close(nvs);
		return OK;
	} else {
		return READ_FAILURE;
	}
}

esp32config::Status esp32config::Namespace::validate()
{
	this->validationErrors.clear();
	if(this->customValidation != 0) {
		for(ValidationError& each : customValidation(this->entries)) {
			this->validationErrors.push_back(each);
		}
	}
	for(Entry& each : this->entries) {
		each.validate(this->validationErrors);
	}
	return this->validationErrors.empty()?OK:INVALID_VALUES;
}

std::string esp32config::Namespace::getTitle()
{
	return this->title;
}

std::string esp32config::Namespace::getName()
{
	return this->name;
}

esp32config::Configuration::Configuration(const std::string& title, std::vector<Namespace> namespaces, const std::string& partition) :
	title(title), namespaces(namespaces), partition(partition) {}

std::vector<esp32config::Namespace> esp32config::Configuration::getNamespaces() {
	return this->namespaces;
}