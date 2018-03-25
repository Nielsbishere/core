#include "utils/timer.h"
#include "utils/log.h"
using namespace oi;

TimePoint::TimePoint(CTP time, String n) : point(time), name(n) {}

CTP Timer::getTime() { return std::chrono::high_resolution_clock::now(); }

Timer::Timer() {
	start = end = getTime();
	isActive = true;
}

void Timer::lap(String lap) {
	if (!isActive) return;
	timePoints.push_back(TimePoint(getTime(), lap));
}

void Timer::stop() {
	end = getTime();
	isActive = false;
}

void Timer::print() const {
	if (isActive) Log::warn(String("Timer started ") + String(getDuration()) + " seconds ago and has " + String((i32)timePoints.size())  + " laps");
	else  Log::warn(String("Timer started ") + String(getDuration()) + " seconds ago, has " + String((i32)timePoints.size()) + " laps and ended after " + String(duration(start, end)) + " seconds");

	for (u32 i = 0; i < timePoints.size(); ++i)
		Log::warn(String("Lap ") + String((i32)(i + 1)) + ": " + timePoints[i].name + "; time since start " + String(duration(start, timePoints[i].point)) + "s, time since last time point: " + String(i == 0 ? duration(start, timePoints[i].point) : duration(timePoints[i - 1].point, timePoints[i].point)) + "s");
}

flp Timer::duration(CTP t0, CTP t1) { return (flp) std::chrono::duration<f64>(t1 - t0).count(); }
flp Timer::count(flp previous) const { return getDuration() - previous; }

flp Timer::getDuration() const { return duration(start, getTime()); }

Timer Timer::global = Timer();
const Timer &Timer::getGlobalTimer() { return global; }