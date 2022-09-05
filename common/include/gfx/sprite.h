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

	inline static constexpr size_t GetNumTiles(Shape shape)
	{
		constexpr size_t sizeTable[15] = {
			1, 2, 2, 0,
			4, 4, 4, 0,
			16, 8, 8, 0,
			64, 32, 32
		};

		return sizeTable[(size_t)shape];
	}

	class alignas(4) Object
	{
	public:
		Object() = default;
		Object(math::Vec2i pos, uint32_t shape, uint32_t size);
		void operator=(const Object& other) volatile
		{
			attribute[0] = other.attribute[0];
			attribute[1] = other.attribute[1];
			attribute[2] = other.attribute[2];
		}

		inline void Configure(ObjectMode objectMode, GfxMode gfxMode, ColorMode colorMode, Shape shape) volatile
		{
			attribute[0] =
				(attribute[0] & 0x00ff) | // preserve y pos
				(((uint16_t)objectMode) << 8) |
				(((uint16_t)gfxMode) << 10) |
				(((uint16_t)colorMode) << 13) |
				(((uint16_t)shape) << 14); // Lowest two bits store the shape
		}

		inline void SetAffineConfig(uint32_t affineIndex, Shape size) volatile
		{
			attribute[1] =
				(attribute[1] & 0x01ff) | // preserve x pos
				((affineIndex&0x1f)<<9) | // Select affine transform
				((uint16_t(size)>>2)<<14); // Select size mode
		}

		inline void SetNonAffineTransform(bool horizontalFlip, bool verticalFlip, Shape size) volatile
		{
			attribute[1] =
				(attribute[1] & 0x01ff) | // preserve x pos
				(horizontalFlip ? (1<<12) : 0) |
				(verticalFlip ? (1<<13) : 0) |
				((uint16_t(size)>>2)<<14);
		}

		inline void setTiles(uint32_t firstTile, uint32_t subPalette = 0, uint32_t priority = 0) volatile
		{
			attribute[2] = 
				(firstTile&0x3ff) | // Bits 0-9, tile number (0-1023)
				(priority&3)<<10 | // Priority relative to BG
				(subPalette&0x0f)<<12; // Palette index
		}
		
		inline void setPos(int32_t x, int32_t y) volatile
		{
			attribute[0] = (attribute[0] & (0xff00)) | (y&0xff);
			attribute[1] = (attribute[1] & (0xfe00)) | (x&0x1ff);
		}

		inline void show(ObjectMode objectMode) volatile
		{
			constexpr uint8_t visibilityMask = uint8_t(~(0x03 << 8));
			attribute[0] = (attribute[0] & visibilityMask) | (((uint16_t)objectMode) << 8);
		}

		inline void hide() volatile
		{
			constexpr uint8_t visibilityMask = uint8_t(~(0x03 << 8));
			attribute[0] = (attribute[0] & visibilityMask) | (((uint16_t)ObjectMode::Disabled) << 8);
		}

	private:
		uint16_t attribute[3];
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

		// Allocate Sprite Objects in OAM memory
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

	struct TransformAllocator
	{
		static void reset()
		{
			sNext = 0;
		}

		// Allocate affine transform blocks in OAM memory.
		// Returns the transform index [0,31], or -1 to
		// signal out of memory
		static int32_t alloc(uint32_t n)
		{
			if(sNext+n > kCapacity)
			{
				return -1; // Out of memory.
			}
			auto pos = sNext;
			sNext += n;
			return pos;
		}

		static inline uint32_t sNext = 0;
		static constexpr uint32_t kCapacity = 32;
	};
};