#if defined _logger_included
    #endinput
#endif
#define _logger_included

#pragma reqlib logger
#if !defined AMXMODX_NOAUTOLOAD
	#pragma loadlib logger
#endif

#include <amxconst>
#include <logger_const>
#include <logger_stocks>

/*******************************************************************************
 * NATIVES
 ******************************************************************************/

native Logger:LoggerCreate(
        verbosity = Severity_Warn,
        const nameFormat[] = DEFAULT_LOGGER_NAME_FORMAT,
        const msgFormat[] = DEFAULT_LOGGER_MSG_FORMAT,
        const dateFormat[] = DEFAULT_LOGGER_DATE_FORMAT,
        const timeFormat[] = DEFAULT_LOGGER_TIME_FORMAT,
        const path[] = DEFAULT_LOGGER_PATH);

native bool:LoggerDestroy(&Logger:logger);

native LoggerGetVerbosity(Logger:logger);
native LoggerSetVerbosity(Logger:logger, verbosity);

native LoggerLog(Logger:logger, severity, bool:printStackTrace, const format[], any:...);

native LoggerLogError(Logger:logger, bool:printStackTrace = true, const format[], any:...);
native LoggerLogWarn(Logger:logger, bool:printStackTrace = true, const format[], any:...);
native LoggerLogInfo(Logger:logger, bool:printStackTrace = false, const format[], any:...);
native LoggerLogDebug(Logger:logger, bool:printStackTrace = false, const format[], any:...);