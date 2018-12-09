#pragma once

#include <chrono>
#include "types/generic.h"
#include "types/string.h"

namespace oi {

	typedef std::chrono::high_resolution_clock::time_point CTP;

	struct TimePoint {
		CTP point;
		String name;

		TimePoint(CTP time, String n);
	};

	class Timer {

	public:

		Timer();
		void lap(String lap);
		void stop();
		void reset();			//Resets start & end to current time

		//Clears timepoints; but not total delta time per lap
		void clear();

		void print() const;
		f32 getDuration() const;

		f32 count(f32 previous) const;

		static const Timer &getGlobalTimer();
		static CTP getTime();

		f32 timeSinceLastPoint(u32 i) const;

		std::unordered_map<String, f32> getTotalTime();
		u32 getTimesReset();

	private:

		static Timer global;

		static f32 duration(CTP t0, CTP t1);

		std::vector<TimePoint> timePoints;
		std::unordered_map<String, f32> totalTime;
		CTP beg;
		CTP end;
		bool isActive;

		u32 timesReset = 0;

	};
}