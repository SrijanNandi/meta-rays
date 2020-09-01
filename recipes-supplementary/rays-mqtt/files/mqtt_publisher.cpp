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
  ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "info" + " " + "Remote client connected");
  sleep(2);
  mosquitto_subscribe(mosq, nullptr, ("node/join/ack/" + apMacAddress).data(),
                      2);
  ExecSystemCommand(
      "cd /etc/ray/clientConnection;./clientConnectionHistoryUpdates.lua onetime empty onlyonline &");  
       ExecSystemCommand("ifconfig ath99 down");
    ExecSystemCommand("ifconfig ath995 down");
    ExecSystemCommand("echo 65535 > /sys/kernel/led_kobject/led_freq");
}

void mqtt_server_disconnect_callback(struct mosquitto *mosqArg, void *obj,
                                  int result) {
ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "info" + " " + "Remote client disconnected");
   ExecSystemCommand("ifconfig ath99 up");
    ExecSystemCommand("echo 1000 > /sys/kernel/led_kobject/led_freq");
}

void local_client_connect_callback(struct mosquitto *mosqArg, void *obj,
                                   int result) {
  printf("local client connect callback, rc=%d\n", result);
   ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "info" + " " + "Local client connected");
  // mqtt_server_publisher subscriptions
  mosquitto_subscribe(localClient, nullptr, "command/ack", 2);
  mosquitto_subscribe(localClient, nullptr, "command/ack/finished", 2);
  mosquitto_subscribe(localClient, nullptr, "node/join", 2);
  mosquitto_subscribe(localClient, nullptr, "node/ssid/add", 2);
  mosquitto_subscribe(localClient, nullptr, "node/ssid/remove", 2);
  mosquitto_subscribe(localClient, nullptr, "node/bssiddetail/register", 2);
  mosquitto_subscribe(localClient, nullptr, "clientConnect", 2);
  mosquitto_subscribe(localClient, nullptr, "node/status/verify/ack", 2);
  mosquitto_subscribe(localClient, nullptr, "stations/connection/event", 2);

  if (continuousStationUpdates == "1") {
    mosquitto_subscribe(localClient, nullptr, "stations/add/wifi", 2);
    mosquitto_subscribe(localClient, nullptr, "stations/delete/wifi", 2);
    mosquitto_subscribe(localClient, nullptr, "stations/add/dhcp/request", 2);
    mosquitto_subscribe(localClient, nullptr, "stations/add/dhcp/ack", 2);
    mosquitto_subscribe(localClient, nullptr, "stations/delete/dhcp", 2);
    mosquitto_subscribe(localClient, nullptr, "stations/stats/interim", 2);
  } else
    puts("No continuousStationUpdates to be configured");

  mosquitto_subscribe(localClient, nullptr, "stations/stats/history", 2);
  mosquitto_subscribe(localClient, nullptr, "stations/stats/reconcile", 2);
  mosquitto_subscribe(localClient, nullptr, "stations/continuousStationUpdates",
                      2);

  mosquitto_subscribe(localClient, nullptr, "speedtest", 2);
  mosquitto_subscribe(localClient, nullptr, "node/interfaceDetail", 2);
  mosquitto_subscribe(localClient, nullptr, "radius/session/ack", 2);
  mosquitto_subscribe(localClient, nullptr, "node/stats/health", 2);
  mosquitto_subscribe(localClient, nullptr, "internet/#", 2);
  mosquitto_subscribe(localClient, nullptr, "websockets/#", 2);
  mosquitto_subscribe(localClient, nullptr, "visitedsites/dns", 2);
  mosquitto_subscribe(localClient, nullptr, "mesh/incoming", 2);
  mosquitto_subscribe(localClient, nullptr, "node/upgrade", 2);
  mosquitto_subscribe(localClient, nullptr, "frontend/fastUpdates/startPublish/+", 2);    
}

