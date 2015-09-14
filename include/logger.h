#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <am-vector.h>
#include "native_handler.h"

#define LOG_SEVERITY_HIGHEST LOG_SEVERITY_ERROR
#define LOG_SEVERITY_LOWEST  LOG_SEVERITY_DEBUG
#define LOG_SEVERITY_ERROR	 301
#define LOG_SEVERITY_WARN	 201
#define LOG_SEVERITY_INFO	 101
#define LOG_SEVERITY_DEBUG	 1
#define LOG_SEVERITY_NONE	 0

#define LOG_ARG_NONE     0
#define LOG_ARG_DATE     1
#define LOG_ARG_FUNCTION 2
#define LOG_ARG_MESSAGE  3
#define LOG_ARG_MAP      4
#define LOG_ARG_SCRIPT   5
#define LOG_ARG_SEVERITY 6
#define LOG_ARG_TIME     7

class Logger {
private:
	static int m_AllVerbosity;

public:
	static int getAllVerbosity() {
		return m_AllVerbosity;
	};

	static int setAllVerbosity(int verbosity) {
		assert(LOG_SEVERITY_NONE <= verbosity);
		int oldVerbosity = m_AllVerbosity;
		m_AllVerbosity = verbosity;
		return oldVerbosity;
	};

private:
	const ke::AString m_pNameFormat;
	const ke::AString m_pMessageFormat;
	const ke::AString m_pDateFormat;
	const ke::AString m_pTimeFormat;
	const ke::AString m_pPathFormat;

	int* m_pNameFormatArgs;
	int* m_pMessageFormatArgs;
	int* m_pPathFormatArgs;

	int m_Verbosity;

public:
	Logger(int verbosity,
				const char* nameFormat,
				const char* messageFormat,
				const char* dateFormat,
				const char* timeFormat,
				const char* pathFormat)
			: m_Verbosity(verbosity),
				m_pNameFormat(formatLoggerString(nameFormat, m_pNameFormatArgs)),
				m_pMessageFormat(formatLoggerString(messageFormat, m_pMessageFormatArgs, true)),
				m_pDateFormat(dateFormat),
				m_pTimeFormat(timeFormat),
				m_pPathFormat(formatLoggerString(pathFormat, m_pPathFormatArgs)) {
		/*MF_PrintSrvConsole("->%s; ", m_pNameFormat);
		for (int i = 0; i < sizeof(m_pNameFormatArgs)*sizeof(int); i++) {
			MF_PrintSrvConsole("%d ", m_pNameFormatArgs[i]);
		}
		MF_PrintSrvConsole("\n");
		MF_PrintSrvConsole("->%s; ", m_pMessageFormat);
		for (int i = 0; i < sizeof(m_pMessageFormatArgs)*sizeof(int); i++) {
			MF_PrintSrvConsole("%d ", m_pMessageFormatArgs[i]);
		}
		MF_PrintSrvConsole("\n");
		MF_PrintSrvConsole("->%s; ", m_pPathFormat);
		for (int i = 0; i < sizeof(m_pPathFormatArgs)*sizeof(int); i++) {
			MF_PrintSrvConsole("%d ", m_pPathFormatArgs[i]);
		}
		MF_PrintSrvConsole("\n");*/
	};

	~Logger() {
		delete m_pNameFormatArgs;
		delete m_pMessageFormatArgs;
		delete m_pPathFormatArgs;
	}

public:
	int getVerbosity() const;
	int setVerbosity(int verbosity);

	const char* getNameFormat() const;
	const char* getMessageFormat() const;
	const char* getDateFormat() const;
	const char* getTimeFormat() const;
	const char* getPathFormat() const;

	const int* getNameFormatArgs() const;
	const int* getMessageFormatArgs() const;
	const int* getPathFormatArgs() const;

	void log(int severity, const char* format, ...) const;

private:
	const char* formatLoggerString(const char *format, int *&argVector, bool appendNewline = false) const;
};

extern NativeHandler<Logger> LoggerHandles;

#endif // _LOGGER_H_