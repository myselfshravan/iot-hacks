#include "ir_port.h"

// 1024-entry capture buffer, 50 ms end-of-frame gap, save_buffer=true so
// decode() resumes capture by itself.
IrPort::IrPort(uint8_t sendPin, uint8_t recvPin, uint8_t recvPowerPin)
    : recvPowerPin_(recvPowerPin), tx_(sendPin), rx_(recvPin, 1024, 50, true) {}

void IrPort::begin() {
  pinMode(recvPowerPin_, OUTPUT);
  digitalWrite(recvPowerPin_, HIGH);
  tx_.begin();
  rx_.enableIRIn();
}
