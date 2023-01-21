#include "esp32-config-lib.hpp"

esp32config::Namespace::Namespace(const std::string& title, const std::string& name, std::vector<esp32config::Entry*> entries, void (*customValidation)(const std::vector<Entry*>& entries, std::vector<ValidationError*>& errors)) :
	title(title), name(name), entries(entries), validationErrors(std::vector<ValidationError*>()), customValidation(customValidation) {}

void esp32config::Namespace::update(const std::string& entryKey, const std::string& value)
{
	for(Entry* e : this->entries) {
		if(e->key == entryKey) {
			e->update(value);
		}
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
					for (Entry* e : this->entries)
					{
						e->save(nvs);
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
        for (Entry* e : this->entries)
        {
            e->load(nvs);
        }
		nvs_close(nvs);
		return OK;
	} else {
		return READ_FAILURE;
	}
}

esp32config::Status esp32config::Namespace::validate()
{
	for(ValidationError* v : this->validationErrors) {
		delete v;
	}
	this->validationErrors.clear();
	if(this->customValidation != 0) {
		customValidation(this->entries, this->validationErrors);
	}
	for(Entry* e : this->entries) {
		e->validate(this->validationErrors);
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

std::vector<esp32config::Entry*>& esp32config::Namespace::getEntries() {
	return this->entries;
}

esp32config::Entry* esp32config::Namespace::getEntry(std::string& key) {
	for(Entry* e : this->entries) {
		if(e->key == key) {
			return e;
		}
	}
	return nullptr;
}