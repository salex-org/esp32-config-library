#include "esp32-config-lib.hpp"

esp32config::Configuration::Configuration(const std::string& title, std::vector<Namespace> namespaces, const std::string& partition) :
	title(title), namespaces(namespaces), partition(partition) {}

std::vector<esp32config::Namespace> esp32config::Configuration::getNamespaces() {
	return this->namespaces;
}

esp32config::Namespace* esp32config::Configuration::getNamespace(std::string& name) {
	for(Namespace& e : this->namespaces) {
		if(e.name == name) {
			return &e;
		}
	}
	return nullptr;
}