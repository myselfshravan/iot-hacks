// Pet packs on LittleFS (/pets/<name>.pak) and which one is active
// (remembered across reboots). "builtin" is the drawn blob.
#pragma once

#include <Arduino.h>

#include "pet.h"
#include "pet_pack.h"

class PetStore {
 public:
  explicit PetStore(Pet& pet) : pet_(pet) {}

  void begin();                     // mounts LittleFS, restores the active pet
  bool use(const String& name);     // "builtin" or an uploaded pack
  const String& active() const { return active_; }
  String listJson();                // {"active":..,"pets":[{"name","kb"}],"free_kb":..}

  // Streaming upload: begin, write chunks, finish (atomically replaces the pack).
  bool beginUpload(const String& name);
  void write(const uint8_t* data, size_t len);
  bool finishUpload();

  static bool validName(const String& name);

 private:
  Pet& pet_;
  PetPack pack_;
  String active_ = "builtin";
  String uploadName_;
  fs::File upload_;
  bool uploadOk_ = false;
};
