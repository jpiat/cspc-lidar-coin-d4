#include "coin_d4.hpp"


void scan_point_callback(float angle, float distance, float quality, bool exp){
    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.print(" Distance: ");
    Serial.print(distance);
    Serial.print("Quality: ");
    Serial.println(quality);
}
CoinD4 lidar(&Serial1);
void setup() {
  Serial.begin();
  lidar.setScanPointCallback(scan_point_callback);
  lidar.begin();
  lidar.stop();
  delay(1000);
  lidar.start();
}



long long last_packet = 0 ;
void loop() {
  long long now = millis();
  int ret = lidar.loop();
  if(ret){
    last_packet = now ;
  }

  if((now - last_packet) > 5000){
    lidar.stop();
    delay(1000);
    lidar.start();
  }

}
