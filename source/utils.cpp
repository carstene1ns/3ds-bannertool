
#include <fstream>
#include <iostream>
#include <stb_vorbis.h>
#include <dr_wav.h>
#include <utf8.h>

#include "utils.h"
#include "log.h"
#include "3ds/cwav.h"
#include "3ds/data.h"
#include "3ds/cbmd.h"
#include "3ds/lz11.h"

bool utf8_to_utf16(u16* dst, const std::string& src, size_t maxLen) {
	if(maxLen == 0) {
		return false;
	}

	std::u16string str16 = utf8::utf8to16(src);
	size_t copyLen = str16.length() * sizeof(char16_t);
	memcpy(dst, str16.data(), MIN(copyLen, maxLen));

	// truncation
	if(copyLen > maxLen) {
		return false;
	}

	return true;
}

DatVec read_file(const fs::path& file) {
	std::error_code ec;

	LOG_F(DEBUG, "Reading file \"%s\"...", file.u8string().c_str());

	size_t fileSize = fs::file_size(file, ec);
	if(ec) {
		LOG_F(ERROR, "Failed to determine file size: %s.", ec.message().c_str());
		return DatVec();
	}

	std::ifstream ifs(file, std::ios::binary);
	if(!ifs.is_open()) {
		LOG_F(ERROR, "Could not open file for reading.");
		return DatVec();
	}

	DatVec buffer(fileSize);
	ifs.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	bool ret = ifs.good();
	ifs.close();

	if(!ret) {
		LOG_F(ERROR, "Failed to read file.");
		return DatVec();
	}

	return buffer;
}

bool write_file(const fs::path& file, const DatVec& contents) {
	return write_file(file, contents.data(), contents.size());
}

bool write_file(const fs::path& file, const void* data, const size_t size) {
	LOG_F(DEBUG, "Writing to file \"%s\"...", file.u8string().c_str());

	std::ofstream ofs(file, std::ios::binary | std::ios::trunc);
	if(!ofs.is_open()) {
		LOG_F(ERROR, "Could not open file for writing.");
		return false;
	}

	ofs.write(reinterpret_cast<const char*>(data), size);
	bool ret = ofs.good();
	ofs.close();

	if(!ret) {
		LOG_F(ERROR, "Failed to write file.");
		return false;
	}

	return true;
}

Image load_image(const fs::path& file, u32 expectedWidth, u32 expectedHeight) {
	Image img(file);

	if((expectedWidth != 0 && img.width != expectedWidth) ||
	   (expectedHeight != 0 && img.height != expectedHeight)) {
		LOG_F(ERROR, "Image must be exactly %d x %d in size.", expectedWidth, expectedHeight);
		return Image();
	}

	return img;
}

// swizzle and convert colors
void image_data_to_tiles(void* out, const Image &img, PixelFormat format) {
	for(u32 y = 0; y < img.height; y++) {
		for(u32 x = 0; x < img.width; x++) {
			u32 index = (((y >> 3) * (img.width >> 3) + (x >> 3)) << 6) +
			            ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3));

			const u8* pixel = img.pixels.data() + (y * img.width + x) * sizeof(u32);
			u16 color = 0;
			if(format == RGB565) {
				float alpha = pixel[3] / 255.0f;
				color = (u16)((((u8)(pixel[0] * alpha) & ~0x7) << 8) |
				              (((u8)(pixel[1] * alpha) & ~0x3) << 3) |
				              ((u8)(pixel[2] * alpha) >> 3));
			} else if(format == RGBA4444) {
				color = (u16)(((pixel[0] & ~0xF) << 8) |
				              ((pixel[1] & ~0xF) << 4) |
				              (pixel[2] & ~0xF) |
				              (pixel[3] >> 4));
			}

			((u16*) out)[index] = color;
		}
	}
}

