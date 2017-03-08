/*
*	Name:		Low-level OPL2/OPL3 I/O interface
*	Project:	MUS File Player Library
*	Version:	1.64
*	Author:		Vladimir Arnost (QA-Software)
*	Last revision:	Mar-1-1996
*	Compiler:	Borland C++ 3.1, Watcom C/C++ 10.0
*
*/

/*
* Revision History:
*
*	Aug-8-1994	V1.00	V.Arnost
*		Written from scratch
*	Aug-9-1994	V1.10	V.Arnost
*		Added stereo capabilities
*	Aug-13-1994	V1.20	V.Arnost
*		Stereo capabilities made functional
*	Aug-24-1994	V1.30	V.Arnost
*		Added Adlib and SB Pro II detection
*	Oct-30-1994	V1.40	V.Arnost
*		Added BLASTER variable parsing
*	Apr-14-1995	V1.50	V.Arnost
*              Some declarations moved from adlib.h to doomtype.h
*	Jul-22-1995	V1.60	V.Arnost
*		Ported to Watcom C
*		Simplified WriteChannel() and WriteValue()
*	Jul-24-1995	V1.61	V.Arnost
*		DetectBlaster() moved to MLMISC.C
*	Aug-8-1995	V1.62	V.Arnost
*		Module renamed to MLOPL_IO.C and functions renamed to OPLxxx
*		Mixer-related functions moved to module MLSBMIX.C
*	Sep-8-1995	V1.63	V.Arnost
*		OPLwriteReg() routine sped up on OPL3 cards
*	Mar-1-1996	V1.64	V.Arnost
*		Cleaned up the source
*/

#include <math.h>
#ifdef _WIN32
#include <dos.h>
#include <conio.h>
#endif
#include "muslib.h"
#include "opl.h"
#include "c_cvars.h"

const double HALF_PI = (M_PI*0.5);

EXTERN_CVAR(Int, opl_core)
extern int current_opl_core;

OPLio::~OPLio()
{
}

void OPLio::SetClockRate(double samples_per_tick)
{
}

void OPLio::WriteDelay(int ticks)
{
}

void OPLio::OPLwriteReg(int which, uint32_t reg, uint8_t data)
{
	if (IsOPL3)
	{
		reg |= (which & 1) << 8;
		which >>= 1;
	}
	if (chips[which] != NULL)
	{
		chips[which]->WriteReg(reg, data);
	}
}

/*
* Write to an operator pair. To be used for register bases of 0x20, 0x40,
* 0x60, 0x80 and 0xE0.
*/
void OPLio::OPLwriteChannel(uint32_t regbase, uint32_t channel, uint8_t data1, uint8_t data2)
{
	static const uint32_t op_num[OPL2CHANNELS] = {
		0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12};

	uint32_t which = channel / OPL2CHANNELS;
	uint32_t reg = regbase + op_num[channel % OPL2CHANNELS];
	OPLwriteReg (which, reg, data1);
	OPLwriteReg (which, reg+3, data2);
}

/*
* Write to channel a single value. To be used for register bases of
* 0xA0, 0xB0 and 0xC0.
*/
void OPLio::OPLwriteValue(uint32_t regbase, uint32_t channel, uint8_t value)
{
	uint32_t which = channel / OPL2CHANNELS;
	uint32_t reg = regbase + (channel % OPL2CHANNELS);
	OPLwriteReg (which, reg, value);
}

