#include "pet_pack.h"

namespace {

// Small buffered reader: frames are read straight from flash, a block at a time.
class Reader {
 public:
  explicit Reader(fs::File& f) : f_(f) {}
  int byte() {
    if (pos_ == len_) {
      len_ = f_.read(buf_, sizeof(buf_));
      pos_ = 0;
      if (!len_) return -1;
    }
    return buf_[pos_++];
  }
  uint16_t u16() {
    int lo = byte(), hi = byte();
    return (lo & 0xFF) | ((hi & 0xFF) << 8);
  }

 private:
  fs::File& f_;
  uint8_t buf_[512];
  size_t len_ = 0, pos_ = 0;
};

template <typename T>
bool readValue(fs::File& f, T* v) {
  return f.read((uint8_t*)v, sizeof(T)) == sizeof(T);
}

}  // namespace

bool PetPack::load(fs::FS& fs, const String& path) {
  unload();
  file_ = fs.open(path, "r");
  if (!file_) return false;
  char magic[4];
  if (file_.read((uint8_t*)magic, 4) != 4 || memcmp(magic, "DXP1", 4) != 0 ||
      !readValue(file_, &animCount_) || !readValue(file_, &w_) || !readValue(file_, &h_) ||
      !readValue(file_, &scale_) || file_.read(slotAnim_, kSlots) != kSlots ||
      animCount_ == 0 || animCount_ > kMaxAnims) {
    unload();
    return false;
  }
  for (int i = 0; i < animCount_; i++) {
    char name[16];
    file_.read((uint8_t*)name, 16);
    if (!readValue(file_, &anims_[i].frames) || !readValue(file_, &anims_[i].frameMs) ||
        !readValue(file_, &anims_[i].table)) {
      unload();
      return false;
    }
  }
  for (auto& s : slotAnim_)
    if (s >= animCount_) s = 0;
  return true;
}

void PetPack::unload() {
  if (file_) file_.close();
  animCount_ = 0;
}

void PetPack::draw(GFXcanvas16& c, int slot, int cx, int bottom, uint32_t now) {
  if (!file_) return;
  const Anim& a = anims_[slotAnim_[constrain(slot, 0, kSlots - 1)]];
  if (!a.frames) return;
  uint32_t frame = (now / max<uint16_t>(a.frameMs, 1)) % a.frames;
  uint32_t offset;
  file_.seek(a.table + frame * 4);
  if (!readValue(file_, &offset)) return;
  file_.seek(offset);

  Reader in(file_);
  int x0 = cx - width() / 2, y0 = bottom - height();
  uint32_t total = (uint32_t)w_ * h_, p = 0;
  auto paint = [&](uint32_t px, uint16_t colour) {
    c.fillRect(x0 + (px % w_) * scale_, y0 + (px / w_) * scale_, scale_, scale_, colour);
  };
  while (p < total) {
    int t = in.byte();
    if (t < 0) return;
    int n = (t & (t < 0x80 ? 0x7F : 0x3F)) + 1;
    if (t < 0x80) {
      p += n;  // transparent: the world shows through
    } else if (t < 0xC0) {
      for (int i = 0; i < n && p < total; i++) paint(p++, in.u16());
    } else {
      uint16_t colour = in.u16();
      for (int i = 0; i < n && p < total; i++) paint(p++, colour);
    }
  }
}
