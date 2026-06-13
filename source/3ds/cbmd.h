#ifndef CBMD_H
#define CBMD_H

#include "types.h"

#define CBMD_NUM_CGFXS 14

enum CBMDCGFX {
	CGFX_COMMON = 0,
	CGFX_EUR_ENGLISH,
	CGFX_EUR_FRENCH,
	CGFX_EUR_GERMAN,
	CGFX_EUR_ITALIAN,
	CGFX_EUR_SPANISH,
	CGFX_EUR_DUTCH,
	CGFX_EUR_PORTUGESE,
	CGFX_EUR_RUSSIAN,
	CGFX_JPN_JAPANESE,
	CGFX_USA_ENGLISH,
	CGFX_USA_FRENCH,
	CGFX_USA_SPANISH,
	CGFX_USA_PORTUGESE
};

struct CBMD {
	DatVec cgfxs[CBMD_NUM_CGFXS];
	DatVec cwav;
};

#define CBMD_MAGIC "CBMD"

typedef struct {
	char magic[4];
	u32 zero;
	u32 cgfxOffsets[CBMD_NUM_CGFXS];
	u8 padding[0x44];
	u32 cwavOffset;
} CBMDHeader;

DatVec cbmd_build(CBMD &cbmd);
DatVec bnr_build(CBMD &cbmd);

#endif
