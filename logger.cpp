#include <amxxmodule.h>
#include <time.h>
#include <am-string.h>
#include <logger.h>

//#define SHOW_LOG_STRING_BUILDER
#define INVALID_LOGGER  0
#define ALL_LOGGERS    -1

NativeHandler<Logger> LoggerHandles;

bool m_LoggedMap = false;

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

int formatLoggerString(const char *format,
			char *buffer, int bufferLen,
			const char *date,
			const char *message,
			const char *time,
			const char *severity,
			const char *plugin,
			const char *mapname) {
	
#ifdef SHOW_LOG_STRING_BUILDER
	MF_PrintSrvConsole("FORMAT: %s\n", format);
#endif

	int offset = 0;
	int len, width, precision;
	const char *c = format;
	goto skip;
nextIteration:
	c++;
skip:
	for (; *c != '\0'; c++) {
#ifdef SHOW_LOG_STRING_BUILDER
		MF_PrintSrvConsole("->%s\n", buffer);
#endif
		if (*c != '%') {
			strncpyc(buffer + offset, *c, bufferLen - offset);
			offset++;
			continue;
		}

		width = 0;
		precision = 0;

		c++;
		switch (*c) {
			case '\0': // EOS
				goto ReturnStmt;
			case '-':  // %-
				while (true) {
					c++;
					switch (*c) {
						case '\0':
							goto ReturnStmt;
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9': // %-#
							width *= 10;
							width += (*c - '0');
							break;
						case '.':
							while (true) {
								c++;
								switch (*c) {
									case '\0':
										goto ReturnStmt;
									case '0': case '1': case '2': case '3': case '4':
									case '5': case '6': case '7': case '8': case '9': // %-#.#
										precision *= 10;
										precision += (*c - '0');
										break;
									case 'd':  // %-#.#d
										offset += (len = strncpys(buffer + offset, date, min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									case 'f':  // %-#.#f
										offset += (len = strncpys(buffer + offset, "function", min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									case 'l':  // %-#.#l
										offset += (len = strncpys(buffer + offset, message, min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									case 'm':  // %-#.#m
										offset += (len = strncpys(buffer + offset, mapname, min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									case 'n':  // %-#.#n
										offset += (len = strncpys(buffer + offset, plugin, min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									case 's':  // %-#.#s
										offset += (len = strncpys(buffer + offset, severity, min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									case 't':  // %-#.#t
										offset += (len = strncpys(buffer + offset, time, min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									case '%':  // %-#.#%
										offset += (len = strncpyc(buffer + offset, '%', min(bufferLen - offset, precision)));
										pad(width - len, offset, buffer, bufferLen);
										goto nextIteration;
									default:   // error, skip "%-#.#" and recover by backing up before incorrect symbol
										c--;
										break;
								}
							}

							break;
						case 'd':  // %-#d
							offset += (len = strncpys(buffer + offset, date, bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						case 'f':  // %-#f
							offset += (len = strncpys(buffer + offset, "function", bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						case 'l':  // %-#l
							offset += (len = strncpys(buffer + offset, message, bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						case 'm':  // %-#m
							offset += (len = strncpys(buffer + offset, mapname, bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						case 'n':  // %-#n
							offset += (len = strncpys(buffer + offset, plugin, bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						case 's':  // %-#s
							offset += (len = strncpys(buffer + offset, severity, bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						case 't':  // %-#t
							offset += (len = strncpys(buffer + offset, time, bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						case '%':  // %-#%
							offset += (len = strncpyc(buffer + offset, '%', bufferLen - offset));
							pad(width - len, offset, buffer, bufferLen);
							goto nextIteration;
						default:   // error, skip "%-#" and recover by backing up before incorrect symbol
							c--;
							break;
					}
				}

				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': // %#
				width = (*c - '0');
				while (true) {
					c++;
					switch (*c) {
						case '\0':
							goto ReturnStmt;
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9': // %#
							width *= 10;
							width += (*c - '0');
							break;
						case '.':
							while (true) {
								c++;
								switch (*c) {
									case '\0':
										goto ReturnStmt;
									case '0': case '1': case '2': case '3': case '4':
									case '5': case '6': case '7': case '8': case '9': // %#.#
										precision *= 10;
										precision += (*c - '0');
										break;
									case 'd':  // %#.#d
										len = strncpys(buffer + offset, date, min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									case 'f':  // %#.#f
										len = strncpys(buffer + offset, "function", min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									case 'l':  // %#.#l
										len = strncpys(buffer + offset, message, min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									case 'm':  // %#.#m
										len = strncpys(buffer + offset, mapname, min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									case 'n':  // %#.#n
										len = strncpys(buffer + offset, plugin, min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									case 's':  // %#.#s
										len = strncpys(buffer + offset, severity, min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									case 't':  // %#.#t
										len = strncpys(buffer + offset, time, min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									case '%':  // %#.#%
										len = strncpyc(buffer + offset, '%', min(bufferLen - offset, precision));
										shift(buffer + offset, len, width - len);
										pad(width - len, offset, buffer, bufferLen);
										offset += len;
										goto nextIteration;
									default:   // error, skip "%#.#" and recover by backing up before incorrect symbol
										c--;
										break;
								}
							}

							break;
						case 'd':  // %#d
							len = strncpys(buffer + offset, date, bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'f':  // %#f
							len = strncpys(buffer + offset, "function", bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'l':  // %#l
							len = strncpys(buffer + offset, message, bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'm':  // %#m
							len = strncpys(buffer + offset, mapname, bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'n':  // %#n
							len = strncpys(buffer + offset, plugin, bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 's':  // %#s
							len = strncpys(buffer + offset, severity, bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 't':  // %#t
							len = strncpys(buffer + offset, time, bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case '%':  // %#%
							len = strncpyc(buffer + offset, '%', bufferLen - offset);
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						default:   // error, skip "%#" and recover by backing up before incorrect symbol
							c--;
							break;
					}
				}

				break;
			case '.':
				while (true) {
					c++;
					switch (*c) {
						case '\0':
							goto ReturnStmt;
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9': // %.#
							precision *= 10;
							precision += (*c - '0');
							break;
						case 'd':  // %.#d
							len = strncpys(buffer + offset, date, min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'f':  // %.#f
							len = strncpys(buffer + offset, "function", min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'l':  // %.#l
							len = strncpys(buffer + offset, message, min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'm':  // %.#m
							len = strncpys(buffer + offset, mapname, min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 'n':  // %.#n
							len = strncpys(buffer + offset, plugin, min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 's':  // %.#s
							len = strncpys(buffer + offset, severity, min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case 't':  // %.#t
							len = strncpys(buffer + offset, time, min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						case '%':  // %.#%
							len = strncpyc(buffer + offset, '%', min(bufferLen - offset, precision));
							shift(buffer + offset, len, width - len);
							pad(width - len, offset, buffer, bufferLen);
							offset += len;
							goto nextIteration;
						default:   // error, skip "%.#" and recover by backing up before incorrect symbol
							c--;
							break;
					}
				}

				break;
			case 'd':  // %d
				offset += strncpys(buffer + offset, date, bufferLen - offset);
				break;
			case 'f':  // %f
				offset += strncpys(buffer + offset, "function", bufferLen - offset);
				break;
			case 'l':  // %l
				offset += strncpys(buffer + offset, message, bufferLen - offset);
				break;
			case 'm':  // %m
				offset += strncpys(buffer + offset, mapname, bufferLen - offset);
				break;
			case 'n':  // %n
				offset += strncpys(buffer + offset, plugin, bufferLen - offset);
				break;
			case 's':  // %s
				offset += strncpys(buffer + offset, severity, bufferLen - offset);
				break;
			case 't':  // %t
				offset += strncpys(buffer + offset, time, bufferLen - offset);
				break;
			case '%':  // %%
				offset += strncpyc(buffer + offset, '%', bufferLen - offset);
				break;
			default:   // error, skip percent and recover by backing up before incorrect symbol
				c--;
				break;
		}
	}

ReturnStmt:
	strncpyc(buffer + offset, '\0', bufferLen - offset);
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
	const char* severityStr = VERBOSITY[toIndex(severity)];

	static char formattedMessage[4096];
	int offset = formatLoggerString(
		getMessageFormat(),
		formattedMessage, sizeof formattedMessage - 2,
		date,
		message,
		time,
		severityStr,
		plugin->getName(),
		STRING(gpGlobals->mapname));
	*(formattedMessage + offset) = '\n';
	*(formattedMessage + offset + 1) = '\0';

	static char fileName[256];
	offset = formatLoggerString(
		getNameFormat(),
		fileName, sizeof fileName - 1,
		date,
		message,
		time,
		severityStr,
		plugin->getName(),
		STRING(gpGlobals->mapname));

	static char path[256];
	offset = formatLoggerString(
		getPathFormat(),
		path, sizeof path - 1,
		date,
		message,
		time,
		severityStr,
		plugin->getName(),
		STRING(gpGlobals->mapname));

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
	{ nullptr,				nullptr }
};

void OnAmxxAttach() {
	MF_AddNatives(amxmodx_Natives);
}