static WORD frequencies[] =
{
	0x133, 0x133, 0x134, 0x134, 0x135, 0x136, 0x136, 0x137, 0x137, 0x138, 0x138, 0x139,
	0x139, 0x13a, 0x13b, 0x13b, 0x13c, 0x13c, 0x13d, 0x13d, 0x13e, 0x13f, 0x13f, 0x140,
	0x140, 0x141, 0x142, 0x142, 0x143, 0x143, 0x144, 0x144, 0x145, 0x146, 0x146, 0x147,
	0x147, 0x148, 0x149, 0x149, 0x14a, 0x14a, 0x14b, 0x14c, 0x14c, 0x14d, 0x14d, 0x14e,
	0x14f, 0x14f, 0x150, 0x150, 0x151, 0x152, 0x152, 0x153, 0x153, 0x154, 0x155, 0x155,
	0x156, 0x157, 0x157, 0x158, 0x158, 0x159, 0x15a, 0x15a, 0x15b, 0x15b, 0x15c, 0x15d,
	0x15d, 0x15e, 0x15f, 0x15f, 0x160, 0x161, 0x161, 0x162, 0x162, 0x163, 0x164, 0x164,
	0x165, 0x166, 0x166, 0x167, 0x168, 0x168, 0x169, 0x16a, 0x16a, 0x16b, 0x16c, 0x16c,
	0x16d, 0x16e, 0x16e, 0x16f, 0x170, 0x170, 0x171, 0x172, 0x172, 0x173, 0x174, 0x174,
	0x175, 0x176, 0x176, 0x177, 0x178, 0x178, 0x179, 0x17a, 0x17a, 0x17b, 0x17c, 0x17c,
	0x17d, 0x17e, 0x17e, 0x17f, 0x180, 0x181, 0x181, 0x182, 0x183, 0x183, 0x184, 0x185,
	0x185, 0x186, 0x187, 0x188, 0x188, 0x189, 0x18a, 0x18a, 0x18b, 0x18c, 0x18d, 0x18d,
	0x18e, 0x18f, 0x18f, 0x190, 0x191, 0x192, 0x192, 0x193, 0x194, 0x194, 0x195, 0x196,
	0x197, 0x197, 0x198, 0x199, 0x19a, 0x19a, 0x19b, 0x19c, 0x19d, 0x19d, 0x19e, 0x19f,
	0x1a0, 0x1a0, 0x1a1, 0x1a2, 0x1a3, 0x1a3, 0x1a4, 0x1a5, 0x1a6, 0x1a6, 0x1a7, 0x1a8,
	0x1a9, 0x1a9, 0x1aa, 0x1ab, 0x1ac, 0x1ad, 0x1ad, 0x1ae, 0x1af, 0x1b0, 0x1b0, 0x1b1,
	0x1b2, 0x1b3, 0x1b4, 0x1b4, 0x1b5, 0x1b6, 0x1b7, 0x1b8, 0x1b8, 0x1b9, 0x1ba, 0x1bb,
	0x1bc, 0x1bc, 0x1bd, 0x1be, 0x1bf, 0x1c0, 0x1c0, 0x1c1, 0x1c2, 0x1c3, 0x1c4, 0x1c4,
	0x1c5, 0x1c6, 0x1c7, 0x1c8, 0x1c9, 0x1c9, 0x1ca, 0x1cb, 0x1cc, 0x1cd, 0x1ce, 0x1ce,
	0x1cf, 0x1d0, 0x1d1, 0x1d2, 0x1d3, 0x1d3, 0x1d4, 0x1d5, 0x1d6, 0x1d7, 0x1d8, 0x1d8,
	0x1d9, 0x1da, 0x1db, 0x1dc, 0x1dd, 0x1de, 0x1de, 0x1df, 0x1e0, 0x1e1, 0x1e2, 0x1e3,
	0x1e4, 0x1e5, 0x1e5, 0x1e6, 0x1e7, 0x1e8, 0x1e9, 0x1ea, 0x1eb, 0x1ec, 0x1ed, 0x1ed,
	0x1ee, 0x1ef, 0x1f0, 0x1f1, 0x1f2, 0x1f3, 0x1f4, 0x1f5, 0x1f6, 0x1f6, 0x1f7, 0x1f8,
	0x1f9, 0x1fa, 0x1fb, 0x1fc, 0x1fd, 0x1fe, 0x1ff, 0x200,
	
	0x201, 0x201, 0x202, 0x203, 0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a, 0x20b, 0x20c, 0x20d, 0x20e, 0x20f,
	0x210, 0x210, 0x211, 0x212, 0x213, 0x214, 0x215, 0x216, 0x217, 0x218, 0x219, 0x21a, 0x21b, 0x21c, 0x21d, 0x21e,
	
	0x21f, 0x220, 0x221, 0x222, 0x223, 0x224, 0x225, 0x226, 0x227, 0x228, 0x229, 0x22a, 0x22b, 0x22c, 0x22d, 0x22e,
	0x22f, 0x230, 0x231, 0x232, 0x233, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x23a, 0x23b, 0x23c, 0x23d, 0x23e,
	
	0x23f, 0x240, 0x241, 0x242, 0x244, 0x245, 0x246, 0x247, 0x248, 0x249, 0x24a, 0x24b, 0x24c, 0x24d, 0x24e, 0x24f,
	0x250, 0x251, 0x252, 0x253, 0x254, 0x256, 0x257, 0x258, 0x259, 0x25a, 0x25b, 0x25c, 0x25d, 0x25e, 0x25f, 0x260,

	0x262, 0x263, 0x264, 0x265, 0x266, 0x267, 0x268, 0x269, 0x26a, 0x26c, 0x26d, 0x26e, 0x26f, 0x270, 0x271, 0x272,
	0x273, 0x275, 0x276, 0x277, 0x278, 0x279, 0x27a, 0x27b, 0x27d, 0x27e, 0x27f, 0x280, 0x281, 0x282, 0x284, 0x285,
	
	0x286, 0x287, 0x288, 0x289, 0x28b, 0x28c, 0x28d, 0x28e, 0x28f, 0x290, 0x292, 0x293, 0x294, 0x295, 0x296, 0x298,
	0x299, 0x29a, 0x29b, 0x29c, 0x29e, 0x29f, 0x2a0, 0x2a1, 0x2a2, 0x2a4, 0x2a5, 0x2a6, 0x2a7, 0x2a9, 0x2aa, 0x2ab,
	
	0x2ac, 0x2ae, 0x2af, 0x2b0, 0x2b1, 0x2b2, 0x2b4, 0x2b5, 0x2b6, 0x2b7, 0x2b9, 0x2ba, 0x2bb, 0x2bd, 0x2be, 0x2bf,
	0x2c0, 0x2c2, 0x2c3, 0x2c4, 0x2c5, 0x2c7, 0x2c8, 0x2c9, 0x2cb, 0x2cc, 0x2cd, 0x2ce, 0x2d0, 0x2d1, 0x2d2, 0x2d4,

	0x2d5, 0x2d6, 0x2d8, 0x2d9, 0x2da, 0x2dc, 0x2dd, 0x2de, 0x2e0, 0x2e1, 0x2e2, 0x2e4, 0x2e5, 0x2e6, 0x2e8, 0x2e9,
	0x2ea, 0x2ec, 0x2ed, 0x2ee, 0x2f0, 0x2f1, 0x2f2, 0x2f4, 0x2f5, 0x2f6, 0x2f8, 0x2f9, 0x2fb, 0x2fc, 0x2fd, 0x2ff,
	
	0x300, 0x302, 0x303, 0x304, 0x306, 0x307, 0x309, 0x30a, 0x30b, 0x30d, 0x30e, 0x310, 0x311, 0x312, 0x314, 0x315,
	0x317, 0x318, 0x31a, 0x31b, 0x31c, 0x31e, 0x31f, 0x321, 0x322, 0x324, 0x325, 0x327, 0x328, 0x329, 0x32b, 0x32c,
	
	0x32e, 0x32f, 0x331, 0x332, 0x334, 0x335, 0x337, 0x338, 0x33a, 0x33b, 0x33d, 0x33e, 0x340, 0x341, 0x343, 0x344,
	0x346, 0x347, 0x349, 0x34a, 0x34c, 0x34d, 0x34f, 0x350, 0x352, 0x353, 0x355, 0x357, 0x358, 0x35a, 0x35b, 0x35d,

	0x35e, 0x360, 0x361, 0x363, 0x365, 0x366, 0x368, 0x369, 0x36b, 0x36c, 0x36e, 0x370, 0x371, 0x373, 0x374, 0x376,
	0x378, 0x379, 0x37b, 0x37c, 0x37e, 0x380, 0x381, 0x383, 0x384, 0x386, 0x388, 0x389, 0x38b, 0x38d, 0x38e, 0x390,
	
	0x392, 0x393, 0x395, 0x397, 0x398, 0x39a, 0x39c, 0x39d, 0x39f, 0x3a1, 0x3a2, 0x3a4, 0x3a6, 0x3a7, 0x3a9, 0x3ab,
	0x3ac, 0x3ae, 0x3b0, 0x3b1, 0x3b3, 0x3b5, 0x3b7, 0x3b8, 0x3ba, 0x3bc, 0x3bd, 0x3bf, 0x3c1, 0x3c3, 0x3c4, 0x3c6,
	
	0x3c8, 0x3ca, 0x3cb, 0x3cd, 0x3cf, 0x3d1, 0x3d2, 0x3d4, 0x3d6, 0x3d8, 0x3da, 0x3db, 0x3dd, 0x3df, 0x3e1, 0x3e3,
	0x3e4, 0x3e6, 0x3e8, 0x3ea, 0x3ec, 0x3ed, 0x3ef, 0x3f1, 0x3f3, 0x3f5, 0x3f6, 0x3f8, 0x3fa, 0x3fc, 0x3fe, 0x36c
};

