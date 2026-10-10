#include "pet_store.h"

#include <LittleFS.h>
#include <Preferences.h>

namespace {

String pathOf(const String& name) { return "/pets/" + name + ".pak"; }

}  // namespace

bool PetStore::validName(const String& name) {
  if (name.isEmpty() || name.length() > 20) return false;
  for (char ch : name)
    if (!isalnum(ch) && ch != '-' && ch != '_') return false;
  return true;
}

void PetStore::begin() {
  // The default partition table's "spiffs" partition (~1.4 MB) holds the packs.
  if (!LittleFS.begin(true, "/littlefs", 10, "spiffs")) {
    Serial.println("LittleFS mount failed: built-in pet only");
    return;
  }
  if (!LittleFS.exists("/pets")) LittleFS.mkdir("/pets");
  Preferences prefs;
  prefs.begin("dexi", true);
  String saved = prefs.getString("pet", "builtin");
  prefs.end();
  if (!use(saved)) use("builtin");
}

bool PetStore::use(const String& name) {
  if (name == "builtin") {
    pack_.unload();
    pet_.usePack(nullptr);
  } else {
    if (!validName(name) || !pack_.load(LittleFS, pathOf(name))) return false;
    pet_.usePack(&pack_);
  }
  active_ = name;
  Preferences prefs;
  prefs.begin("dexi", false);
  prefs.putString("pet", name);
  prefs.end();
  Serial.printf("pet: %s\n", name.c_str());
  return true;
}

bool PetStore::remove(const String& name) {
  if (!validName(name) || name == "builtin" || !LittleFS.exists(pathOf(name))) return false;
  if (active_ == name) use("builtin");
  return LittleFS.remove(pathOf(name));
}

String PetStore::listJson() {
  String j = "{\"active\":\"" + active_ + "\",\"pets\":[{\"name\":\"builtin\",\"kb\":0}";
  File dir = LittleFS.open("/pets");
  for (File f = dir.openNextFile(); f; f = dir.openNextFile()) {
    String n = f.name();
    if (!n.endsWith(".pak")) continue;
    j += ",{\"name\":\"" + n.substring(0, n.length() - 4) + "\",\"kb\":" + String(f.size() / 1024) + "}";
  }
  j += "],\"free_kb\":" + String((LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024) + "}";
  return j;
}

bool PetStore::beginUpload(const String& name) {
  uploadOk_ = validName(name) && name != "builtin";
  if (!uploadOk_) return false;
  uploadName_ = name;
  if (active_ == name) use("builtin");  // release the file we're about to replace
  upload_ = LittleFS.open("/pets/upload.tmp", "w");
  uploadOk_ = (bool)upload_;
  return uploadOk_;
}

void PetStore::write(const uint8_t* data, size_t len) {
  if (uploadOk_ && upload_.write(data, len) != len) uploadOk_ = false;  // flash full
}

bool PetStore::finishUpload() {
  if (upload_) upload_.close();
  if (!uploadOk_) {
    LittleFS.remove("/pets/upload.tmp");
    return false;
  }
  LittleFS.remove(pathOf(uploadName_));
  return LittleFS.rename("/pets/upload.tmp", pathOf(uploadName_));
}
