#include <amxxmodule.h>
#include <time.h>
#include <am-string.h>
#include <logger.h>

#define INVALID_LOGGER  0
#define ALL_LOGGERS    -1

NativeHandler<Logger> LoggerHandles;

bool m_LoggedMap = false;

const char* VERBOSITY[] = {
	"ERROR",
	"WARN ",
	"INFO ",
	"DEBUG"
};

const int VERBOSITY_LEN[] = {
	5,
	5,
	5,
	5
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

const char* Logger::getPathFormat() const {
	return m_pPathFormat.chars();
}

const int* Logger::getNameFormatArgs() const {
	return m_pNameFormatArgs;
}

const int* Logger::getMessageFormatArgs() const {
	return m_pMessageFormatArgs;
}

const int* Logger::getPathFormatArgs() const {
	return m_pPathFormatArgs;
}

const char* Logger::formatLoggerString(const char *format, int *&argVector, bool appendNewline) const {
	char *fmtString = new char[sizeof format + 1];
	char *temp = fmtString;
	int *tempArray = new int[16];
	int *tempInt = tempArray;
	for (const char *c = format; *c != '\0'; c++) {
		*(temp++) = *c;
		if (*c != '%') {
			continue;
		}

		c++;
		switch (*c) {
			case '\0':// EOS
				return temp;
			case 'd': // date
				*(tempInt++) = LOG_ARG_DATE;
				*(temp++) = 's';
				break;
			case 'f': // function
				*(tempInt++) = LOG_ARG_FUNCTION;
				*(temp++) = 's';
				break;
			case 'l': // message
				*(tempInt++) = LOG_ARG_MESSAGE;
				*(temp++) = 's';
				break;
			case 'm': // map
				*(tempInt++) = LOG_ARG_MAP;
				*(temp++) = 's';
				break;
			case 'n': // script name
				*(tempInt++) = LOG_ARG_SCRIPT;
				*(temp++) = 's';
				break;
			case 's': // severity
				*(tempInt++) = LOG_ARG_SEVERITY;
				*(temp++) = 's';
				break;
			case 't': // time
				*(tempInt++) = LOG_ARG_TIME;
				*(temp++) = 's';
				break;
			case '%': // percent
			default:  // anything else
				*(temp++) = *c;
		}
	}

	if (appendNewline) {
		*(temp++) = '\n';
	}

	*tempInt = 0;
	*temp = '\0';

	argVector = tempArray;
	return fmtString;
}

int doFormatting(const char *format, int formatLen,
			const int *formatArgs,
			char *buffer, int bufferLen,
			const char *date, const int dateLen,
			const char *message, const int messageLen,
			const char *time, const int timeLen,
			const char *severity, const int severityLen,
			const char *plugin, const int pluginLen,
			const char *mapname, const int mapnameLen) {
	
	int offset = 0;
	const char *c;
	for (c = format; *c != '\0'; c++) {
		if (*c != '%') {
			*(buffer + offset) = *c;
			offset++;
			continue;
		}

		c++;
		switch (*c) {
			case '\0':// EOS
				goto breakFor;
			case 's': // severity
				c++;
				goto breakFor;
			case '%': // percent
				*(buffer + offset) = '%';
				offset++;
				break;
			default:  // anything else
				*(buffer + offset) = *c;
				offset++;
		}
	}

breakFor:
	int size = sizeof formatArgs*sizeof(int);
	for (int i = 0; i < size; i++) {
		switch (formatArgs[i]) {
			case LOG_ARG_DATE:
				strcpy(buffer + offset, date);
				offset += dateLen;
				break;
			case LOG_ARG_FUNCTION:
				strcpy(buffer + offset, "function");
				offset += 8;
				break;
			case LOG_ARG_MESSAGE:
				strcpy(buffer + offset, message);
				offset += messageLen;
				break;
			case LOG_ARG_MAP:
				strcpy(buffer + offset, mapname);
				offset += mapnameLen;
				break;
			case LOG_ARG_SCRIPT:
				strcpy(buffer + offset, plugin);
				offset += pluginLen;
				break;
			case LOG_ARG_SEVERITY:
				strcpy(buffer + offset, severity);
				offset += severityLen;
				break;
			case LOG_ARG_TIME:
				strcpy(buffer + offset, time);
				offset += timeLen;
				break;
			case LOG_ARG_NONE:
				strcpy(buffer + offset, c);
				return offset + (c - format);
		}

		for (; *c != '\0'; c++) {
			if (*c != '%') {
				*(buffer + offset) = *c;
				offset++;
				continue;
			}

			c++;
			switch (*c) {
				case '\0':// EOS
					goto breakLoop;
				case 's': // severity
					c++;
					goto breakLoop;
				case '%': // percent
					*(buffer + offset) = '%';
					offset++;
					break;
				default:  // anything else
					*(buffer + offset) = *c;
					offset++;
			}
		}

breakLoop:;
	}

	return offset;
}

void Logger::log(CPluginMngr::CPlugin *plugin, int severity, const char* msgFormat, ...) const {
	if (severity < getVerbosity()) {
		return;
	}

	time_t td;
	time(&td);
	tm* curTime = localtime(&td);

	char date[16];
	int dateLen = strftime(date, sizeof date - 1, getDateFormat(), curTime);

	char time[16];
	int timeLen = strftime(time, sizeof time - 1, getTimeFormat(), curTime);

	static char message[4096];
	
	va_list arglst;
	va_start(arglst, msgFormat);
	int messageLen = ke::SafeVsprintf(message, sizeof message - 1, msgFormat, arglst);
	va_end(arglst);

	//MF_PrintSrvConsole("got %s\n", plugin+31);
	int sevId = toIndex(severity);
	int pluginLen = strlen(plugin->getName());
	int mapnameLen = strlen(STRING(gpGlobals->mapname));
	static char formattedMessage[4096];
	int offset = doFormatting(
		m_pMessageFormat.chars(), m_pMessageFormat.length(),
		getMessageFormatArgs(),
		formattedMessage, sizeof formattedMessage - 1,
		date, dateLen,
		message, messageLen,
		time, timeLen,
		VERBOSITY[sevId], VERBOSITY_LEN[sevId],
		plugin->getName(), pluginLen,
		STRING(gpGlobals->mapname), mapnameLen);

	static char fileName[256];
	offset = doFormatting(
		m_pNameFormat.chars(), m_pNameFormat.length(),
		getNameFormatArgs(),
		fileName, sizeof fileName - 1,
		date, dateLen,
		message, messageLen,
		time, timeLen,
		VERBOSITY[sevId], VERBOSITY_LEN[sevId],
		plugin->getName(), pluginLen,
		STRING(gpGlobals->mapname), mapnameLen);
	//MF_PrintSrvConsole("got [%d]: %s\n", offset, fileName);

	static char path[256];
	offset = doFormatting(
		m_pPathFormat.chars(), m_pPathFormat.length(),
		getPathFormatArgs(),
		path, sizeof path - 1,
		date, dateLen,
		message, messageLen,
		time, timeLen,
		VERBOSITY[sevId], VERBOSITY_LEN[sevId],
		plugin->getName(), pluginLen,
		STRING(gpGlobals->mapname), mapnameLen);
	//MF_PrintSrvConsole("got [%d]: %s\n", offset, path);

	FILE *pF = NULL;
	if (getPathFormat()[0]) {
		UTIL_Format(path, sizeof path - 1, "%s/%s.log",
			path,
			fileName,
			date);
	} else {
		UTIL_Format(path, sizeof path - 1, "%s.log",
			fileName,
			date);
	}

	static char fullPath[256];
	MF_BuildPathnameR(fullPath, sizeof fullPath - 1, "%s/%s",
			MF_GetLocalInfo("amxx_logsdir", "addons/amxmodx/logs"),
			path);
	pF = fopen(fullPath, "a+");

	if (pF) {
		if (!m_LoggedMap) {
			//fprintf(pF, "[%-5s] [%s] Start of logging session.\n", VERBOSITY[toIndex(LOG_SEVERITY_INFO)], time);
			//fprintf(pF, "[%-5s] [%s] Map: \"%s\"; File: \"%s\"\n", VERBOSITY[toIndex(LOG_SEVERITY_INFO)], time, STRING(gpGlobals->mapname), fileName);
			m_LoggedMap = true;
		}

		fprintf(pF, formattedMessage);
		fclose(pF);
	} else {
		ALERT(at_logged, "[LOGGER] Unexpected fatal logging error (couldn't open %s for a+). Logger disabled for this map.\n", fullPath);
		return;
	}

	MF_PrintSrvConsole(formattedMessage);
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

	CPluginMngr::CPlugin *p = (CPluginMngr::CPlugin*)amx->userdata[3];
	int len;
	char* buffer = MF_FormatAmxString(amx, params, 3, &len);
	logger->log(p, params[2], buffer); // MF_GetScriptName(MF_FindScriptByAmx(amx))
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