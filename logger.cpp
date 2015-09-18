#include <amxxmodule.h>
#include <time.h>
#include <am-string.h>
#include <logger.h>

//#define SHOW_PARSER_DEBUGGING
//#define SHOW_LOG_STRING_BUILDER
#define INVALID_LOGGER  0
#define ALL_LOGGERS    -1

NativeHandler<Logger> LoggerHandles;

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

int Logger::m_AllVerbosity = LOG_SEVERITY_LOWEST;

int Logger::getVerbosity() const {
	return m_Verbosity;
}

int Logger::setVerbosity(int verbosity) {
	int oldVerbosity = m_Verbosity;
	m_Verbosity = max(LOG_SEVERITY_NONE, verbosity);
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

int strncpys(char *destination, const char *source, int len) {
	int count = 0;
	while (count < len && *source != '\0') {
		*destination++ = *source++;
		count++;
	}

	*destination = '\0';
	return count;
}

int strncpyc(char* destination, const char source, int len) {
	if (len > 0) {
		*destination = source;
		return 0;
	}

	return 1;
}

void pad(int len, int &offset, char* buffer, const int bufferLen) {
	for (; len > 0; len--, offset++) {
		strncpyc(buffer + offset, ' ', bufferLen - offset);
	}
}

void shift(char* str, int len, int right) {
	if (right <= 0) {
		return;
	}

	for (; len >= 0; len--) {
		*(str + len + right) = *(str + len);
	}
}

bool parseFormat(const char *&c, char &specifier, bool &lJustify, int &width, int &precision) {
	specifier = ' ';
	lJustify = false;
	width = -1;
	precision = -1;
	if (*c != '%') {
		return false;
	}

	int temp;
#ifdef SHOW_PARSER_DEBUGGING
	MF_PrintSrvConsole("c=%c\n", *c);
#endif
	c++;
	switch (*c) {
		case '\0':
			return false;
		case '-':
#ifdef SHOW_PARSER_DEBUGGING
			MF_PrintSrvConsole("- c=%c\n", *c);
#endif
			lJustify = true;
			c++;
			if (*c == '\0') {
				return false;
			}
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
#ifdef SHOW_PARSER_DEBUGGING
			MF_PrintSrvConsole("# c=%c", *c);
#endif
			if (0 <= (temp = *c - '0') && temp <= 9) {
				width = temp;
				c++;
				while (0 <= (temp = *c - '0') && temp <= 9) {
#ifdef SHOW_PARSER_DEBUGGING
					MF_PrintSrvConsole("\n# c=%c", *c);
#endif
					width *= 10;
					width += temp;
					c++;
				}
			} else {
#ifdef SHOW_PARSER_DEBUGGING
				MF_PrintSrvConsole("; next");
#endif
			}

#ifdef SHOW_PARSER_DEBUGGING
			MF_PrintSrvConsole(";\n");
#endif

			if (*c == '\0') {
				return false;
			}
		case '.':
#ifdef SHOW_PARSER_DEBUGGING
			MF_PrintSrvConsole(". c=%c", *c);
#endif
			if (*c == '.') {
				c++;
				if (0 <= (temp = *c - '0') && temp <= 9) {
#ifdef SHOW_PARSER_DEBUGGING
					MF_PrintSrvConsole("\n# c=%c", *c);
#endif
					precision = temp;
					c++;
					while (0 <= (temp = *c - '0') && temp <= 9) {
#ifdef SHOW_PARSER_DEBUGGING
						MF_PrintSrvConsole("\n# c=%c", *c);
#endif
						precision *= 10;
						precision += temp;
						c++;
					}
				} else {
					return false;
				}
			} else {
#ifdef SHOW_PARSER_DEBUGGING
				MF_PrintSrvConsole("; next");
#endif
			}

#ifdef SHOW_PARSER_DEBUGGING
			MF_PrintSrvConsole(";\n");
#endif

			if (*c == '\0') {
				return false;
			}
		case 'd': case 'f': case 'l': case 'm': case 'n':
		case 's': case 't': case '%':
#ifdef SHOW_PARSER_DEBUGGING
			MF_PrintSrvConsole("s c=%c\n", *c);
#endif
			switch (*c) {
				case 'd': case 'f': case 'l': case 'm': case 'n':
				case 's': case 't': case '%':
					specifier = *c;
					return true;
			}			
	}

	return false;
}

int parseLoggerString(const char *format,
			char *buffer, int bufferLen,
			const char *date,
			const char *message,
			const char *time,
			const char *severity,
			const char *plugin,
			const char *function,
			const char *mapname) {
	
#ifdef SHOW_LOG_STRING_BUILDER
	MF_PrintSrvConsole("FORMAT: %s\n", format);
#endif

	int offset = 0;
	char specifier;
	bool lJustify;
	int len, width, precision;
	const char *c = format;
	for (; *c != '\0'; c++) {
#ifdef SHOW_LOG_STRING_BUILDER
		MF_PrintSrvConsole("->%s|%s\n", buffer, c);
#endif
		if (*c != '%') {
			strncpyc(buffer + offset, *c, bufferLen - offset);
			offset++;
			continue;
		}

		assert (parseFormat(c, specifier, lJustify, width, precision));
		switch (specifier) {
			case 'd': len = strncpys(buffer + offset, date, precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
			case 'f': len = strncpys(buffer + offset, function, precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
			case 'l': len = strncpys(buffer + offset, message, precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
			case 'm': len = strncpys(buffer + offset, mapname, precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
			case 'n': len = strncpys(buffer + offset, plugin, precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
			case 's': len = strncpys(buffer + offset, severity, precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
			case 't': len = strncpys(buffer + offset, time, precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
			case '%': len = strncpyc(buffer + offset, '%', precision == -1 ? bufferLen - offset : min(bufferLen - offset, precision)); break;
		}

		if (lJustify) {
			offset += len;
			pad(width - len, offset, buffer, bufferLen);
		} else {
			shift(buffer + offset, len, width - len);
			pad(width - len, offset, buffer, bufferLen);
			offset += len;
		}
	}

#ifdef SHOW_LOG_STRING_BUILDER
	MF_PrintSrvConsole("->%s|%s\n", buffer, c);
#endif
	strncpyc(buffer + offset, '\0', bufferLen - offset);
	return offset;
}

void Logger::log(CPluginMngr::CPlugin *plugin, const char *function, int severity, const char* msgFormat, ...) const {
	if (!plugin->isDebug() && (severity < Logger::getAllVerbosity() || severity < getVerbosity())) {
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

	const char* severityStr = VERBOSITY[toIndex(severity)];

	static char pluginName[64];
	strcpy(pluginName, plugin->getName());
	*strrchr(pluginName, '.') = '\0';
	static char formattedMessage[4096];
	int len = parseLoggerString(
		getMessageFormat(),
		formattedMessage, sizeof formattedMessage - 2,
		date,
		message,
		time,
		severityStr,
		pluginName,
		function,
		STRING(gpGlobals->mapname));
	*(formattedMessage + len) = '\n';
	*(formattedMessage + len + 1) = '\0';

	static char fileName[256];
	int fileNameLen = parseLoggerString(
		getNameFormat(),
		fileName, sizeof fileName - 1,
		date,
		message,
		time,
		severityStr,
		pluginName,
		function,
		STRING(gpGlobals->mapname));

	static char path[256];
	int pathLen = parseLoggerString(
		getPathFormat(),
		path, sizeof path - 1,
		date,
		message,
		time,
		severityStr,
		pluginName,
		function,
		STRING(gpGlobals->mapname));

	static char fullPath[256];
	static const char *amxxLogsDir;
	if (!amxxLogsDir) {
		amxxLogsDir = MF_GetLocalInfo("amxx_logsdir", "addons/amxmodx/logs");
		MF_BuildPathnameR(fullPath, sizeof fullPath - 1, "%s", amxxLogsDir);
#if defined(__linux__) || defined(__APPLE__)
		mkdir(amxxLogsDir, 0700);
#else
		mkdir(amxxLogsDir);
#endif
	}

	if (getPathFormat()[0] != '\0') {
		MF_BuildPathnameR(fullPath, sizeof fullPath - 1, "%s/%s", amxxLogsDir, path);
#if defined(__linux__) || defined(__APPLE__)
		mkdir(fullPath, 0700);
#else
		mkdir(fullPath);
#endif
		MF_BuildPathnameR(fullPath, sizeof fullPath - 1, "%s/%s/%s.log", amxxLogsDir, path, fileName);
	} else {
		MF_BuildPathnameR(fullPath, sizeof fullPath - 1, "%s/%s.log", amxxLogsDir, fileName);
	}
	
	FILE *pF = NULL;
	pF = fopen(fullPath, "a+");
	if (pF) {
		fprintf(pF, formattedMessage);
		fclose(pF);
	} else {
		ALERT(at_logged, "[%s] Unexpected fatal logging error (couldn't open %s for a+). Logger disabled for this map.\n", MODULE_LOGTAG, fullPath);
		return;
	}

	MF_PrintSrvConsole(formattedMessage);
}

bool isValidLoggerFormat(const char *str, int &percentLoc, int &errorLoc) {
	percentLoc = -1;
	errorLoc = -1;

	char specifier;
	bool lJustify;
	int width, precision;
	const char *c = str;
	for (; *c != '\0'; c++) {
		if (*c != '%') {
			continue;
		}

		percentLoc = c - str;
		if (!parseFormat(c, specifier, lJustify, width, precision)) {
			errorLoc = c-str;
			return false;
		}
	}

	return true;
}

// native Logger:LoggerCreate(
//		verbosity = DEFAULT_LOGGER_VERBOSITY,
//		const nameFormat[] = DEFAULT_LOGGER_NAME_FORMAT,
//		const msgFormat[] = DEFAULT_LOGGER_MSG_FORMAT,
//		const dateFormat[] = DEFAULT_LOGGER_DATE_FORMAT,
//		const timeFormat[] = DEFAULT_LOGGER_TIME_FORMAT,
//		const path[] = DEFAULT_LOGGER_PATH);
static cell AMX_NATIVE_CALL LoggerCreate(AMX* amx, cell* params) {
	int len, percentLoc, errorLoc;
	int verbosity = params[1];
	char* nameFormat = MF_GetAmxString(amx, params[2], 0, &len);
	if (!isValidLoggerFormat(nameFormat, percentLoc, errorLoc)) {
		char *error = new char[errorLoc - percentLoc + 2];
		strncpy(error, nameFormat + percentLoc, errorLoc - percentLoc + 1);
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid logger name format provided: \"%s\" (position %d = \"%s\")", nameFormat, percentLoc + 1, error);
		delete[] error;
		return INVALID_LOGGER;
	}

	char* msgFormat = MF_GetAmxString(amx, params[3], 1, &len);
	if (!isValidLoggerFormat(msgFormat, percentLoc, errorLoc)) {
		char *error = new char[errorLoc - percentLoc + 2];
		strncpy(error, msgFormat + percentLoc, errorLoc - percentLoc + 1);
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid logger name format provided: \"%s\" (position %d = \"%s\")", msgFormat, percentLoc + 1, error);
		delete[] error;
		return INVALID_LOGGER;
	}

	char* dateFormat = MF_GetAmxString(amx, params[4], 2, &len);
	char* timeFormat = MF_GetAmxString(amx, params[5], 3, &len);
	char* path = MF_GetAmxString(amx, params[6], 4, &len);
	if (!isValidLoggerFormat(path, percentLoc, errorLoc)) {
		char *error = new char[errorLoc - percentLoc + 2];
		strncpy(error, path + percentLoc, errorLoc - percentLoc + 1);
		MF_LogError(amx, AMX_ERR_NATIVE, "Invalid logger name format provided: \"%s\" (position %d = \"%s\")", path, percentLoc + 1, error);
		delete[] error;
		return INVALID_LOGGER;
	}

	int loggerHandle = LoggerHandles.create(
		verbosity,
		nameFormat,
		msgFormat,
		dateFormat,
		timeFormat,
		path);

	Logger *logger = LoggerHandles.lookup(loggerHandle);
	assert (logger);
	CPluginMngr::CPlugin *p = (CPluginMngr::CPlugin*)amx->userdata[3];
	logger->log(p, "function", LOG_SEVERITY_INFO, "Logger initialized; map: %s", STRING(gpGlobals->mapname));
	return static_cast<cell>(loggerHandle);
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
	logger->log(p, "function", params[2], buffer);
	return 1;
}

AMX_NATIVE_INFO amxmodx_Natives[] = {
	{ "LoggerCreate",		LoggerCreate },
	{ "LoggerDestroy",		LoggerDestroy },
	{ "LoggerGetVerbosity",	LoggerGetVerbosity },
	{ "LoggerSetVerbosity",	LoggerSetVerbosity },
	{ "LoggerLog",			LoggerLog },
	{ nullptr,				nullptr }
};

void OnAmxxAttach() {
	MF_AddNatives(amxmodx_Natives);
}

void OnAmxxDetach() {
	LoggerHandles.clear();
}