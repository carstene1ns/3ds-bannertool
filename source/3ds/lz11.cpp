
#include <cstring>
#include <sstream>

#include "lz11.h"
#include "utils.h"
#include "log.h"

// Ported from: https://github.com/svn2github/3DS-Explorer/blob/master/3DSExplorer/DSDecmp/Formats/Nitro/LZ11.cs

static u32 lz11_get_occurence_length(u8* newPtr, u32 newLength, u8* oldPtr, u32 oldLength, u32* disp) {
	if(disp) {
		*disp = 0;
	}

	if(newLength == 0 || oldLength == 0) {
		return 0;
	}

	u32 maxLength = 0;
	for(u32 i = 0; i < oldLength - 1; i++) {
		u8* currentOldStart = oldPtr + i;
		u32 currentLength = 0;
		for(u32 j = 0; j < newLength; j++) {
			if(*(currentOldStart + j) != *(newPtr + j)) {
				break;
			}

			currentLength++;
		}

		if(currentLength > maxLength) {
			maxLength = currentLength;
			if(disp) {
				*disp = oldLength - i;
			}

			if(maxLength == newLength) {
				break;
			}
		}
	}

	return maxLength;
}

DatVec lz11_compress(DatVec &input) {
	if(input.size() > 0xFFFFFF) {
		LOG_F(ERROR, "LZ11 input is too large.");
		return DatVec();
	}

	// FIXME: rewrite to use DatVec directly // c1
	std::stringstream ss;

	u8 header[4] = { 0x11, (u8)(input.size() & 0xFF), (u8)((input.size() >> 8) & 0xFF), (u8)((input.size() >> 16) & 0xFF) };
	ss.write((char*) header, 4);

	size_t compressedLength = 4;
	u8 outbuffer[8 * 4 + 1];
	outbuffer[0] = 0;
	u32 bufferlength = 1;
	u32 bufferedBlocks = 0;
	u32 readBytes = 0;
	while(readBytes < input.size()) {
		if(bufferedBlocks == 8) {
			ss.write((char*) outbuffer, bufferlength);
			compressedLength += bufferlength;
			outbuffer[0] = 0;
			bufferlength = 1;
			bufferedBlocks = 0;
		}

		u32 disp = 0;
		u32 oldLength = MIN(readBytes, 0x1000);
		u32 length = lz11_get_occurence_length(&input[readBytes], MIN(input.size() - readBytes, 0x10110),
		                                       &input[readBytes - oldLength], oldLength, &disp);
		if(length < 3) {
			outbuffer[bufferlength++] = input[readBytes++];
		} else {
			readBytes += length;
			outbuffer[0] |= (u8)(1 << (7 - bufferedBlocks));
			if(length > 0x110) {
				outbuffer[bufferlength] = 0x10;
				outbuffer[bufferlength] |= (u8)(((length - 0x111) >> 12) & 0x0F);
				bufferlength++;
				outbuffer[bufferlength] = (u8)(((length - 0x111) >> 4) & 0xFF);
				bufferlength++;
				outbuffer[bufferlength] = (u8)(((length - 0x111) << 4) & 0xF0);
			} else if(length > 0x10) {
				outbuffer[bufferlength] = 0x00;
				outbuffer[bufferlength] |= (u8)(((length - 0x111) >> 4) & 0x0F);
				bufferlength++;
				outbuffer[bufferlength] = (u8)(((length - 0x111) << 4) & 0xF0);
			} else {
				outbuffer[bufferlength] = (u8)(((length - 1) << 4) & 0xF0);
			}

			outbuffer[bufferlength] |= (u8)(((disp - 1) >> 8) & 0x0F);
			bufferlength++;
			outbuffer[bufferlength] = (u8)((disp - 1) & 0xFF);
			bufferlength++;
		}

		bufferedBlocks++;
	}

	if(bufferedBlocks > 0) {
		ss.write((char*) outbuffer, bufferlength);
		compressedLength += bufferlength;
	}

	if(compressedLength % 4 != 0) {
		u32 padLength = 4 - (compressedLength % 4);
		u8 pad[padLength] = {0};
		ss.write((char*) pad, padLength);
		compressedLength += padLength;
	}

	DatVec output(compressedLength);
	ss.read((char*) output.data(), compressedLength);

	return output;
}