/*
* Write frequency/octave/keyon data to a channel
* [RH] This is totally different from the original MUS library code
* but matches exactly what DMX does. I haven't a clue why there are 284
* special bytes at the beginning of the table for the first few notes.
* That last byte in the table doesn't look right, either, but that's what
* it really is.
*/
void OPLio::OPLwriteFreq(uint32_t channel, uint32_t note, uint32_t pitch, uint32_t keyon)
{
	int octave = 0;
	int j = (note << 5) + pitch;

	if (j < 0)
	{
		j = 0;
	}
	else if (j >= 284)
	{
		j -= 284;
		octave = j / (32*12);
		if (octave > 7)
		{
			octave = 7;
		}
		j = (j % (32*12)) + 284;
	}
	int i = frequencies[j] | (octave << 10);

	OPLwriteValue (0xA0, channel, (uint8_t)i);
	OPLwriteValue (0xB0, channel, (uint8_t)(i>>8)|(keyon<<5));
}

/*
* Adjust volume value (register 0x40)
*/
inline uint32_t OPLio::OPLconvertVolume(uint32_t data, uint32_t volume)
{
	static uint8_t volumetable[128] = {
		0,   1,   3,   5,   6,   8,  10,  11,
		13,  14,  16,  17,  19,  20,  22,  23,
		25,  26,  27,  29,  30,  32,  33,  34,
		36,  37,  39,  41,  43,  45,  47,  49,
		50,  52,  54,  55,  57,  59,  60,  61,
		63,  64,  66,  67,  68,  69,  71,  72,
		73,  74,  75,  76,  77,  79,  80,  81,
		82,  83,  84,  84,  85,  86,  87,  88,
		89,  90,  91,  92,  92,  93,  94,  95,
		96,  96,  97,  98,  99,  99, 100, 101,
		101, 102, 103, 103, 104, 105, 105, 106,
		107, 107, 108, 109, 109, 110, 110, 111,
		112, 112, 113, 113, 114, 114, 115, 115,
		116, 117, 117, 118, 118, 119, 119, 120,
		120, 121, 121, 122, 122, 123, 123, 123,
		124, 124, 125, 125, 126, 126, 127, 127};

	return 0x3F - (((0x3F - data) *
		(uint32_t)volumetable[volume <= 127 ? volume : 127]) >> 7);

}

