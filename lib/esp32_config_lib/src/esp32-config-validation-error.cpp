#include "esp32-config-lib.hpp"

esp32config::ValidationError::ValidationError(const std::string& message,  std::vector<esp32config::Entry>& invalidEntries) :
	message(message), invalidEntries(invalidEntries) {}

esp32config::ValidationError::ValidationError(const std::string& message,  esp32config::Entry* invalidEntry) :
	message(message), invalidEntries( { *invalidEntry } ) {}

std::string esp32config::ValidationError::getMessage() {
	return this->message;
}

std::vector<esp32config::Entry> esp32config::ValidationError::getInvalidEntries() {
	return this->invalidEntries;
}