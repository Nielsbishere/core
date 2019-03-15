#include "utils/timer.h"
#include "utils/log.h"
using namespace oi;

TimePoint::TimePoint(CTP time, String n) : point(time), name(n) {}

CTP Timer::getTime() { return std::chrono::high_resolution_clock::now(); }

Timer::Timer() {
	beg = end = getTime();
	isActive = true;
}

f32 Timer::timeSinceLastPoint(u32 i) const {
	CTP t = timePoints[i].point;
	return i == 0 ? duration(beg, t) : duration(timePoints[i - 1].point, t);
}

void Timer::lap(String lap) {

	if (!isActive) return;

	timePoints.push_back(TimePoint(getTime(), lap));

	f32 delta = timeSinceLastPoint((u32) timePoints.size() - 1);

	auto it = totalTime.find(lap);

	if (it == totalTime.end())
		totalTime[lap] = delta;
	else
		it->second += delta;

}

void Timer::clear() {
	timePoints.clear();
}

std::unordered_map<String, f32> Timer::getTotalTime() { return totalTime; }
u32 Timer::getTimesReset() { return timesReset; }

void Timer::stop() {
	end = getTime();
	isActive = false;
}

void Timer::reset() {
	clear();
	beg = end = getTime();
	isActive = true;
	++timesReset;
}

void Timer::print() const {

	if (isActive) 
		Log::println(String("Timer started ") + String(getDuration()) + " seconds ago and has " + String((i32)timePoints.size())  + " laps");
	else  
		Log::println(String("Timer started ") + String(getDuration()) + " seconds ago, has " + String((i32)timePoints.size()) + " laps and ended after " + String(duration(beg, end)) + " seconds");

	for (u32 i = 0; i < timePoints.size(); ++i)
		Log::println(String("Lap ") + String((i32)(i + 1)) + ": " + timePoints[i].name + "; time since start " + String(duration(beg, timePoints[i].point)) + "s, time since last time point: " + String(timeSinceLastPoint(i)) + "s");

}

f32 Timer::duration(CTP t0, CTP t1) { return std::chrono::duration<f32>(t1 - t0).count(); }
f32 Timer::count(f32 previous) const { return getDuration() - previous; }

f32 Timer::getDuration() const { return duration(beg, getTime()); }

Timer Timer::global = Timer();
const Timer &Timer::getGlobalTimer() { return global; }