uint32_t OPLio::OPLpanVolume(uint32_t volume, int pan)
{
	if (pan >= 0)
		return volume;
	else
		return (volume * (pan + 64)) / 64;
}

/*
* Write volume data to a channel
*/
void OPLio::OPLwriteVolume(uint32_t channel, struct OPL2instrument *instr, uint32_t volume)
{
	if (instr != 0)
	{
		OPLwriteChannel(0x40, channel, ((instr->feedback & 1) ?
			OPLconvertVolume(instr->level_1, volume) : instr->level_1) | instr->scale_1,
			OPLconvertVolume(instr->level_2, volume) | instr->scale_2);
	}
}

/*
* Write pan (balance) data to a channel
*/
void OPLio::OPLwritePan(uint32_t channel, struct OPL2instrument *instr, int pan)
{
	if (instr != 0)
	{
		uint8_t bits;
		if (pan < -36) bits = 0x10;		// left
		else if (pan > 36) bits = 0x20;	// right
		else bits = 0x30;			// both

		OPLwriteValue(0xC0, channel, instr->feedback | bits);

		// Set real panning if we're using emulated chips.
		int chanper = IsOPL3 ? OPL3CHANNELS : OPL2CHANNELS;
		int which = channel / chanper;
		if (chips[which] != NULL)
		{
			// This is the MIDI-recommended pan formula. 0 and 1 are
			// both hard left so that 64 can be perfectly center.
			// (Note that the 'pan' passed to this function is the
			// MIDI pan position, subtracted by 64.)
			double level = (pan <= -63) ? 0 : (pan + 64 - 1) / 126.0;
			chips[which]->SetPanning(channel % chanper,
				(float)cos(HALF_PI * level), (float)sin(HALF_PI * level));
		}
	}
}

