
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "cbmd.h"
#include "lz11.h"

static DatVec cbmd_build_data(CBMD &cbmd, bool bnr) {
	CBMDHeader header;
	size_t outputSize = sizeof(CBMDHeader);

	memset(&header, 0, outputSize);
	memcpy(header.magic, CBMD_MAGIC, sizeof(header.magic));

	DatVec compressedCGFXs[14];
	for(int i = 0; i < CBMD_NUM_CGFXS; i++) {
		if(!cbmd.cgfxs[i].empty()) {
			header.cgfxOffsets[i] = outputSize;

			compressedCGFXs[i] = lz11_compress(cbmd.cgfxs[i]);
			outputSize += compressedCGFXs[i].size();
		}
	}

	// align size
	if(bnr) {
		outputSize = (outputSize + 0xF) & ~0xF;
	}

	if(!cbmd.cwav.empty()) {
		header.cwavOffset = outputSize;
		outputSize += cbmd.cwav.size();
	}

	DatVec output(outputSize);
	/*if(!output) {
		printf("ERROR: Could not allocate memory for CBMD data.\n");
		return DatVec();
	}*/
	memcpy(output.data(), &header, sizeof(header));

	for(int i = 0; i < CBMD_NUM_CGFXS; i++) {
		if(!compressedCGFXs[i].empty()) {
			memcpy(&output[header.cgfxOffsets[i]], compressedCGFXs[i].data(), compressedCGFXs[i].size());
		}
	}

	if(!cbmd.cwav.empty()) {
		memcpy(&output[header.cwavOffset], cbmd.cwav.data(), cbmd.cwav.size());
	}

	return output;
}

DatVec cbmd_build(CBMD &cbmd) {
	return cbmd_build_data(cbmd, false);
}

DatVec bnr_build(CBMD &cbmd) {
	return cbmd_build_data(cbmd, true);
}
