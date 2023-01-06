#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <vector>
#include <algorithm>
#include "nvs_flash.h"
#include "config.h"

/*
void writeWifiConfig() {
  Preferences prefs;
  if(prefs.begin("wifi", false)) {
    Serial.println("\nOpened namespace 'wifi' for writing");
    if(prefs.clear()) {
      Serial.println("Namespace 'wifi' cleaned");
      prefs.putString("ssid", "");
      prefs.putString("password", "");
      prefs.end();
      Serial.println("Closed namespace 'wifi'");
    } else {
      Serial.println("ERROR cleaning namespace 'wifi'");
    }
  } else {
    Serial.println("ERROR opening namespace 'wifi'");
  }
}

void writeNtpConfig() {
  Preferences prefs;
  if(prefs.begin("ntp", false)) {
    Serial.println("\nOpened namespace 'ntp' for writing");
    if(prefs.clear()) {
      Serial.println("Namespace 'ntp' cleaned");
      prefs.putString("hostname", "");
      prefs.end();
      Serial.println("Closed namespace 'ntp'");
    } else {
      Serial.println("ERROR cleaning namespace 'ntp'");
    }
  } else {
    Serial.println("ERROR opening namespace 'ntp'");
  }
}

void writeMqttConfig() {
  Preferences prefs;
  if(prefs.begin("mqtt", false)) {
    Serial.println("\nOpened namespace 'mqtt' for writing");
    if(prefs.clear()) {
      Serial.println("Namespace 'mqtt' cleaned");
      prefs.putString("hostname", "");
      prefs.putString("username", "");
      prefs.putString("password", "");
      prefs.end();
      Serial.println("Closed namespace 'mqtt'");
    } else {
      Serial.println("ERROR cleaning namespace 'mqtt'");
    }
  } else {
    Serial.println("ERROR opening namespace 'mqtt'");
  }
}

void scanForWiFi() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get a wifi connection");
    while (true);
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tChannel: ");
    Serial.print(WiFi.channel(thisNet));
    Serial.print("\n");
  }
}

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
*/
/*
void setup() {
  Serial.begin(115200);

  list_nvs_partitions();

  list_namespaces(NVS_DEFAULT_PART_NAME);

  list_keys(NVS_DEFAULT_PART_NAME, "wifi");

}
*/
/*
void setup() {
  Serial.begin(115200);
  Serial.println("\n\nStart writing configuration");
  writeWifiConfig();
  writeNtpConfig();
  writeMqttConfig();
  Serial.print("\nWho are you? ");
  String name = "";
  do {
    name = Serial.readStringUntil('\n');
  } while(name.length() == 0);
  Serial.printf("\n\nHello %s, brewing the configuration completed \U0001F37A\n", name);
}
*/

WebServer webServer(80);

String configStyle() {
	return "body { background-color: cyan; }";
}

ConfigServer configServer({
    ConfigNamespace("General", "general", {
      ConfigEntry("NTP Hostname", TEXT, "ntp-host", "pool.ntp.org"),
      ConfigEntry("Display Timeout (ms)", INTEGER, "display-timeout", "60000")
    }),
    ConfigNamespace("WiFi", "wifi", {
      ConfigEntry("SSID", TEXT, "ssid"),
      ConfigEntry("Password", PASSWORD, "password")
    }),
    ConfigNamespace("MQTT", "mqtt", {
      ConfigEntry("Hostname", TEXT, "hostname"),
      ConfigEntry("Username", TEXT, "username"),
      ConfigEntry("Password", PASSWORD, "password")
    })
  }, configStyle);

void handle_config_page_request() {
  webServer.send(200, "text/html", configServer.html().c_str());
}

void handle_config_update_request() {
  if(webServer.hasArg("general.timeserver")) {
    String timeserver = webServer.arg("general.timeserver").c_str();
    Serial.printf("Updating configuration, new timeserver value is '%s'\n", timeserver);
    webServer.send(201, "text/html", configServer.html().c_str());
  } else {
    Serial.println("ERROR: Received post call without data");
    webServer.send(400, "text/html", "missing data");
  }
}

ConfigCli CLI;

void setup() {
  Serial.begin(115200);
  Serial.println();
  CLI.begin(&Serial);

  configServer.load();

  WiFi.softAP("salex.org Smart Home Agent", "chicago2011");
  WiFi.softAPConfig(IPAddress(192,168,10,1),IPAddress(192,168,10,1), IPAddress(255,255,255,0));
  Serial.println("WiFi access point started");
  delay(100); // Wait 0.1 seconds for the wifi to be up and running

  webServer.on("/", HTTP_GET, handle_config_page_request);
  webServer.on("/", HTTP_POST, handle_config_update_request);
  webServer.begin(); 
  Serial.println("Web server started");

  Serial.println("Connect to WiFi 'salex.org Smart Home Agent' and open 'http://192.168.10.1' to access the configuration");
}

void loop() {
  webServer.handleClient();
}

