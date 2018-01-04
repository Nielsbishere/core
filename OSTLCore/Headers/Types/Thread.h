#pragma once

#include "Generic.h"
#include <future>
#include <functional>

namespace oi {

	class Thread {

	public:

		static u32 cores() {
			return std::thread::hardware_concurrency();
		}

		static void foreachCore(std::function<void (u32)> f) {

			u32 threads = cores();
			std::vector<std::future<void>> thr(threads);

			for (u32 i = 0; i < threads; ++i)
				thr[i] = std::move(std::async(f, i));

			for (u32 i = 0; i < threads; ++i)
				thr[i].get();
		}

	};

}