DatVec convert_to_cgfx(const fs::path& file) {
	Image img = load_image(file, BANNER_GFX_WIDTH, BANNER_GFX_HEIGHT);
	if(!img) {
		return DatVec();
	}

	size_t bufferSize = BANNER_CGFX_HEADER_LENGTH + (BANNER_GFX_SIZE * sizeof(u16));
	DatVec buffer(bufferSize);
	/*if(!buffer) {
		printf("ERROR: Could not allocate memory for CGFX data.\n");
		return nullptr;
	}*/

	memcpy(buffer.data(), BANNER_CGFX_HEADER, BANNER_CGFX_HEADER_LENGTH);
	image_data_to_tiles(&buffer[BANNER_CGFX_HEADER_LENGTH], img, RGBA4444);

	return buffer;
}

DatVec convert_to_cwav(const fs::path& file, bool loop, u32 loopStartFrame, u32 loopEndFrame) {
	DatVec input = read_file(file);
	if(input.empty()) {
		return DatVec();
	}

	CWAV cwav;
	cwav.loop = loop;
	cwav.loopStartFrame = loopStartFrame;
	cwav.loopEndFrame = loopEndFrame;

	// First check OGG
	if(memcmp(input.data(), "OggS", 4) == 0) {
		int error = 0;
		stb_vorbis* vorbis = stb_vorbis_open_memory(input.data(), input.size(), &error, nullptr);
		if(vorbis) {
			stb_vorbis_info info = stb_vorbis_get_info(vorbis);
			unsigned int sampleCount = stb_vorbis_stream_length_in_samples(vorbis) * info.channels;

			cwav.channels = info.channels;
			cwav.sampleRate = info.sample_rate;
			cwav.bitsPerSample = 16;
			cwav.data.resize(sampleCount * sizeof(short));
			stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, (short*) cwav.data.data(), sampleCount);

			stb_vorbis_close(vorbis);
		} else {
			LOG_F(ERROR, "Failed to load vorbis file \"%s\": Error %d.", file.u8string().c_str(), error);
			return DatVec();
		}
	} else {
		// Then let dr_wav try decoding
		drwav wav;
		if(drwav_init_memory(&wav, input.data(), input.size(), nullptr)) {
			cwav.channels = wav.channels;
			cwav.sampleRate = wav.sampleRate;

			// We use the audio data directly
			if(wav.translatedFormatTag == DR_WAVE_FORMAT_PCM &&
			   (wav.bitsPerSample == 8 || wav.bitsPerSample == 16)) {
				cwav.bitsPerSample = wav.bitsPerSample;
				cwav.data.resize(wav.totalPCMFrameCount * wav.channels * wav.bitsPerSample / 8);
				drwav_read_pcm_frames(&wav, wav.totalPCMFrameCount, cwav.data.data());
				// FIXME: dr_wav only can decode unsigned 8 bit PCM data.
				//        vgmstream tells for bcwav it has signed data
				/*
				if(wav.bitsPerSample == 8) {
					LOG_WARNING("8 bit PCM is expected to be unsigned. Converting to signed!");
					for (size_t i = 0; i < cwav.dataSize; i++) {
						((u8*)cwav.data)[i] += 128;
					}
				}*/
			} else {
				if(wav.translatedFormatTag == DR_WAVE_FORMAT_ADPCM) {
					LOG_F(INFO, "ADPCM encoding is currently unsupported and will be converted to 16 bit PCM!");
				} else {
					LOG_F(INFO, "Unsupported encoding. Converting to 16 bit PCM!");
				}
				// Simply decode as 16 bit PCM frames
				cwav.bitsPerSample = 16;
				cwav.data.resize(wav.totalPCMFrameCount * wav.channels * sizeof(short));
				drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, (short*) cwav.data.data());
			}

			drwav_uninit(&wav);
		} else {
			LOG_F(ERROR, "Audio file magic '%c%c%c%c' in file \"%s\" is unrecognized or unsupported.", input[0], input[1], input[2],
			      input[3], file.u8string().c_str());
			return DatVec();
		}
	}

	/*if(cwav.data.empty()) {
		printf("ERROR: Could not allocate memory for CWAV sample data.\n");
		return DatVec();
	}*/

	return cwav_build(cwav);
}

