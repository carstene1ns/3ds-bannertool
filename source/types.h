#ifndef TYPES_H
#define TYPES_H

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <filesystem>
#include <vector>

#define BITNUM(n) (1U << (n))

// shorthand types

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

namespace fs = std::filesystem;

using ArgMap = std::map<std::string, std::string>;
using DatVec = std::vector<u8>;

// image container

struct Image {
	Image() = default;
	explicit Image(const fs::path& file);
	Image(int w, int h);

	operator bool() const {
		return !pixels.empty();
	}

	u32 width = 0;
	u32 height = 0;
	DatVec pixels;
};

enum PixelFormat {
	RGB565,
	RGBA4444
};

#endif
