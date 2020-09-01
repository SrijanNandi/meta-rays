#define _GLIBCXX_USE_C99 1

#include "mqtt_client.h"
#include "commonlib.h"
#include <ctime>
#include <fstream>
#include <jansson.h>
#include <set>

struct mosquitto *mosq = NULL;
struct mosquitto *localClient = NULL;
auto continuousStationUpdates =
    ExecUciCommand("uci get ray.features.continuousStationUpdates");
auto apMacAddress = read_ap_mac();

void mosq_log_callback(struct mosquitto *mosq, void *userdata, int level,
                       const char *str) {
  /* Pring all log messages regardless of level. */

  switch (level) {
  case MOSQ_LOG_WARNING:
  case MOSQ_LOG_ERR: {
    printf("%i:%s\n", level, str);
  }
  }
}

std::string GetString(json_t *data, const char *key) {
  puts("In get string");
  printf("The key is %s", key);
  auto object = json_object_get(data, key);
  switch (json_typeof(object)) {
  case JSON_STRING:
    return std::string(json_string_value(object));
  case JSON_INTEGER:
    return std::to_string(json_integer_value(object));
  case JSON_REAL:
    return std::to_string(json_real_value(object));
  default:
    return "";
  }
}

void mqtt_server_connect_callback(struct mosquitto *mosqArg, void *obj,
                                  int result) {
  // auto apMacAddress = read_ap_mac();
  ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Remote client connected");
  sleep(2);
  ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Reconciling stations");  
  ExecSystemCommand(
      "cd /etc/ray/clientConnection;./clientConnectionHistoryUpdates.lua onetime empty onlyonline &");  

    mosquitto_subscribe(mosq, nullptr, ("command/" + apMacAddress).data(), 2);
    mosquitto_subscribe(mosq, nullptr, ("node/join/ack/" + apMacAddress).data(), 2);
    mosquitto_subscribe(mosq, nullptr, ("frontend/fastUpdates/+/" + apMacAddress).data(), 2);             
   // mosquitto_subscribe(mosq, nullptr, ("mesh/broadcast/" + apMacAddress).data(), 2);
    mosquitto_subscribe(mosq, nullptr, ("node/status/verify/" + apMacAddress).data(), 2);
    
    ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Stopping default ssid");  
    ExecSystemCommand("ifconfig ath99 down");
    ExecSystemCommand("ifconfig ath995 down");
    ExecSystemCommand("/etc/init.d/autossh stop");
    ExecSystemCommand("echo 65535 > /sys/kernel/led_kobject/led_freq");
}

void mqtt_server_disconnect_callback(struct mosquitto *mosqArg, void *obj,
                                  int result) {
ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Remote client disconnected");
   ExecSystemCommand("ifconfig ath99 up");
    ExecSystemCommand("/etc/init.d/autossh start");
    ExecSystemCommand("echo 1000 > /sys/kernel/led_kobject/led_freq");
}

void mqtt_server_message_callback(struct mosquitto *mosqArg, void *obj,
                                   const struct mosquitto_message *message) {
std::string topic(message->topic);
ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Received message on remote client on topic " + topic);
  if (topic == "node/join/ack") {
    // do nothing
    ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Received Node join ack");
  }else if (topic == "node/status/verify") { 
    ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Node status verify call");
    mosquitto_publish(localClient, nullptr, "node/status/verify/ack", message->payloadlen,
                      message->payload, 2, 0);    
  } else {
    puts("Command Received on topic");
    ExecSystemCommand("logger -t ray-mqttServer-downlink-" + apMacAddress + "  -p " + "info" + " " + "Forwarding message to local client on topic " + topic);
    mosquitto_publish(localClient, nullptr, topic.c_str(), message->payloadlen,
                      message->payload, 2, 0);                                     
  }
                                   }

int mqtt_send(const char *topic, const char *msg) {
  int snd = -1;
  printf("topic on publish: %s\n", topic);
  printf("msg to publish: %s\n", msg);
  snd = mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, 2, 0);
  if (snd != 0) {
    printf("mqtt_send error=%i\n", snd);
    return 1;
  }
  return 0;
}

