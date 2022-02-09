#pragma once

#include <cstddef>
#include <cstdint>
#include "vector.h"

namespace IO
{
    // IO Memory registers
    template<class T, uint32_t _address>
    struct IORegister
    {
        static constexpr uint32_t address = _address;
        static auto& Get() { return *reinterpret_cast<IORegister*>(address); }

		template<uint16_t x>
		static void Set()
		{
			Get().value = x;
		}

        template<size_t n>
        void setBit()
        {
            static_assert(n < 8*sizeof(T));
            value |= (1<<n);
        }
        
        template<size_t n>
        void clearBit()
        {
            static_assert(n < 8*sizeof(T));
            value &= ~(1<<n);
        }

        volatile T value;
    };

    struct AffineTransform2D
    {
        int16_t A, B, C, D;
    };

    template<uint32_t _address>
    struct AffineTxRegister
    {
        static constexpr uint32_t address = _address;
        static auto& Get() { return *reinterpret_cast<AffineTxRegister*>(address); }

        volatile AffineTransform2D tx;
        volatile math::Vec2p8 refPoint;
    };

    // IO Memory map
    // Reference: https://problemkaputt.de/gbatek.htm#gbaiomap
    using DISPCNT   = IORegister<uint16_t, 0x4000000>;
    using GREENSWAP = IORegister<uint16_t, 0x4000002>;
    using DISPSTAT  = IORegister<uint16_t, 0x4000004>;
    using VCOUNT    = IORegister<uint16_t, 0x4000006>;
    using BG0CNT    = IORegister<uint16_t, 0x4000008>;
    using BG1CNT    = IORegister<uint16_t, 0x400000A>;
    using BG2CNT    = IORegister<uint16_t, 0x400000C>;
    using BG3CNT    = IORegister<uint16_t, 0x400000E>;
    using BG0HOFS   = IORegister<uint16_t, 0x4000010>;
    using BG0VOFS   = IORegister<uint16_t, 0x4000012>;
    using BG1HOFS   = IORegister<uint16_t, 0x4000014>;
    using BG1VOFS   = IORegister<uint16_t, 0x4000016>;
    using BG2HOFS   = IORegister<uint16_t, 0x4000018>;
    using BG2VOFS   = IORegister<uint16_t, 0x400001A>;
    using BG3HOFS   = IORegister<uint16_t, 0x400001C>;
    using BG3VOFS   = IORegister<uint16_t, 0x400001E>;
    using BG2P      = AffineTxRegister<0x4000020>;
    using BG3P      = AffineTxRegister<0x4000030>;

	// DMA
	using DMA0SAD = IORegister<uint32_t, 0x40000B0>; // Source address
	using DMA0DAD = IORegister<uint32_t, 0x40000B4>; // Destination address
	using DMA0CNT_L = IORegister<uint16_t, 0x40000B8>;
	using DMA0CNT_H = IORegister<uint16_t, 0x40000BA>;
	using DMA1SAD = IORegister<uint32_t, 0x40000BC>; // Source address
	using DMA1DAD = IORegister<uint32_t, 0x40000C0>; // Destination address
	using DMA1CNT_L = IORegister<uint16_t, 0x40000C4>;
	using DMA1CNT_H = IORegister<uint16_t, 0x40000C6>;
	using DMA2SAD = IORegister<uint32_t, 0x40000C8>; // Source address
	using DMA2DAD = IORegister<uint32_t, 0x40000CC>; // Destination address
	using DMA2CNT_L = IORegister<uint16_t, 0x40000D0>;
	using DMA2CNT_H = IORegister<uint16_t, 0x40000D2>;
	using DMA3SAD = IORegister<uint32_t, 0x40000D4>; // Source address
	using DMA3DAD = IORegister<uint32_t, 0x40000D8>; // Destination address
	using DMA3CNT_L = IORegister<uint16_t, 0x40000DC>;
	using DMA3CNT_H = IORegister<uint16_t, 0x40000DE>;

    // Keypad
    using KEYINPUT  = IORegister<uint16_t, 0x4000130>;
    using KEYCNT    = IORegister<uint16_t, 0x4000132>;

    // Interrupts
    using IE        = IORegister<uint16_t, 0x4000200>;
    using IF        = IORegister<uint16_t, 0x4000202>;
    using WAITCNT   = IORegister<uint16_t, 0x4000204>;
    using IME       = IORegister<uint16_t, 0x4000208>;

}   // namespace IO

namespace DMA
{
	struct Channel
	{
		volatile uint32_t srcAddress;
		volatile uint32_t dstAddress;
		volatile uint16_t wordCount;
		volatile uint16_t control;
	};

	inline Channel& Channel0() {
		return *reinterpret_cast<Channel*>(IO::DMA0SAD::address);
	}

	inline Channel& Channel1() {
		return *reinterpret_cast<Channel*>(IO::DMA1SAD::address);
	}

	inline Channel& Channel2() {
		return *reinterpret_cast<Channel*>(IO::DMA2SAD::address);
	}

	inline Channel& Channel3() {
		return *reinterpret_cast<Channel*>(IO::DMA3SAD::address);
	}
}

static constexpr uint32_t PaletteMemAddress = 0x05000000;
static constexpr uint32_t PaletteMemSize = 0x400;
static constexpr uint32_t VideoMemAddress = 0x06000000;
static constexpr uint32_t VideoMemSize = 0x18000;
static constexpr uint32_t OAMAddress = 0x07000000;
static constexpr uint32_t OAMSize = 0x400;