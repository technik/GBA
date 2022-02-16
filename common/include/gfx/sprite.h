#pragma once
#include <Device.h>

struct Sprite
{

	enum class ObjectMode : uint16_t
	{
		Normal = 0,
		Affine = 1,
		Disabled = 2,
		Affine2x = 3
	};

	enum class GfxMode : uint16_t
	{
		Normal = 0,
		AlphaBlend = 1,
		Window = 2,
	};

	enum class ColorMode : uint16_t
	{
		e4bits,
		e16bits
	};

	// Actual shape goes in the bits 0:1.
	// Size goes in bits 2:3.
	enum class Shape : uint16_t
	{
		square8x8 = 0,
		wide16x8 = 1,
		tall8x16 = 2,
		square16x16 = 4,
		wide32x8 = 5,
		tall8x32 = 6,
		square32x32 = 8,
		wide32x16 = 9,
		tall16x32 = 10,
		square64x64 = 12,
		wide64x32 = 13,
		tall32x64 = 14
	};

	struct alignas(4) Object
	{
		Object() = default;
		Object(math::Vec2i pos, uint32_t shape, uint32_t size);

		uint16_t attribute[3];

		inline void Configure(ObjectMode objectMode, GfxMode gfxMode, ColorMode colorMode, Shape shape)
		{
			attribute[0] =
				(attribute[0] & 0x00ff) | // preserve y pos
				((uint8_t)objectMode << 8) |
				((uint8_t)gfxMode << 10) |
				((uint8_t)colorMode << 13) |
				((uint8_t)shape << 14); // Lowest two bits store the shape
		}

		inline void SetAffineConfig(uint32_t affineIndex)
		{
			attribute[1] =
				(attribute[1] & 0x01ff) | // preserve x pos
				((affineIndex&0x1f)<<9);
		}

		inline void SetNonAffineTransform(bool horizontalFlip, bool verticalFlip, Shape size)
		{
			attribute[1] =
				(attribute[1] & 0x01ff) | // preserve x pos
				(horizontalFlip ? (1<<12) : 0) |
				(verticalFlip ? (1<<13) : 0) |
				((uint8_t(size)>>2)<<14);
		}

		inline void setPos(uint32_t x, uint32_t y)
		{
			attribute[0] = (attribute[0] & (0xff00)) | (y&0xff);
			attribute[1] = (attribute[1] & (0xff00)) | (x&0xff);
		}
		
		inline void setPos(uint32_t x, uint32_t y) volatile
		{
			attribute[0] = (attribute[0] & (0xff00)) | (y&0xff);
			attribute[1] = (attribute[1] & (0xff00)) | (x&0xff);
		}
	};

	struct alignas(4) Transform
	{
		uint16_t fill0[3];
		volatile int16_t pa;
		uint16_t fill1[3];
		volatile int16_t pb;
		uint16_t fill2[3];
		volatile int16_t pc;
		uint16_t fill3[3];
		volatile int16_t pd;
	};

	union Block
	{
		Object objects[4];
		Transform transform;
	};

	// Memory aliases for OAM access
	static Block* OAM_Blocks()
	{
		return reinterpret_cast<Block*>(OAMAddress);
	}

	static Transform* OAM_Transforms()
	{
		return reinterpret_cast<Transform*>(OAMAddress);
	}

	static Object* OAM_Objects()
	{
		return reinterpret_cast<Object*>(OAMAddress);
	}

	struct ObjectAllocator
	{
		static void reset()
		{
			sNext = 0;
		}

		// Allocate small tiles where each dot is only 4 bits.
		// Should return an object pointer
		static Object* alloc(uint32_t n)
		{
			if(sNext+n > kCapacity)
			{
				return nullptr; // Out of memory.
			}
			auto pos = sNext;
			sNext += n;
			return &reinterpret_cast<Object*>(OAMAddress)[pos];
		}

		static inline uint32_t sNext = 0;
		static constexpr uint32_t kCapacity = 128;
	};
};