int mqtt_setup_receiver() {
  bool clean_session = true;

  mosquitto_lib_init();
  auto apMacAddress = read_ap_mac();
  mosq = mosquitto_new((apMacAddress).data(), clean_session, NULL);
  if (!mosq) {
    printf("Error: Out of memory.\n");
    return 1;
  }

  localClient = mosquitto_new(NULL, clean_session, NULL);
  if (!localClient) {
    printf("Error: Out of memory.\n");
    return 1;
  }

  // Fetch via uci library
auto mqttCaCertPath = ExecUciCommand("uci get ray.mqtt.mqttCaCertPath");      
auto mqttCertificatesDirectory = ExecUciCommand("uci get ray.mqtt.mqttCertificatesDirectory");
auto mqttClientKeyPath = ExecUciCommand("uci get ray.mqtt.mqttClientKeyPath");
auto mqttClientCertPath = ExecUciCommand("uci get ray.mqtt.mqttClientCertPath"); 
auto certificatesWritten = ExecUciCommand("uci get ray.mqtt.certificatesWritten");
auto isMqttInsecure = ExecUciCommand("uci get ray.mqtt.mqttInSecure");

  if (!certificatesWritten.empty() && certificatesWritten.compare("1") == 0) {
    mosquitto_tls_set(mosq,mqttCaCertPath.data(),mqttCertificatesDirectory.data(),mqttClientCertPath.data(),mqttClientKeyPath.data(),NULL);
    if (!isMqttInsecure.empty() && (isMqttInsecure.compare("true") == 0)) {
        mosquitto_tls_insecure_set(mosq,true);
    } else {
        mosquitto_tls_insecure_set(mosq,false);
    }
  } 


  mosquitto_log_callback_set(mosq, mosq_log_callback);
  /* Set the connect callback */
  mosquitto_connect_callback_set(mosq, mqtt_server_connect_callback);
  mosquitto_disconnect_callback_set(mosq, mqtt_server_disconnect_callback);
  /* Set the message callback */
  mosquitto_message_callback_set(mosq, mqtt_server_message_callback);

  // Fetch via uci library
  auto mqttPrimaryBroker = ExecUciCommand("uci get ray.ray.primaryBroker");

  // Fetch via uci library
  int mqttKeepAliveTime =
      std::stoi(ExecUciCommand("uci get ray.mqtt.mqttKeepAliveTime"));

  // Fetch via uci library
  int mqttPort = std::stoi(ExecUciCommand("uci get ray.mqtt.mqttPort"));

  /* Connect to an MQTT broker */
  if (mosquitto_connect(mosq, mqttPrimaryBroker.data(), mqttPort,
                        mqttKeepAliveTime)) {
    printf("Unable to connect.\n");
    return 1;
  }

  /* Connect to a Local MQTT broker */
  if (mosquitto_connect(localClient, MQTT_HOST, MQTT_PORT, KEEP_ALIVE)) {
    printf("Unable to connect.\n");
    return 1;
  }  
  
  /* Call this once to start a new thread to process network traffic */
  int loop = mosquitto_loop_start(mosq); 
  if (loop != MOSQ_ERR_SUCCESS) {
    printf("Unable to start loop: %i\n", loop);
    return 1;
  }

  sleep(5);

  int localLoop = mosquitto_loop_forever(localClient,-1,1);
  if (localLoop != MOSQ_ERR_SUCCESS) {
    printf("Unable to start loop: %i\n", localLoop);
    return 1;
  }

  return 0;
}

int mqtt_close() {
  /*  Call this once to stop the network thread previously
   *  created with mosquitto_loop_start */
  int loop = mosquitto_loop_stop(mosq, true);
  if (loop != MOSQ_ERR_SUCCESS) {
    printf("Unable to start loop: %i\n", loop);
    return 1;
  }

  loop = mosquitto_loop_stop(localClient, true);
  if (loop != MOSQ_ERR_SUCCESS) {
    printf("Unable to start loop: %i\n", loop);
    return 1;
  }

  mosquitto_destroy(mosq);
  mosquitto_destroy(localClient);

  mosquitto_lib_cleanup();

  return 0;
}
