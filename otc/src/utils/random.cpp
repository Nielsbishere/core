#include "utils/random.h"
#include "utils/timer.h"
using namespace oi;

const u32 RMAX_min = 0x7FFF;
u32 Random::off = 0, Random::seed = 0;

void Random::setSeed(u32 newSeed) {
	off = seed = newSeed;
}

void Random::seedRandom() {
	setSeed(u32(Timer::getGlobalTimer().getTime().time_since_epoch().count() % u32_MAX));
}

u32 Random::getSeed() { return seed; }
u32 Random::getOffset() { return off - seed; }

u32 Random::randU32() {
	u32 a = off;
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	++off;
	return a;
}

u32 Random::randInt(u32 beg, u32 end) {
	if (beg == end) return beg;
	if (beg > end) {
		u32 obeg = beg;
		beg = end;
		end = obeg;
	}
	if (end - beg <= RMAX_min) return randU32() % (end - beg + 1) + beg;
	return randU32() % (end - beg + 1) + beg;
}

f32 Random::randFloat(f32 beg, f32 end) {
	if (beg == end) return beg;
	if (beg > end) {
		f32 obeg = beg;
		beg = end;
		end = obeg;
	}
	f64 ri = (f64)randU32() / u32_MAX;
	return (f32)(beg + (end - beg) * ri);
}

void Random::randomizeFloat(f32 *arr, u32 len, f32 beg, f32 end) {
	for (u32 i = 0; i < len; ++i)
		arr[i] = randFloat(beg, end);
}

void Random::randomizeInt(u32 *arr, u32 len, u32 beg, u32 end) {
	for (u32 i = 0; i < len; ++i)
		arr[i] = randInt(beg, end);
}