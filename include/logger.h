#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <assert.h>

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
	ke::AString m_pNameFormat;
	ke::AString m_pMessageFormat;
	ke::AString m_pDateFormat;
	ke::AString m_pTimeFormat;
	ke::AString m_pPath;

	int m_Verbosity;

public:
	Logger(int verbosity,
				const char* nameFormat,
				const char* messageFormat,
				const char* dateFormat,
				const char* timeFormat,
				const char* path)
			: m_Verbosity(verbosity),
				m_pNameFormat(nameFormat),
				m_pMessageFormat(messageFormat),
				m_pDateFormat(dateFormat),
				m_pTimeFormat(timeFormat),
				m_pPath(path) {};

public:
	int getVerbosity() const;
	int setVerbosity(int verbosity);
	const char* getNameFormat() const;
	const char* getMessageFormat() const;
	const char* getDateFormat() const;
	const char* getTimeFormat() const;
	const char* getPath() const;
	void log(int severity, const char* format, ...) const;
};

extern NativeHandler<Logger> LoggerHandles;

#endif // _LOGGER_H_