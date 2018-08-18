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

		template<typename T>
		static std::vector<T> foreachCore(std::function<T (u32)> f) {

			u32 threads = cores();
			std::vector<std::future<T>> thr(threads);
			std::vector<T> results(threads);

			for (u32 i = 0; i < threads; ++i)
				thr[i] = std::move(std::async(f, i));

			for (u32 i = 0; i < threads; ++i)
				results[i] = thr[i].get();

			return results;
		}

		static void foreachCore(std::function<void (u32)> f) {

			u32 threads = cores();
			std::vector<std::future<void>> thr(threads);

			for (u32 i = 0; i < threads; ++i)
				thr[i] = std::move(std::async(f, i));

			for (u32 i = 0; i < threads; ++i)
				thr[i].get();
		}

		template<typename T>
		static std::vector<T> foreachCore(T (*f)(u32)) {

			u32 threads = cores();
			std::vector<std::future<T>> thr(threads);
			std::vector<T> results(threads);

			for (u32 i = 0; i < threads; ++i)
				thr[i] = std::move(std::async(f, i));

			for (u32 i = 0; i < threads; ++i)
				results[i] = thr[i].get();

			return results;
		}

		static void foreachCore(void (*f)(u32)) {

			u32 threads = cores();
			std::vector<std::future<void>> thr(threads);

			for (u32 i = 0; i < threads; ++i)
				thr[i] = std::move(std::async(f, i));

			for (u32 i = 0; i < threads; ++i)
				thr[i].get();
		}

	};

}