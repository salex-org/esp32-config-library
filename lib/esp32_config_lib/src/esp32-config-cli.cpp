#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <vector>
#include <algorithm>
#include "nvs_flash.h"
#include "esp32-config-lib.h"


esp32config::Cli::Cli(const Configuration& configuration) :
	configuration(configuration), console(nullptr) {}

void esp32config::Cli::begin(Stream *console)
{
    this->console = console;
    this->console->printf("Config CLI started\n");
}

void esp32config::Cli::loop() {
	this->console->

	if(this->console->available() > 0) {
		std::string	input = "";
		input = this->console->readStringUntil('\n').c_str();
		this->console->printf("\U0001F37A Cheers %s\n", input.c_str());
	}
}

// ===========================================================

/*

void list_nvs_partitions() {
  Serial.println("\n\nNVS data partitions\n==================================");
  esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
  while(pi != NULL) {
    const esp_partition_t *part = esp_partition_get(pi);
    pi = esp_partition_next(pi);
    Serial.printf("label '%s', size '%d'\n", part->label, part->size);
  }
}

void list_namespaces(const char *part_name) {
  Serial.printf("\n\nNamespaces in partition '%s'\n======================================\n", part_name);
  std::vector<String> collector;
  nvs_iterator_t ki = nvs_entry_find(part_name, NULL, NVS_TYPE_ANY);
  while(ki != NULL) {
    nvs_entry_info_t info;
    nvs_entry_info(ki, &info);
    ki = nvs_entry_next(ki);
    collector.push_back(info.namespace_name);
  }
  std::vector<String> namespaces(collector.size());
  std::vector<String>::iterator ci = std::unique_copy(collector.begin(), collector.end(), namespaces.begin());
  namespaces.resize(std::distance(namespaces.begin(), ci));
  for(int i = 0 ; i < namespaces.size(); i++) {
    Serial.printf("name '%s'\n", namespaces[i].c_str());
  }
}

void list_keys(const char *part_name, const char *namespace_name) {
  Serial.printf("\n\nKeys in partition '%s', namespace '%s'\n======================================\n", part_name, namespace_name);
  nvs_iterator_t ki = nvs_entry_find(part_name, namespace_name, NVS_TYPE_ANY);
  while(ki != NULL) {
    nvs_entry_info_t info;
    nvs_entry_info(ki, &info);
    ki = nvs_entry_next(ki);
    Serial.printf("key '%s', type '%d'\n", info.key, info.type);
  }
}

std::string getInput() {
	std::string input = "";
	do {
    	input = Serial.readStringUntil('\n');
	} while(name.length() == 0);
	return input;
}

*/