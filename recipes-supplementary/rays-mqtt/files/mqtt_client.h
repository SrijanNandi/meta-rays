#ifndef MQTT_CLIENT_H_
#define MQTT_CLIENT_H_

#include <mosquitto.h>

#define MQTT_HOST    "localhost"
#define MQTT_PORT    1883
#define KEEP_ALIVE   60
//#define TOPIC_MAC_TO_SSID_MAPPING   "stations/add/wifi"
#define TOPIC_ADD_DHCP_ACK     "stations/add/dhcp/ack"
#define TOPIC_ADD_WIFI   "stations/add/wifi"
#define TOPIC_DELETE_WIFI   "stations/delete/wifi"
#define TOPIC_CAPTIVE_URL   "topic/captive/url"
#define TOPIC_ADD_DHCP_REQUEST     "stations/add/dhcp/request"

int mqtt_setup_publisher();
int mqtt_setup_receiver();
int mqtt_close();
int mqtt_send(const char *topic, const char *msg);
int on_message_callback(char *topic, char *data);

#endif /* MQTT_CLIENT_H_ */
