
#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "3ds/smdh.h"

#define BANNER_GFX_WIDTH 256
#define BANNER_GFX_HEIGHT 128
#define BANNER_GFX_SIZE (BANNER_GFX_WIDTH * BANNER_GFX_HEIGHT)

#define MIN(a, b) (((a)<(b)) ? (a) : (b))

bool utf8_to_utf16(u16* dst, const std::string& src, size_t maxLen);

DatVec read_file(const fs::path& file);
bool write_file(const fs::path& file, const DatVec& contents);
bool write_file(const fs::path& file, const void* data, const size_t size);

Image load_image(const fs::path& file, u32 expectedWidth, u32 expectedHeight);

void image_data_to_tiles(void* out, const Image& img, PixelFormat format);
DatVec convert_to_cgfx(const fs::path& file);
DatVec convert_to_cwav(const fs::path& file, bool loop, u32 loopStartFrame,
                       u32 loopEndFrame);

int cmd_make_banner(const fs::path* images, const fs::path& audio, const fs::path* cgfxFiles,
                    const fs::path& cwavFile, const fs::path& output);
int cmd_make_smdh(SMDH& smdh, const fs::path& icon, const fs::path& output);
int cmd_make_cwav(const fs::path& input, const fs::path& output, bool loop,
                  u32 loopStartFrame, u32 loopEndFrame);
int cmd_lz11(const fs::path& input, const fs::path& output);

#endif
