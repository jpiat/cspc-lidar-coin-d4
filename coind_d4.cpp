#include "coin_d4.hpp"

CoinD4::CoinD4(SerialUART  * intf) {
  this->intf = intf;
  this->current_state = READ_SYNC;
}

void CoinD4::begin() {
  this->intf->begin(230400);
}

void CoinD4::start() {
  this->intf->write(CoinD4::start_lidar, 4);
  this->intf->flush();
  delay(200);
  this->intf->write(CoinD4::high_speed, 4);
  this->intf->flush();
}
void CoinD4::stop() {
  this->intf->write(CoinD4::end_lidar, 4);
  this->intf->flush();
  delay(200);
}
int CoinD4::loop() {
  int ret = 0 ;
  int actual_read;
  int to_read;
  int serial_available = 0;
  serial_available = this->intf->available();
  if(serial_available == 0) return 0 ;
  switch (this->current_state) {
    case READ_SYNC:
      if (serial_available >= 2) {
        uint8_t aa = this->intf->read();
        if (0xAA == aa) {
          uint8_t cc = this->intf->read();
          if (cc == 0x55) {
            this->current_state = READ_HEADER;
            checksum_calc = 0x55AA;
          }
        }
      }
      break;
    case READ_HEADER:
      if (serial_available >= 8) {
        this->intf->readBytes(this->rx_buffer, 8);
        if ((this->rx_buffer[0] & 0x01) == 1) {
          //RING START, can be used to compute scan duration
          long long now = millis();
          long long diff = now - last_start;
          //Serial.println(diff);
          last_start = now;
          this->rate = 1000.f/(float) (diff) ;
          this->scan_freq = (this->rx_buffer[0] & 0xFE) >> 1;
        }
        this->nb_samples_and_ct_cal = this->rx_buffer[0] + this->rx_buffer[1] << 8;
        this->nb_samples = this->rx_buffer[1];
        this->start_angle = (this->rx_buffer[2] + (this->rx_buffer[3] << 8));
        if (this->start_angle & 0x01 == 0) {
          this->current_state = READ_SYNC;
          break;
        }
        this->checksum_calc ^= this->start_angle;
        this->start_angle = start_angle >> 1;
        this->end_angle = (this->rx_buffer[4] + (this->rx_buffer[5] << 8));
        if (this->end_angle & 0x01 == 0) {
          this->current_state = READ_SYNC;
          break;
        }
        this->checksum_calc ^= this->end_angle;
        this->end_angle = this->end_angle >> 1;
        this->checksum = (this->rx_buffer[6] + (this->rx_buffer[7] << 8));  //Need to check checksum
        this->packet_buffer_to_read = nb_samples * 3;
        this->start_angle_f = ((float)start_angle) / 64.f;
        this->end_angle_f = ((float)end_angle) / 64.f;
        if (nb_samples == 1) {
          this->angle_interval = 0;
        } else {
          if (end_angle_f < start_angle_f) {
            this->end_angle_f += 360;
          }
          this->angle_interval = (this->end_angle_f - this->start_angle_f) / (this->nb_samples - 1);
        }
        this->rx_buffer_write_index = 0;
        if (this->packet_buffer_to_read <= 256) {
          this->rx_buffer_write_index = 0;
          this->current_state = READ_PACKETS;
        } else {
          this->current_state = READ_SYNC;
        }
      }
      break;
    case READ_PACKETS:
      to_read = this->packet_buffer_to_read;
      if (to_read > serial_available) {
        to_read = serial_available;
      }
      if (to_read == 0 && this->packet_buffer_to_read <= 0) {
        this->current_state = READ_SYNC;
      }
      if (to_read > 0) {
        actual_read = this->intf->readBytes(&this->rx_buffer[this->rx_buffer_write_index], to_read);
        if (actual_read > 0) {
          this->rx_buffer_write_index += actual_read;
          this->packet_buffer_to_read -= actual_read;
        }
      }
      if (this->packet_buffer_to_read == 0) {
        for (int i = 0; i < nb_samples; i++) {
          uint16_t distance = (rx_buffer[i * 3 + 2] << 6) + (rx_buffer[i * 3 + 1] >> 2);
          uint16_t quality = (rx_buffer[i * 3] >> 2) + ((rx_buffer[i * 3 + 1] & 0x03) << 6);
          uint16_t exp = rx_buffer[i * 3] & 0x01;

          float distance_m_f = ((float)distance) / 1000.f;
          float angle_deg_f = (start_angle_f + (i * angle_interval));
          angle_deg_f = angle_deg_f >= 360.f ? (angle_deg_f - 360.f) : angle_deg_f;
          this->scan_point_callback(angle_deg_f, distance_m_f, (float) quality,  exp);
        }
        ret = 1 ;
        this->current_state = READ_SYNC;
      }
      break;
    default:
      this->current_state = READ_SYNC;
      break;
  }
  return ret ;
}


void CoinD4::setScanPointCallback(ScanPointCallback scan_callback) {
  this->scan_point_callback = scan_callback;
}
 float CoinD4::getScanFreqHz(){
  return this-> rate ;
 }