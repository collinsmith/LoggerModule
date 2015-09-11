#include <assert.h>

#include "public\sdk\amxxmodule.h"
#include "public\sdk\moduleconfig.h"

#include "include\logger.h"

#define INVALID_LOGGER  0
#define ALL_LOGGERS    -1

NativeHandler<Logger> LoggerHandles;

int Logger::getVerbosity() const {
	return m_Verbosity;
}

int Logger::setVerbosity(int verbosity) {
	assert (SEVERITY_NONE <= verbosity);
	int oldVerbosity = m_Verbosity;
	m_Verbosity = verbosity;
	return oldVerbosity;
}

void Logger::log(int severity, const char* format, ...) const {
	if (getVerbosity() <= severity) {
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
}

void OnAmxxAttach() {
	//...
}

void OnAmxxDetach() {
	//...
}

// native Logger:LoggerCreate(
//		const nameFormat[] = DEFAULT_LOGGER_NAME_FORMAT,
//		const msgFormat[] = DEFAULT_LOGGER_MSG_FORMAT,
//		const dateFormat[] = DEFAULT_LOGGER_DATE_FORMAT,
//		const timeFormat[] = DEFAULT_LOGGER_TIME_FORMAT,
//		verbosity = DEFAULT_LOGGER_VERBOSITY,
//		const path[] = DEFAULT_LOGGER_PATH);
static cell AMX_NATIVE_CALL LoggerCreate(AMX* amx, cell* params) {
	MF_LogError(amx, AMX_ERR_NATIVE, "");

	int len;
	char* nameFormat = g_fn_GetAmxString(amx, params[1], 0, &len);
	char* msgFormat = g_fn_GetAmxString(amx, params[2], 0, &len);
	char* dateFormat = g_fn_GetAmxString(amx, params[3], 0, &len);
	char* timeFormat = g_fn_GetAmxString(amx, params[4], 0, &len);
	int verbosity = params[5];
	char* path = g_fn_GetAmxString(amx, params[6], 0, &len);
	return static_cast<cell>(LoggerHandles.create(
			nameFormat,
			msgFormat,
			dateFormat,
			timeFormat,
			verbosity, path));
}

static cell AMX_NATIVE_CALL LoggerDestroy(AMX* amx, cell* params) {
	cell* ptr = g_fn_GetAmxAddr(amx, params[1]);
	Logger* logger = LoggerHandles.lookup(*ptr);
	if (!logger) {
		return 0;
	}

	if (LoggerHandles.destroy(*ptr)) {
		*ptr = 0;
		return 1;
	}

	return 0;
}

static cell AMX_NATIVE_CALL LoggerGetVerbosity(AMX* amx, cell* params) {
	if (params[1] == ALL_LOGGERS) {
		return Logger::getAllVerbosity();
	}

	Logger* logger = LoggerHandles.lookup(params[1]);
	if (!logger) {
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid logger handle provided (%d)", params[1]);
		return 0;
	}

	return logger->getVerbosity();
}

static cell AMX_NATIVE_CALL LoggerSetVerbosity(AMX* amx, cell* params) {
	if (params[2] < SEVERITY_NONE) {
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid logger verbosity level provided (%d)", params[2]);
		return 0;
	}

	if (params[1] == ALL_LOGGERS) {
		return Logger::setAllVerbosity(params[2]);
	}

	Logger* logger = LoggerHandles.lookup(params[1]);
	if (!logger) {
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid logger handle provided (%d)", params[1]);
		return 0;
	}

	return logger->setVerbosity(params[2]);
}

static cell AMX_NATIVE_CALL LoggerIsLogging(AMX* amx, cell* params) {
	//...
}

static cell AMX_NATIVE_CALL LoggerSetLogging(AMX* amx, cell* params) {
	//...
}

// 
static cell AMX_NATIVE_CALL LoggerLog(AMX* amx, cell* params) {
	//...
}

AMX_NATIVE_INFO amxmodx_Natives[] = {
	{ "LoggerCreate",		LoggerCreate },
	{ "LoggerDestroy",		LoggerDestroy },
	{ "LoggerGetVerbosity",	LoggerGetVerbosity },
	{ "LoggerSetVerbosity",	LoggerSetVerbosity },
	{ "LoggerIsLogging",	LoggerIsLogging },
	{ "LoggerSetLogging",	LoggerSetLogging },
	{ "LoggerLog",			LoggerLog },
	{ NULL,					NULL }
};