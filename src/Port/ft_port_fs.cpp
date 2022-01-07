
#include "FS.h"
#include "SD.h"

extern "C" {
typedef void lvbe_FILE;
lvbe_FILE *lvbe_fopen(const char *filename, const char *mode) {
  File f = SD.open(filename, mode);
  if (f) {
    File *f_ptr = new File(f);  // copy to dynamic object
    *f_ptr = f;                 // TODO is this necessary?
    return f_ptr;
  }
  return nullptr;
}
int lvbe_fclose(lvbe_FILE *stream) {
  File *f_ptr = (File *)stream;
  f_ptr->close();
  delete f_ptr;
  // AddLog(LOG_LEVEL_INFO, "LVG: lvbe_fclose(%p)", f_ptr);
  return 0;
}
size_t lvbe_fread(void *ptr, size_t size, size_t count, lvbe_FILE *stream) {
  File *f_ptr = (File *)stream;
  // AddLog(LOG_LEVEL_INFO, "LVG: lvbe_fread (%p, %i, %i, %p)", ptr, size,
  // count, f_ptr);

  int32_t ret = f_ptr->read((uint8_t *)ptr, size * count);
  // AddLog(LOG_LEVEL_INFO, "LVG: lvbe_fread -> %i", ret);
  if (ret < 0) {  // error
    ret = 0;
  }
  return ret;
}
int lvbe_fseek(lvbe_FILE *stream, long int offset, int origin) {
  File *f_ptr = (File *)stream;
  // AddLog(LOG_LEVEL_INFO, "LVG: lvbe_fseek(%p, %i, %i)", f_ptr, offset,
  // origin);

  fs::SeekMode mode = fs::SeekMode::SeekSet;
  if (SEEK_CUR == origin) {
    mode = fs::SeekMode::SeekCur;
  } else if (SEEK_END == origin) {
    mode = fs::SeekMode::SeekEnd;
  }
  bool ok = f_ptr->seek(offset, mode);
  return ok ? 0 : -1;
}
int lvbe_ftell(lvbe_FILE *stream) {
  File *f_ptr = (File *)stream;
  // AddLog(LOG_LEVEL_INFO, "LVG: lvbe_ftell(%p) -> %i", f_ptr,
  // f_ptr->position());
  return f_ptr->position();
}
}
