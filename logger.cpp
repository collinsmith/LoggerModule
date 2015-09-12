#include <amxxmodule.h>
#include <time.h>
#include <am-string.h>
#include <logger.h>

extern ke::AString g_log_dir;
ke::AString g_log_dir;

#define INVALID_LOGGER  0
#define ALL_LOGGERS    -1

NativeHandler<Logger> LoggerHandles;

bool m_LoggedMap = false;

int Logger::m_AllVerbosity = LOG_SEVERITY_LOWEST;

int Logger::getVerbosity() const {
	return m_Verbosity;
}

int Logger::setVerbosity(int verbosity) {
	assert (LOG_SEVERITY_NONE <= verbosity);
	int oldVerbosity = m_Verbosity;
	m_Verbosity = verbosity;
	return oldVerbosity;
}

const char* Logger::getNameFormat() const {
	return m_pNameFormat.chars();
}

const char* Logger::getMessageFormat() const {
	return m_pMessageFormat.chars();
}

const char* Logger::getDateFormat() const {
	return m_pDateFormat.chars();
}

const char* Logger::getTimeFormat() const {
	return m_pTimeFormat.chars();
}

const char* Logger::getPath() const {
	return m_pPath.chars();
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
	strftime(date, 15, getDateFormat(), curTime);

	char time[16];
	strftime(time, 15, getTimeFormat(), curTime);

	// msg
	static char msg[3072];

	va_list arglst;
	va_start(arglst, format);
	vsnprintf(msg, 3071, format, arglst);
	va_end(arglst);

	// "[%-5severity] [%time] %message"
}

// native Logger:LoggerCreate(
//		verbosity = DEFAULT_LOGGER_VERBOSITY,
//		const nameFormat[] = DEFAULT_LOGGER_NAME_FORMAT,
//		const msgFormat[] = DEFAULT_LOGGER_MSG_FORMAT,
//		const dateFormat[] = DEFAULT_LOGGER_DATE_FORMAT,
//		const timeFormat[] = DEFAULT_LOGGER_TIME_FORMAT,
//		const path[] = DEFAULT_LOGGER_PATH);
static cell AMX_NATIVE_CALL LoggerCreate(AMX* amx, cell* params) {
	int len;
	int verbosity = params[1];
	char* nameFormat = MF_GetAmxString(amx, params[2], 0, &len);
	char* msgFormat = MF_GetAmxString(amx, params[3], 1, &len);
	char* dateFormat = MF_GetAmxString(amx, params[4], 2, &len);
	char* timeFormat = MF_GetAmxString(amx, params[5], 3, &len);
	char* path = MF_GetAmxString(amx, params[6], 4, &len);
	return static_cast<cell>(LoggerHandles.create(
			verbosity,
			nameFormat,
			msgFormat,
			dateFormat,
			timeFormat,
			path));
}

// native bool:LoggerDestroy(&Logger:logger);
static cell AMX_NATIVE_CALL LoggerDestroy(AMX* amx, cell* params) {
	cell* ptr = MF_GetAmxAddr(amx, params[1]);
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

// native Severity:LoggerGetVerbosity(Logger:logger);
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

// native Severity:LoggerSetVerbosity(Logger:logger, Severity:verbosity);
static cell AMX_NATIVE_CALL LoggerSetVerbosity(AMX* amx, cell* params) {
	if (params[2] < LOG_SEVERITY_NONE) {
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

const char* VERBOSITY[] = {
	"ERROR",
	"WARN",
	"INFO",
	"DEBUG"
};

int toIndex(int severity) {
	if (severity >= LOG_SEVERITY_ERROR) {
		return 0;
	} else if (severity >= LOG_SEVERITY_WARN) {
		return 1;
	} else if (severity >= LOG_SEVERITY_INFO) {
		return 2;
	} else {
		return 3;
	}
}

// native LoggerLog(Logger:logger, Severity:severity, const format[], any:...);
static cell AMX_NATIVE_CALL LoggerLog(AMX* amx, cell* params) {
	if (params[2] < Logger::getAllVerbosity()) {
		return 0;
	}

	Logger* logger = LoggerHandles.lookup(params[1]);
	if (!logger) {
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid logger handle provided (%d)", params[1]);
		return 0;
	}
	
	if (params[2] < logger->getVerbosity()) {
		return 0;
	}

	static char file[256];
	static char name[256];

	time_t td;
	time(&td);
	tm* curTime = localtime(&td);

	char date[16];
	strftime(date, sizeof(date), logger->getDateFormat(), curTime);
	
	char time[16];
	strftime(time, sizeof(time), logger->getTimeFormat(), curTime);

	int len;
	char* format = MF_GetAmxString(amx, params[3], 0, &len);
	char* buffer = MF_FormatAmxString(amx, params, 3, &len);

	FILE *pF = NULL;
	UTIL_Format(name, 255, "%s/%s_%s.log", g_log_dir.chars(), logger->getNameFormat(), date);
	MF_BuildPathnameR(file, 255, "%s", name);
	pF = fopen(file, "a+");

	if (pF) {
		if (!m_LoggedMap) {
			fprintf(pF, "[%-5s] [%s] Start of error session.\n", VERBOSITY[2], time);
			fprintf(pF, "[%-5s] [%s] Info (map \"%s\") (file \"%s\")\n", VERBOSITY[2], time, STRING(gpGlobals->mapname), name);
			m_LoggedMap = true;
		}

		fprintf(pF, "[%-5s] [%s] %s\n", VERBOSITY[toIndex(params[2])], time, buffer);
		fclose(pF);
	} else {
		ALERT(at_logged, "[AMXX] Unexpected fatal logging error (couldn't open %s for a+). AMXX Error Logging disabled for this map.\n", file);
		return 0;
	}

	MF_PrintSrvConsole("[%-5s] [%s] %s\n", VERBOSITY[toIndex(params[2])], time, buffer);
	return 1;
}

AMX_NATIVE_INFO amxmodx_Natives[] = {
	{ "LoggerCreate",		LoggerCreate },
	{ "LoggerDestroy",		LoggerDestroy },
	{ "LoggerGetVerbosity",	LoggerGetVerbosity },
	{ "LoggerSetVerbosity",	LoggerSetVerbosity },
	{ "LoggerLog",			LoggerLog },
	{ NULL,					NULL }
};

void OnAmxxAttach() {
	MF_AddNatives(amxmodx_Natives);
}

void OnAmxxDetach() {
	//...
}