#ifndef CWAV_H
#define CWAV_H

#include "types.h"

#define CWAV_MAGIC "CWAV"

enum {
	CWAV_ENDIANNESS_LITTLE = 0xFEFF,
	CWAV_ENDIANNESS_BIG = 0xFFFE
};

#define CWAV_VERSION 0x02010000

enum {
	CWAV_REF_DSP_ADPCM_INFO = 0x0300,
	CWAV_REF_IMA_ADPCM_INFO = 0x0301,
	CWAV_REF_SAMPLE_DATA = 0x1F00,
	CWAV_REF_INFO_BLOCK = 0x7000,
	CWAV_REF_DATA_BLOCK = 0x7001,
	CWAV_REF_CHANNEL_INFO = 0x7100
};

typedef struct {
	u16 typeId;
	u16 padding;
	u32 offset;
} CWAVReference;

typedef struct {
	CWAVReference ref;
	u32 size;
} CWAVSizedReference;

typedef struct {
	u32 count;
	CWAVReference contents[0]; // Relative to beginning of CWAVReferenceTable.
} CWAVReferenceTable;

typedef struct {
	char magic[4];
	u16 endianness;
	u16 headerSize;
	u32 version;
	u32 fileSize;
	u16 numBlocks;
	u16 reserved;
	CWAVSizedReference infoBlock; // Relative to start of file.
	CWAVSizedReference dataBlock; // Relative to start of file.
} CWAVHeader;

#define CWAV_BLOCK_MAGIC_INFO "INFO"
#define CWAV_BLOCK_MAGIC_DATA "DATA"

typedef struct {
	char magic[4];
	u32 size;
} CWAVBlockHeader;

enum {
	CWAV_ENCODING_PCM8 = 0,
	CWAV_ENCODING_PCM16,
	CWAV_ENCODING_DSP_ADPCM,
	CWAV_ENCODING_IMA_ADPCM
};

typedef struct {
	CWAVBlockHeader header;
	u8 encoding;
	bool loop;
	u16 padding;
	u32 sampleRate;
	u32 loopStartFrame;
	u32 loopEndFrame;
	u32 reserved;
	CWAVReferenceTable channelInfos;
} CWAVInfoBlockHeader;

typedef struct {
	CWAVReference samples; // Relative to CWAVDataBlock.data
	CWAVReference adpcmInfo; // Relative to beginning of CWAVChannelInfo.
	u32 reserved;
} CWAVChannelInfo;

typedef struct {
	u16 coefficients[16];
} CWAVDSPADPCMParam;

typedef struct {
	u8 predictorScale;
	u8 reserved;
	u16 previousSample;
	u16 secondPreviousSample;
} CWAVDSPADPCMContext;

typedef struct {
	CWAVDSPADPCMParam param;
	CWAVDSPADPCMContext context;
	CWAVDSPADPCMContext loopContext;
	u16 padding;
} CWAVDSPADPCMInfo;

typedef struct {
	u16 data;
	u8 tableIndex;
	u8 padding;
} CWAVIMAADPCMContext;

typedef struct {
	CWAVIMAADPCMContext context;
	CWAVIMAADPCMContext loopContext;
} CWAVIMAADPCMInfo;

typedef struct {
	CWAVBlockHeader header;
	u8 data[0];
} CWAVDataBlock;

struct CWAV {
	CWAV() : channels(0), sampleRate(0), bitsPerSample(0),
		loop(false), loopStartFrame(0), loopEndFrame(0) {}

	u32 channels;
	u32 sampleRate;
	u32 bitsPerSample;

	bool loop;
	u32 loopStartFrame;
	u32 loopEndFrame;

	DatVec data;
};

DatVec cwav_build(CWAV &wav);

#endif
