#include "Utils/Timer.h"
#include "Utils/Log.h"
using namespace oi;

TimePoint::TimePoint(CTP time, OString n) : point(time), name(n) {}

CTP Timer::getTime() { return std::chrono::high_resolution_clock::now(); }

Timer::Timer() {
	start = end = getTime();
	isActive = true;
}

void Timer::lap(OString lap) {
	if (!isActive) return;
	timePoints.push_back(TimePoint(getTime(), lap));
}

void Timer::stop() {
	end = getTime();
	isActive = false;
}

void Timer::print() {
	if (isActive) Log::warn(OString("Timer started ") + OString((f32)getDuration()) + " seconds ago and has " + OString((i32)timePoints.size())  + " laps");
	else  Log::warn(OString("Timer started ") + OString((f32)getDuration()) + " seconds ago, has " + OString((i32)timePoints.size()) + " laps and ended after " + OString((f32)duration(start, end)) + " seconds");

	for (u32 i = 0; i < timePoints.size(); ++i)
		Log::warn(OString("Lap ") + OString((i32)(i + 1)) + ": " + timePoints[i].name + "; time since start " + OString((f32)duration(start, timePoints[i].point)) + "s, time since last time point: " + OString(i == 0 ? (f32)duration(start, timePoints[i].point) : (f32)duration(timePoints[i - 1].point, timePoints[i].point)) + "s");
}

f64 Timer::duration(CTP t0, CTP t1) { return std::chrono::duration<f64>(t1 - t0).count(); }
f64 Timer::count(f64 previous) { return getDuration() - previous; }

f64 Timer::getDuration() { return duration(start, getTime()); }