#pragma once
#include <Arduino.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

typedef enum coin_d4_read_state {
  READ_SYNC,
  READ_HEADER,
  READ_PACKETS,
} coin_d4_read_state;




class CoinD4 {
public:
  static constexpr uint8_t start_lidar[4] = { 0xAA, 0x55, 0xF0, 0x0F };
  static constexpr uint8_t end_lidar[4] = { 0xAA, 0x55, 0xF5, 0x0A };
  static constexpr uint8_t low_exposure[4] = { 0xAA, 0x55, 0xF1, 0x0E };
  static constexpr uint8_t high_exposure[4] = { 0xAA, 0x55, 0xF2, 0x0D };
  static constexpr uint8_t get_version[4] = { 0xAA, 0x55, 0xFC, 0x03 };
  static constexpr uint8_t high_speed[4] = { 0xAA, 0x55, 0xF2, 0x0D };
  static constexpr uint8_t low_speed[4] = { 0xAA, 0x55, 0xF1, 0x0E };
  typedef void (*ScanPointCallback)(float, float, float, bool);

public:
  CoinD4(SerialUART* intf);
  void begin();
  void init();
  void start();
  void stop();
  int loop();


  void setScanPointCallback(ScanPointCallback scan_callback);
  float getScanFreqHz();


protected:

  SerialUART* intf;
  uint8_t rx_buffer[256];
  uint8_t rx_buffer_write_index = 0;
  ScanPointCallback scan_point_callback;
  coin_d4_read_state current_state;
  uint16_t packet_buffer_to_read ;
  uint16_t nb_samples = 0;
  uint16_t start_angle = 0;
  uint16_t end_angle = 0;
  uint16_t checksum = 0;
  uint16_t checksum_calc = 0;
  float angle_interval;
  float angle_interval_last_package;
  uint8_t scan_freq = 0;
  long long last_start = 0;
  float rate  = -1; 
  bool one_turn ;
  float start_angle_f, end_angle_f;
};