void local_client_message_callback(struct mosquitto *mosqArg, void *obj,
                                   const struct mosquitto_message *message) {
  // mqtt_server_publisher logic  
  std::string topic(message->topic);
  ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "info" + " " + "Local client Received message on topic " + topic);  
  if (topic == "stations/continuousStationUpdates") {
    auto payloadData =
      json_loads(static_cast<char *>(message->payload), 0, nullptr);
      if(payloadData ==  NULL) {
          return;
        }  
    continuousStationUpdates = json_string_value(
        json_object_get(payloadData, "continuousStationUpdates"));

    if (continuousStationUpdates == "0") {
      ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "debug" + " " + "Stopping continuous updates");
      mosquitto_unsubscribe(localClient, nullptr, "stations/add/wifi");
      mosquitto_unsubscribe(localClient, nullptr, "stations/delete/wifi");
      mosquitto_unsubscribe(localClient, nullptr, "stations/add/dhcp/request");
      mosquitto_unsubscribe(localClient, nullptr, "stations/add/dhcp/ack");
      mosquitto_unsubscribe(localClient, nullptr, "stations/delete/dhcp");
      mosquitto_unsubscribe(localClient, nullptr, "stations/stats/interim");
    } else {
      ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "debug" + " " + "Starting continuous updates");
      mosquitto_subscribe(localClient, nullptr, "stations/add/wifi", 2);
      mosquitto_subscribe(localClient, nullptr, "stations/delete/wifi", 2);
      mosquitto_subscribe(localClient, nullptr, "stations/add/dhcp/request", 2);
      mosquitto_subscribe(localClient, nullptr, "stations/add/dhcp/ack", 2);
      mosquitto_subscribe(localClient, nullptr, "stations/delete/dhcp", 2);
      mosquitto_subscribe(localClient, nullptr, "stations/stats/interim", 2);
    }
    json_decref(payloadData);
  } else if (topic.find("frontend/fastUpdates/startPublish") != std::string::npos) {
      if(continuousStationUpdates != "1") {
            ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "debug" + " " + "Sending all stations");
            ExecSystemCommand("cd /etc/ray/clientConnection;./clientConnectionHistoryUpdates.lua onetime &");
      //        mosquitto_subscribe(localClient, nullptr, "stations/add/wifi", 2);
      // mosquitto_subscribe(localClient, nullptr, "stations/delete/wifi", 2);
      // mosquitto_subscribe(localClient, nullptr, "stations/add/dhcp/request", 2);
      // mosquitto_subscribe(localClient, nullptr, "stations/add/dhcp/ack", 2);
      // mosquitto_subscribe(localClient, nullptr, "stations/delete/dhcp", 2);
      // mosquitto_subscribe(localClient, nullptr, "stations/stats/interim", 2);
      } else {
        ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "debug" + " " + "Sending only online stations");
        //ExecSystemCommand("cd /etc/ray/clientConnection;./clientConnectionHistoryUpdates.lua onetime empty onlyonline &");
      }
  } else if (topic.find("frontend/fastUpdates/stopPublish") != std::string::npos) {
            if(continuousStationUpdates != "1") {
                 mosquitto_unsubscribe(localClient, nullptr, "stations/add/wifi");
                mosquitto_unsubscribe(localClient, nullptr, "stations/delete/wifi");
                mosquitto_unsubscribe(localClient, nullptr, "stations/add/dhcp/request");
                mosquitto_unsubscribe(localClient, nullptr, "stations/add/dhcp/ack");
                mosquitto_unsubscribe(localClient, nullptr, "stations/delete/dhcp");
                mosquitto_unsubscribe(localClient, nullptr, "stations/stats/interim");
            }
  } else if ((topic == "stations/add/dhcp/ack") or (topic == "stations/add/dhcp/request")) {
     if(continuousStationUpdates == "1") {
        auto payloadData =
            json_loads(static_cast<char *>(message->payload), 0, nullptr);
        if(payloadData ==  NULL) {
          return;
        }            
        std::string clientMacAddress = std::string(json_string_value(json_object_get(payloadData, "clientMacAddress")));
        std::string uniqueId = std::string(json_string_value(json_object_get(payloadData, "uniqueId")));
        std::string isInterimClient = ExecUciCommand("cat /tmp/interimClients | grep " + clientMacAddress + " | grep " + uniqueId);
          if(!isInterimClient.empty()) {
              ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "debug" + " " + "Mac Address already present.. ignoring " + clientMacAddress);  
          } else {
            ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "debug" + " " + "Publishing to cloud on topic : " + topic);
             mosquitto_publish(mosq, nullptr, (topic + "/" + apMacAddress).data(), message->payloadlen,
                      message->payload, 2, false);                   
          }
          json_decref(payloadData);
     }
  } else {    
    ExecSystemCommand("logger -t ray-mqttServer-uplink-" + apMacAddress + "  -p " + "debug" + " " + "Publishing to cloud on topic : " + topic);
    mosquitto_publish(mosq, nullptr, (topic + "/" + apMacAddress).data(), message->payloadlen,
                      message->payload, 2, false);
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

int mqtt_setup_publisher() {
  bool clean_session = true;

  mosquitto_lib_init();
  auto apMacAddress = read_ap_mac();
  mosq = mosquitto_new(("publisher_" + apMacAddress).data(), clean_session, NULL);
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
  //mosquitto_message_callback_set(mosq, mqtt_server_message_callback);

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

  mosquitto_connect_callback_set(localClient, local_client_connect_callback);
  mosquitto_message_callback_set(localClient, local_client_message_callback);

  mosquitto_log_callback_set(localClient, mosq_log_callback);

  /* Call this once to start a new thread to process network traffic */
  int loop = mosquitto_loop_start(localClient);
  if (loop != MOSQ_ERR_SUCCESS) {
    printf("Unable to start loop: %i\n", loop);
    return 1;
  }

  sleep(5);

  int localLoop = mosquitto_loop_forever(mosq,-1,1);
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
