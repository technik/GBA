#pragma once

#include <cstddef>
#include <cstdint>
#include "vector.h"

namespace IO
{
    // IO Memory registers
    template<class T, uint32_t _address>
    class IORegister
    {
	public:
		IORegister() = delete;
		~IORegister() = delete;

        static constexpr std::size_t address = _address;
        static auto& Get() { return *reinterpret_cast<IORegister*>(address); }
		
        static auto& Value() { return Get().value; }

        volatile T value;
    };

    template<uint32_t _address>
    struct AffineTxRegister
    {
        static constexpr uint32_t address = _address;
        static auto& Get() { return *reinterpret_cast<AffineTxRegister*>(address); }

        volatile int16_t A, B, C, D;
        volatile math::Vec2p8 refPoint;
    };

    // IO Memory map
    // Reference: https://problemkaputt.de/gbatek.htm#gbaiomap

	class DISPCNT : public IORegister<uint16_t, 0x4000000>
	{
		~DISPCNT() = delete; // Get rid of warning C4624
	};

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
		volatile uint32_t srcAddress; // 28 bits
		volatile uint32_t dstAddress; // 27 bits
		volatile uint16_t wordCount;
		volatile uint16_t control;

		static constexpr uint16_t dstAdjustShift = 5;
		enum class DstAddrAdjust : uint16_t
		{
			Inc = 0 << dstAdjustShift,
			Dec = 1 << dstAdjustShift,
			Fixed = 2 << dstAdjustShift,
			Reload = 3<< dstAdjustShift
		};

		static constexpr uint16_t srcAdjustShift = 7;
		enum class SrcAddrAdjust : uint16_t
		{
			Inc = 0<<srcAdjustShift,
			Dec = 1<<srcAdjustShift,
			Fixed = 2<<srcAdjustShift
		};

		enum class ChunkSize : uint16_t
		{
			Dma16Bit = 0,
			Dma32Bit = (1<<10)
		};

		enum class TimingMode : uint16_t
		{
			Now = 0,
			VBlank = 1<<12,
			HBlank = 2<<12,
			// Used with channel3 only.
			// DMA will start at the next screen refresh, delayed by two
			// scanlines, so it can safely be used to copy full backgrounds to VRAM
			Refresh = 3<<12
		};

		enum class RepeatMode : uint16_t
		{
			Off = 0,
			On = 1<<9
		};

		enum class IRQDispatch : uint16_t
		{
			Off = 0,
			On = 1<<14
		};

		static constexpr uint16_t DmaEnable = 1<<15;

		// Cancels any pending transfer on this channel
		void clear()
		{
#ifndef _WIN32
			control = 0; // Clear enable bit
#endif // _WIN32
		}

		// Note count is in the number of uint16_t chunks to fill
		void Fill(uint16_t* dst, const volatile uint16_t src, uint32_t count)
		{
#ifdef _WIN32
			for (int i = 0; i < count; ++i)
			{
				dst[i] = src;
			}
#else
			// Stop any previous DMA transfers
			clear();

			// Set start and end destinations
			srcAddress = reinterpret_cast<uint32_t>(&src);
			dstAddress = reinterpret_cast<uint32_t>(dst);
			wordCount = count;

			// Config and dispatch the copy
			control = uint16_t(ChunkSize::Dma16Bit) | uint16_t(SrcAddrAdjust::Fixed) | uint16_t(TimingMode::Now) | DmaEnable;
#endif
		}

		// Note count is in the number of uint32_t chunks to fill
		void Fill(uint32_t* dst, const volatile uint32_t src, uint32_t count)
		{
#ifdef _WIN32
			for (int i = 0; i < count; ++i)
			{
				dst[i] = src;
			}
#else
			// Stop any previous DMA transfers
			clear();

			// Set start and end destinations
			srcAddress = reinterpret_cast<uint32_t>(&src);
			dstAddress = reinterpret_cast<uint32_t>(dst);
			wordCount = count;

			// Config and dispatch the copy
			control = uint16_t(ChunkSize::Dma32Bit) | uint16_t(SrcAddrAdjust::Fixed) | uint16_t(TimingMode::Now) | DmaEnable;
#endif
		}

		// Note count is in the number of uint32_t chunks to copy
		void Copy(uint32_t* dst, const uint32_t* src, uint32_t count)
		{
#ifdef _WIN32
			for (int i = 0; i < count; ++i)
			{
				dst[i] = src[i];
			}
#else
			// Stop any previous DMA transfers
			clear();
#pragma
			// Set start and end destinations
			srcAddress = reinterpret_cast<uint32_t>(src);
			dstAddress = reinterpret_cast<uint32_t>(dst);
			wordCount = count;

			// Config and dispatch the copy
			control = uint16_t(ChunkSize::Dma32Bit) | uint16_t(TimingMode::Now) | DmaEnable;
#endif
		}

		// Note count is in the number of uint16_t chunks to copy
		void Copy(uint16_t* dst, const uint16_t* src, uint32_t count)
		{
#ifdef _WIN32
			for (int i = 0; i < count; ++i)
			{
				dst[i] = src[i];
			}
#else
			// Stop any previous DMA transfers
			clear();

			// Set start and end destinations
			srcAddress = reinterpret_cast<uint32_t>(src);
			dstAddress = reinterpret_cast<uint32_t>(dst);
			wordCount = count;

			// Config and dispatch the copy
			control = uint16_t(ChunkSize::Dma16Bit) | uint16_t(TimingMode::Now) | DmaEnable;
#endif
		}
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
static constexpr uint32_t PaletteMemSize = 0x200;
static constexpr uint32_t VideoMemAddress = 0x06000000;
static constexpr uint32_t VideoMemSize = 0x18000;
static constexpr uint32_t OAMAddress = 0x07000000;
static constexpr uint32_t OAMSize = 0x400;