/*
* Write an instrument to a channel
*
* Instrument layout:
*
*   Operator1  Operator2  Descr.
*    data[0]    data[7]   reg. 0x20 - tremolo/vibrato/sustain/KSR/multi
*    data[1]    data[8]   reg. 0x60 - attack rate/decay rate
*    data[2]    data[9]   reg. 0x80 - sustain level/release rate
*    data[3]    data[10]  reg. 0xE0 - waveform select
*    data[4]    data[11]  reg. 0x40 - key scale level
*    data[5]    data[12]  reg. 0x40 - output level (bottom 6 bits only)
*          data[6]        reg. 0xC0 - feedback/AM-FM (both operators)
*/
void OPLio::OPLwriteInstrument(uint32_t channel, struct OPL2instrument *instr)
{
	OPLwriteChannel(0x40, channel, 0x3F, 0x3F);		// no volume
	OPLwriteChannel(0x20, channel, instr->trem_vibr_1, instr->trem_vibr_2);
	OPLwriteChannel(0x60, channel, instr->att_dec_1,   instr->att_dec_2);
	OPLwriteChannel(0x80, channel, instr->sust_rel_1,  instr->sust_rel_2);
	OPLwriteChannel(0xE0, channel, instr->wave_1,      instr->wave_2);
	OPLwriteValue  (0xC0, channel, instr->feedback | 0x30);
}

/*
* Stop all sounds
*/
void OPLio::OPLshutup(void)
{
	uint32_t i;

	for(i = 0; i < OPLchannels; i++)
	{
		OPLwriteChannel(0x40, i, 0x3F, 0x3F);	// turn off volume
		OPLwriteChannel(0x60, i, 0xFF, 0xFF);	// the fastest attack, decay
		OPLwriteChannel(0x80, i, 0x0F, 0x0F);	// ... and release
		OPLwriteValue(0xB0, i, 0);		// KEY-OFF
	}
}

/*
* Initialize hardware upon startup
*/
int OPLio::OPLinit(uint32_t numchips, bool stereo, bool initopl3)
{
	assert(numchips >= 1 && numchips <= countof(chips));
	uint32_t i;
	IsOPL3 = (current_opl_core == 1 || current_opl_core == 2 || current_opl_core == 3);

	memset(chips, 0, sizeof(chips));
	if (IsOPL3)
	{
		numchips = (numchips + 1) >> 1;
	}
	for (i = 0; i < numchips; ++i)
	{
		OPLEmul *chip = IsOPL3 ? (current_opl_core == 1 ? DBOPLCreate(stereo) : (current_opl_core == 2 ? JavaOPLCreate(stereo) : NukedOPL3Create(stereo))) : YM3812Create(stereo);
		if (chip == NULL)
		{
			break;
		}
		chips[i] = chip;
	}
	NumChips = i;
	OPLchannels = i * (IsOPL3 ? OPL3CHANNELS : OPL2CHANNELS);
	OPLwriteInitState(initopl3);
	return i;
}

void OPLio::OPLwriteInitState(bool initopl3)
{
	for (uint32_t i = 0; i < NumChips; ++i)
	{
		int chip = i << (int)IsOPL3;
		if (IsOPL3 && initopl3)
		{
			OPLwriteReg(chip, 0x105, 0x01);	// enable YMF262/OPL3 mode
			OPLwriteReg(chip, 0x104, 0x00);	// disable 4-operator mode
		}
		OPLwriteReg(chip, 0x01, 0x20);	// enable Waveform Select
		OPLwriteReg(chip, 0x0B, 0x40);	// turn off CSW mode
		OPLwriteReg(chip, 0xBD, 0x00);	// set vibrato/tremolo depth to low, set melodic mode
	}
	OPLshutup();
}

/*
* Deinitialize hardware before shutdown
*/
void OPLio::OPLdeinit(void)
{
	for (size_t i = 0; i < countof(chips); ++i)
	{
		if (chips[i] != NULL)
		{
			delete chips[i];
			chips[i] = NULL;
		}
	}
}
