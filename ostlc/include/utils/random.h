#pragma once

#include "types/generic.h"
#include "types/vector.h"

namespace oi {

	class Random {

	public:

		//Set seed
		static void setSeed(u32 seed);

		//Set random
		static void seedRandom();

		//Get seed
		static u32 getSeed();

		//Get offset to seed
		static u32 getOffset();

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

		template<u32 n>
		static TVec<f32, n> randomize(f32 beg, f32 end);

	protected:

		static u32 off, seed;

	};

	template<u32 n>
	TVec<f32, n> Random::randomize(f32 beg, f32 end) {

		TVec<f32, n> result;

		for (u32 i = 0; i < n; ++i)
			result[i] = randFloat(beg, end);

		return result;
	}

}