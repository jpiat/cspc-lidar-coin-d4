#include "coin_d4.hpp"
#include <WiFi.h>
#include "http.h"
#include <WebSocketsServer.h>  //websockets library by Markus Sattler
//https://github.com/Links2004/arduinoWebSockets

#define LIDAR_RX 25
#define LIDAR_TX 26

#define SSID ""
#define PASSWORD ""

WiFiServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#define SCAN_DATA_LENGTH (512 * 3)
float scan_data[SCAN_DATA_LENGTH];
uint32_t scan_data_idx = 0;
void scan_point_callback(float angle_deg, float distance_m, float quality,
                         bool scan_completed) {
  static int i = 0;
  float data_array[3] = { angle_deg, distance_m, quality };
  if (scan_data_idx < SCAN_DATA_LENGTH) {
    scan_data[scan_data_idx++] = angle_deg;
    scan_data[scan_data_idx++] = distance_m * 1000.f;
    scan_data[scan_data_idx++] = quality;
  }
  if (scan_completed) {
    webSocket.broadcastBIN((uint8_t *)scan_data, scan_data_idx * sizeof(float));
    scan_data_idx = 0;
  }
  /*if ((i++ % 20 == 0) || scan_completed) {
    Serial.print(i);
    Serial.print(' ');
    Serial.print(distance_mm);
    Serial.print(' ');
    Serial.print(angle_deg);
    if (scan_completed)
      Serial.println('*');
    else
      Serial.println();
  }*/
}



CoinD4 lidar(&Serial1);
String http_page_index = String(index_html);
bool connected = false;

void setup() {

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("coin-lidar");
  Serial.printf("Connecting to '%s' with '%s'\n", SSID, PASSWORD);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  lidar.setScanPointCallback(scan_point_callback);
#if defined(LIDAR_RX) && defined(LIDAR_TX)
  lidar.begin(LIDAR_RX, LIDAR_TX);
#else
  lidar.begin();
#endif
  lidar.stop();
  delay(1000);
  lidar.start();
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      connected = false;
      break;
    case WStype_CONNECTED:
      connected = true;
      break;
    case WStype_TEXT:
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}


void http_resp() {
  WiFiClient client = server.accept();
  if (client.connected() && client.available()) {
    Serial.println("One client");
    client.flush();
    client.print(http_page_index);
    //client.stop();
  }
}


long long last_packet = 0;
void loop() {
  long long now = millis();
  int ret = lidar.loop();
  if (ret) {
    last_packet = now;
  }
  if (WiFi.status() == WL_CONNECTED) {
    webSocket.loop();
    http_resp();
  }
  if ((now - last_packet) > 5000) {
    lidar.stop();
    delay(1000);
    lidar.start();
    last_packet = now;
  }
}
