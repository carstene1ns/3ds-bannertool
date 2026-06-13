
#include <stb_image.h>

#include "types.h"
#include "utils.h"
#include "log.h"

Image::Image(const fs::path& file) {
	// read whole file
	DatVec png = read_file(file);

	// convert to image
	int w, h;
	u8 *data = stbi_load_from_memory(png.data(), png.size(), &w, &h, nullptr, STBI_rgb_alpha);
	if(!data) {
		LOG_F(ERROR, "Could not load image file: %s.", stbi_failure_reason());
		return;
	}

	width = w;
	height = h;

	// copy pixels, discard buffer
	pixels.assign(data, data + w * h * sizeof(u32));
	stbi_image_free(data);
}

Image::Image(int w, int h) : width(w), height(h) {
	pixels.clear();
	pixels.resize(w * h * sizeof(u32));
}
