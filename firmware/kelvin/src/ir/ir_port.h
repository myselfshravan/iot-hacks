// The IR hardware: one LED transmitter and one VS1838B receiver, shared by
// every remote (AC, projector, ...).
#pragma once

#include <IRrecv.h>
#include <IRsend.h>

class IrPort {
 public:
  IrPort(uint8_t sendPin, uint8_t recvPin, uint8_t recvPowerPin);

  void begin();

  IRsend& tx() { return tx_; }

  // True when a new frame was captured; it's then available in frame().
  bool receive() { return rx_.decode(&frame_); }
  const decode_results& frame() const { return frame_; }

 private:
  uint8_t recvPowerPin_;
  IRsend tx_;
  IRrecv rx_;
  decode_results frame_;
};
