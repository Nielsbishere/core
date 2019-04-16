#ifdef __ANDROID__

#include "utils/log.h"
#include <android/log.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <pthread.h>
using namespace oi;

void printstr(const String &str){
	__android_log_print(ANDROID_LOG_INFO, "oi_Log", "%s", str.begin());
}

void printwar(const String &str){
	__android_log_print(ANDROID_LOG_WARN, "oi_Log", "%s", str.begin());
}

void printerr(const String &str){
	__android_log_print(ANDROID_LOG_ERROR, "oi_Log", "%s", str.begin());
}

LogCallback Log::errorc = printerr, Log::warningc = printwar, Log::printc = printstr;

struct BacktraceState {
	void **current, **end;
};

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context *context, void *arg) {

	BacktraceState *state = (BacktraceState*)arg;
	_Unwind_Ptr pc = _Unwind_GetIP(context);

	if (!pc)
		return _URC_NO_REASON;

	if (state->current == state->end)
		return _URC_END_OF_STACK;

	*state->current++ = (void*)pc;
	return _URC_NO_REASON;
}

size_t captureBacktrace(void **buffer, size_t max) {

	BacktraceState state = { buffer, buffer + max };
	_Unwind_Backtrace(unwindCallback, &state);

	return state.current - buffer;
}

void printStackTrace() {

	constexpr size_t count = 64;
	void *buffer[count]{};

	captureBacktrace(buffer, count);

	for (size_t idx = 5; idx < count; ++idx) {

		const void *addr = buffer[idx];

		if (!addr)
			break;

		const char *symbol = "";

		Dl_info info;
		if (dladdr(addr, &info) && info.dli_sname)
			symbol = info.dli_sname;

		if (!strlen(symbol)) {
			Log::error(String(addr));
			continue;
		}

		int status = 0;
		String name = abi::__cxa_demangle(symbol, 0, 0, &status);

		Log::error(name + "(" + String(addr) + ")");
	}
}

void handleAbort(int signum, siginfo_t *si, void*) {

	String value;

	switch (signum) {

	case SIGSEGV: value = "Segfault"; break;
	case SIGBUS: value = "Invalid address"; break;
	case SIGILL: value = "Invalid instruction"; break;
	case SIGFPE: value = "Floating point error"; break;
	case SIGPIPE: value = "Read from closed pipe"; break;
	default: value = "Aborted"; break;

	}

	Log::error(value);
	Log::error("Stacktrace:");
	printStackTrace();

	//TODO: Call ANativeActivity_finish, but that requires a Window (AppExt), which is handled by owc not otc; move Logging to owc?

}

bool Log::setupExitHandler() {

	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handleAbort;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
	sigaction(SIGILL, &sa, NULL);
	sigaction(SIGFPE, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	return true;
}

bool Log::setupExitHandlerValue = Log::setupExitHandler();

#endif