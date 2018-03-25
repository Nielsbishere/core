#pragma once

#include "types/generic.h"
#include "types/vector.h"

namespace oi {

	class Random {

	public:

		//Returns 0->u32_max
		static u32 randU32();

		//Returns beg->end (returns beg if beg = end and swaps end with beg if the one is bigger)
		//Includes beg and end
		static u32 randInt(u32 beg = u32_MIN, u32 end = u32_MAX);

		static f32 randFloat(f32 beg = 0.f, f32 end = 1.f);

		//Randomize a number of floats
		static void randomizeFloat(f32 *arr, u32 size, f32 beg = 0.f, f32 end = 1.f);

		//Randomize a number of integers
		static void randomizeInt(u32 *arr, u32 size, u32 beg = u32_MIN, u32 end = u32_MAX);

	protected:

		static u32 off;
	};

}