int cmd_make_banner(const fs::path* images, const fs::path& audio, const fs::path* cgfxFiles,
                    const fs::path& cwavFile, const fs::path& output) {
	CBMD cbmd;

	for(int i = 0; i < CBMD_NUM_CGFXS; i++) {
		if(!cgfxFiles[i].empty()) {
			cbmd.cgfxs[i] = read_file(cgfxFiles[i]);
		} else if(!images[i].empty()) {
			cbmd.cgfxs[i] = convert_to_cgfx(images[i]);
		}
		if(!(cgfxFiles[i].empty() && images[i].empty())) {
			if(cbmd.cgfxs[i].empty()) {
				return 1;
			}
		}
	}

	if(!cwavFile.empty()) {
		cbmd.cwav = read_file(cwavFile);
	} else {
		cbmd.cwav = convert_to_cwav(audio, false, 0, 0);
	}
	if(cbmd.cwav.empty()) {
		return 1;
	}

	DatVec bnr = bnr_build(cbmd);
	if(bnr.empty()) {
		LOG_F(ERROR, "Failed to generate banner.");
		return 1;
	}

	if(!write_file(output, bnr)) {
		return 1;
	}

	LOG_F(INFO, "Created banner \"%s\".", output.u8string().c_str());
	return 0;
}

int cmd_make_smdh(SMDH& smdh, const fs::path& icon, const fs::path& output) {
	Image icon48 = load_image(icon.c_str(), SMDH_LARGE_ICON_DIM, SMDH_LARGE_ICON_DIM);
	if(!icon48) {
		return 1;
	}

	Image icon24(SMDH_SMALL_ICON_DIM, SMDH_SMALL_ICON_DIM);

	int scale = SMDH_LARGE_ICON_DIM / SMDH_SMALL_ICON_DIM;
	int samples = scale * scale;
	for(int y = 0; y < SMDH_LARGE_ICON_DIM; y += scale) {
		for(int x = 0; x < SMDH_LARGE_ICON_DIM; x += scale) {
			u32 r = 0;
			u32 g = 0;
			u32 b = 0;
			u32 a = 0;

			for(int oy = 0; oy < scale; oy++) {
				for(int ox = 0; ox < scale; ox++) {
					int i = ((y + oy) * SMDH_LARGE_ICON_DIM + (x + ox)) * sizeof(u32);
					r += icon48.pixels[i];
					g += icon48.pixels[i + 1];
					b += icon48.pixels[i + 2];
					a += icon48.pixels[i + 3];
				}
			}

			int i = ((y / scale) * SMDH_SMALL_ICON_DIM + (x / scale)) * sizeof(u32);
			icon24.pixels[i] = (u8)(r / samples);
			icon24.pixels[i + 1] = (u8)(g / samples);
			icon24.pixels[i + 2] = (u8)(b / samples);
			icon24.pixels[i + 3] = (u8)(a / samples);
		}
	}

	image_data_to_tiles(smdh.largeIcon, icon48, RGB565);
	image_data_to_tiles(smdh.smallIcon, icon24, RGB565);

	if(!write_file(output, &smdh, sizeof(SMDH))) {
		return 1;
	}

	LOG_F(INFO, "Created SMDH \"%s\".", output.u8string().c_str());
	return 0;
}

int cmd_make_cwav(const fs::path& input, const fs::path& output, bool loop, u32 loopStartFrame,
                  u32 loopEndFrame) {
	DatVec cwav = convert_to_cwav(input, loop, loopStartFrame, loopEndFrame);
	if(cwav.empty() || !write_file(output, cwav)) {
		return 1;
	}

	LOG_F(INFO, "Created CWAV \"%s\".", output.u8string().c_str());
	return 0;
}

int cmd_lz11(const fs::path& input, const fs::path& output) {
	DatVec data = read_file(input);
	if(data.empty()) {
		return 1;
	}

	DatVec compressed = lz11_compress(data);
	if(compressed.empty()) {
		LOG_F(ERROR, "Failed to compress data.");
		return 1;
	}

	if(!write_file(output, compressed)) {
		return 1;
	}

	LOG_F(INFO, "Compressed to file \"%s\".", output.u8string().c_str());
	return 0;
}
