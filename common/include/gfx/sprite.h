#pragma once
#include <Device.h>

struct Sprite
{
	enum Shape
	{
		Square = 0,
		Wide = 1,
		Tall = 2
	};

	struct alignas(4) Object
	{
		Object() = default;
		Object(math::Vec2i pos, uint32_t shape, uint32_t size);

		volatile uint16_t attribute[4];

		void setPos(uint32_t x, uint32_t y)
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
		static volatile Object* alloc(uint32_t n)
		{
			if(sNext+n > kCapacity)
			{
				return uint32_t(-1); // Out of memory.
			}
			auto pos = sNext;
			sNext += n;
			return pos;
		}

		static inline uint32_t sNext = {};
		static constexpr uint32_t kCapacity = 128;
	};
};