#ifndef _LOGGER_H_
#define _LOGGER_H_

#include "native_handler.h"

#define LOG_SEVERITY_HIGHEST LOG_SEVERITY_ERROR
#define LOG_SEVERITY_LOWEST  LOG_SEVERITY_DEBUG
#define LOG_SEVERITY_ERROR	 301
#define LOG_SEVERITY_WARN	 201
#define LOG_SEVERITY_INFO	 101
#define LOG_SEVERITY_DEBUG	 1
#define LOG_SEVERITY_NONE	 0

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
	const ke::AString m_pPath;

	int m_Verbosity;

public:
	Logger(int verbosity,
				const char* nameFormat,
				const char* messageFormat,
				const char* dateFormat,
				const char* timeFormat,
				const char* path)
			: m_Verbosity(verbosity),
				m_pNameFormat(formatLoggerString(nameFormat)),
				m_pMessageFormat(formatLoggerString(messageFormat, true)),
				m_pDateFormat(dateFormat),
				m_pTimeFormat(timeFormat),
				m_pPath(formatLoggerString(path)) {};

public:
	int getVerbosity() const;
	int setVerbosity(int verbosity);
	const char* getNameFormat() const;
	const char* getMessageFormat() const;
	const char* getDateFormat() const;
	const char* getTimeFormat() const;
	const char* getPath() const;
	void log(int severity, const char* message) const;

private:
	const char* formatLoggerString(const char* format, bool appendNewline = false) const;
};

extern NativeHandler<Logger> LoggerHandles;

#endif // _LOGGER_H_