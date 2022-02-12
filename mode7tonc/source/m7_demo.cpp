//
// m7_demo.c
// block, sawtooth and smooth mode7 in one demo :)
// Using pre-calculated LUTs, courtesy of my excellut program
// 
//
// Coordinate system:
//
//    y
//     |
//     |___x
//  z /
//
// The viewing direction is in the NEGATIVE z direction!!
//
// (20040104 - 20070808, Cearn)

#include <stdio.h>
#include <tonc.h>

#include "m7_demo.h"
#include "nums.h"

#include <Color.h>
#include <Device.h>
#include <Display.h>
#include <Keypad.h>
#include <linearMath.h>
#include <tiles.h>

// === CONSTANTS & MACROS =============================================
//constexpr uint32_t MAP_AFF_SIZE = 0x0100;

static const VECTOR cam_pos_default= { 256<<8, 32<<8, 256<<8 };

// === GLOBALS ========================================================

VECTOR cam_pos;
u16 cam_phi= 0;

FIXED g_cosf= 1<<8, g_sinf= 0;	// temporaries for cos and sin cam_phi

// === FUNCTIONS ======================================================

void init_main()
{
	cam_pos= cam_pos_default;

	GRIT_CPY(&tile8_mem[0][1], numsTiles);
	GRIT_CPY(pal_bg_mem, numsPal);
		
	// Fill the map with a band pattern
	for(int i=0; i<16; i++)
		memset32(&se_mem[8][i*16*16/2], quad8(i+1), 16*16/4);

	REG_BG2CNT= BG_CBB(0) | BG_SBB(8) | BG_AFF_64x64;
}

void initBackground()
{
	// Prepare the background tile map
	BackgroundPalette()[1].raw = BasicColor::White.raw;
	BackgroundPalette()[2].raw = BasicColor::Red.raw;

	// Config BG2
	// Use charblock 0 for the tiles
	// Use the first screen block after charblock 0 (i.e. screenblock 8)
	// 128*128 map size
	IO::BG2CNT::Get().value =
		(1<<7) | // 16 bit color
		(8<<8) | // screenblock 8
		(2<<0xe); // size 256x256

	// Fill in a couple tiles in video memory
    auto& tile0 = Sprite::DTileBlock(0)[0];
	tile0.fill(1); // White
	auto& tile1 = Sprite::DTileBlock(0)[1];
	tile1.fill(2); // Red

	// Fill in map data
	// Affine maps use 8 bit indices
	auto* mapMem = reinterpret_cast<volatile uint16_t*>(VideoMemAddress+0x4000);
	for(int32_t y = 0; y < 64; ++y)
	{
		for(int32_t x = 0; x < 32; ++x)
		{
			mapMem[y*32+x] = (y&1) ? 1 : (1<<8);
		}
	}
}

void updateCamera()
{
	const FIXED speed= 2, DY= 64;
	VECTOR dir;

	// left/right : strafe
	dir.x= speed*(Keypad::Held(Keypad::R) - Keypad::Held(Keypad::L));
	// B/A : rise/sink
	dir.y= DY*(Keypad::Held(Keypad::B) - Keypad::Held(Keypad::A));
	// up/down : forward/back
	dir.z= speed*(Keypad::Held(Keypad::DOWN) - Keypad::Held(Keypad::UP));

	cam_pos.x += dir.x*g_cosf - dir.z*g_sinf;
	cam_pos.y += dir.y;
	cam_pos.z += dir.x*g_sinf + dir.z*g_cosf;

	if(cam_pos.y<0)
		cam_pos.y= 0;

	cam_phi += 128*(Keypad::Held(Keypad::RIGHT) - Keypad::Held(Keypad::LEFT));

	g_cosf= lu_cos(cam_phi)>>4;
	g_sinf= lu_sin(cam_phi)>>4;
}

int main()
{
	cam_pos= cam_pos_default;
	initBackground();

	tte_init_chr4c_b4_default(0, BG_CBB(2)|BG_SBB(28));
	tte_init_con();
	tte_set_margins(8, 8, 232, 40);

	// enable hblank register and set the mode7 type
	irq_init(NULL);
	irq_add(II_HBLANK, (fnptr)m7_hbl_c);
	// and vblank int for vsync
	irq_add(II_VBLANK, NULL);

	REG_DISPCNT= DCNT_MODE1 | DCNT_BG0 | DCNT_BG2;

	// main loop
	while(1)
	{
		VBlankIntrWait();
		updateCamera();
		
		setBg2AffineTx(0); // Prepare first scanline for next frame
	}
	return 0;
}
