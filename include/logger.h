#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <cstdlib>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#if defined(_WIN32)
	#include <io.h>
#endif

#define SEVERITY_ERROR 1000
#define SEVERITY_WARN 1000
#define SEVERITY_INFO 1000
#define SEVERITY_DEBUG 1000
#define SEVERITY_NONE 1000

//#include <amxmodx.h>
//#include <natives_handles.h>

class Logger {
private:
	const char* m_pNameFormat;
	const char* m_pMessageFormat;
	const char* m_pDateFormat;
	const char* m_pTimeFormat;
	const char* m_pPath;

	unsigned int m_Verbosity;

public:
	Logger(const char* nameFormat,
				const char* messageFormat,
				const char* dateFormat,
				const char* timeFormat,
				unsigned int verbosity,
				const char* path)
			: m_pNameFormat(nameFormat),
				m_pMessageFormat(messageFormat),
				m_pDateFormat(dateFormat),
				m_pTimeFormat(timeFormat),
				m_Verbosity(verbosity),
				m_pPath(path) {
	};

	~Logger() {
		free((char*)m_pNameFormat);
		free((char*)m_pMessageFormat);
		free((char*)m_pDateFormat);
		free((char*)m_pTimeFormat);
		free((char*)m_pPath);
	};

public:
	unsigned int getVerbosity() const;
	unsigned int setVerbosity(unsigned int verbosity);
	void log(int severity, const char* format, ...) const;
};

//extern NativeHandle<Logger> LoggerHandles;

#endif