#include "public\sdk\amxxmodule.h"
#include "public\sdk\moduleconfig.h"

#include "include\logger.h"

//NativeHandle<Logger> LoggerHandles;

unsigned int Logger::getVerbosity() const {
	return m_Verbosity;
};

unsigned int Logger::setVerbosity(unsigned int verbosity) {
	int old = m_Verbosity;
	m_Verbosity = verbosity;
	return old;
};

void Logger::log(int severity, const char* format, ...) const {
	if (m_Verbosity <= severity) {
		return;
	}

	// get time
	time_t td;
	time(&td);
	tm *curTime = localtime(&td);

	char date[16];
	strftime(date, 15, m_pDateFormat, curTime);

	char time[16];
	strftime(time, 15, m_pTimeFormat, curTime);

	// msg
	static char msg[3072];

	va_list arglst;
	va_start(arglst, format);
	vsnprintf(msg, 3071, format, arglst);
	va_end(arglst);

	// "[%-5severity] [%time] %message"
};

void OnAmxxAttach() {
	//...
}

void OnAmxxDetach() {
	//...
}

static cell AMX_NATIVE_CALL LoggerCreate(AMX* amx, cell* params) {
	MF_LogError(amx, AMX_ERR_NATIVE, "");
}

