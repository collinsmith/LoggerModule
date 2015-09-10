#ifndef _LOGGER_H_
	#define _LOGGER_H_

	#include <cstdlib>
	#include <stdio.h>
	#include <stdarg.h>
	#include <time.h>

	#if defined(_WIN32)
		#include <io.h>
	#endif
	
	//#include <amxmodx.h>
	//#include <natives_handles.h>

	class Logger {
	private:
		const char* m_pNameFormat;
		const char* m_pMessageFormat;
		const char* m_pDateFormat;
		const char* m_pTimeFormat;
		const char* m_pPath;

		int m_Verbosity;

	public:
		Logger(const char* nameFormat, const char* messageFormat, const char* dateFormat, const char* timeFormat, int verbosity, const char* path)
				: m_pNameFormat(nameFormat), m_pMessageFormat(messageFormat), m_pDateFormat(dateFormat), m_pTimeFormat(timeFormat), m_Verbosity(verbosity), m_pPath(path) {
		};

		~Logger() {
			free((char*)m_pNameFormat);
			free((char*)m_pMessageFormat);
			free((char*)m_pDateFormat);
			free((char*)m_pTimeFormat);
			free((char*)m_pPath);
		}

	public:
		const char* getNameFormat() const {
			return m_pNameFormat;
		};

		const char* getMessageFormat() const {
			return m_pMessageFormat;
		};

		const char* getDateFormat() const {
			return m_pDateFormat;
		};

		const char* getTimeFormat() const {
			return m_pTimeFormat;
		};

		const char* getPath() const {
			return m_pPath;
		};

		int getVerbosity() const {
			return m_Verbosity;
		};


	public:
		void log(int severity, const char* format, ...) const {
			if (getVerbosity() <= severity) {
				return;
			}

			// get time
			time_t td;
			time(&td);
			tm *curTime = localtime(&td);

			char date[32];
			strftime(date, 31, "%m/%d/%Y - %H:%M:%S", curTime);

			// msg
			static char msg[3072];

			va_list arglst;
			va_start(arglst, format);
			vsnprintf(msg, 3071, format, arglst);
			va_end(arglst);
		}
	};

	//extern NativeHandle<Logger> LoggerHandles;

#endif