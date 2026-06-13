
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include "cwav.h"

DatVec cwav_build(CWAV &cwav) {
	u32 headerSize = (sizeof(CWAVHeader) + 0x1F) & ~0x1F;
	u32 infoSize = ((sizeof(CWAVInfoBlockHeader) +
	                 (cwav.channels * (sizeof(CWAVReference) +
	                                   sizeof(CWAVChannelInfo)))) + 0x1F) & ~0x1F;
	u32 dataSize = ((sizeof(CWAVDataBlock) + 0x1F) & ~0x1F) + cwav.data.size();

	u32 outputSize = headerSize + infoSize + dataSize;

	DatVec output(outputSize);
	/*if(!output) {
		LOG_ERROR("Could not allocate memory for CWAV data.");
		return DatVec();
	}*/

	CWAVHeader* header = (CWAVHeader*) &output[0];
	memcpy(header->magic, CWAV_MAGIC, sizeof(header->magic));
	header->endianness = CWAV_ENDIANNESS_LITTLE;
	header->headerSize = (u16) headerSize;
	header->version = CWAV_VERSION;
	header->fileSize = outputSize;
	header->numBlocks = 2;
	header->infoBlock.ref.typeId = CWAV_REF_INFO_BLOCK;
	header->infoBlock.ref.offset = headerSize;
	header->infoBlock.size = infoSize;
	header->dataBlock.ref.typeId = CWAV_REF_DATA_BLOCK;
	header->dataBlock.ref.offset = headerSize + infoSize;
	header->dataBlock.size = dataSize;

	CWAVInfoBlockHeader* infoBlockHeader = (CWAVInfoBlockHeader*) &output[headerSize];
	memcpy(infoBlockHeader->header.magic, CWAV_BLOCK_MAGIC_INFO, sizeof(infoBlockHeader->header.magic));
	infoBlockHeader->header.size = infoSize;
	infoBlockHeader->encoding = cwav.bitsPerSample == 16 ? CWAV_ENCODING_PCM16 : CWAV_ENCODING_PCM8;
	infoBlockHeader->loop = cwav.loop;
	infoBlockHeader->sampleRate = cwav.sampleRate;
	infoBlockHeader->loopStartFrame = cwav.loopStartFrame;
	infoBlockHeader->loopEndFrame = cwav.loopEndFrame != 0 ? cwav.loopEndFrame :
	                                cwav.data.size() / cwav.channels / (cwav.bitsPerSample / 8);
	infoBlockHeader->channelInfos.count = cwav.channels;
	for(u32 c = 0; c < cwav.channels; c++) {
		infoBlockHeader->channelInfos.contents[c].typeId = CWAV_REF_CHANNEL_INFO;
		infoBlockHeader->channelInfos.contents[c].offset = sizeof(CWAVReferenceTable) + (cwav.channels * sizeof(
		        CWAVReference)) + (c * sizeof(CWAVChannelInfo));

		CWAVChannelInfo* info = (CWAVChannelInfo*) &output[headerSize + sizeof(CWAVInfoBlockHeader) +
		                        (cwav.channels * sizeof(CWAVReference)) + (c * sizeof(CWAVChannelInfo))];
		info->samples.typeId = CWAV_REF_SAMPLE_DATA;
		info->samples.offset = (((sizeof(CWAVDataBlock) + 0x1F) & ~0x1F) - sizeof(CWAVDataBlock)) + (c *
		                       (cwav.data.size() / cwav.channels));
		info->adpcmInfo.typeId = 0;
		info->adpcmInfo.offset = 0xFFFFFFFF;
	}

	CWAVDataBlock* dataBlock = (CWAVDataBlock*) &output[headerSize + infoSize];
	memcpy(dataBlock->header.magic, CWAV_BLOCK_MAGIC_DATA, sizeof(dataBlock->header.magic));
	dataBlock->header.size = dataSize;

	for(u32 i = 0; i < cwav.data.size(); i += cwav.channels * (cwav.bitsPerSample / 8)) {
		for(u32 c = 0; c < cwav.channels; c++) {
			CWAVChannelInfo* info = (CWAVChannelInfo*) &output[headerSize + sizeof(CWAVInfoBlockHeader) +
			                        (cwav.channels * sizeof(CWAVReference)) + (c * sizeof(CWAVChannelInfo))];

			memcpy(&dataBlock->data[info->samples.offset + (i / cwav.channels)],
			       &cwav.data[i + (c * (cwav.bitsPerSample / 8))], cwav.bitsPerSample / 8);
		}
	}

	return output;
}
