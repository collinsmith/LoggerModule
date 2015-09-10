#if defined _logger_included
    #endinput
#endif
#define _logger_included

#include <amxconst>
#include "include/logger_const.inc"
#include "include/logger_stocks.inc"

/*******************************************************************************
 * NATIVES
 ******************************************************************************/

native Logger:LoggerCreate(
        const nameFormat[] = DEFAULT_LOGGER_NAME_FORMAT,
        const msgFormat[] = DEFAULT_LOGGER_MSG_FORMAT,
        const dateFormat[] = DEFAULT_LOGGER_DATE_FORMAT,
        const timeFormat[] = DEFAULT_LOGGER_TIME_FORMAT,
        verbosity = DEFAULT_LOGGER_VERBOSITY,
        const path[] = DEFAULT_LOGGER_PATH);

native bool:LoggerDestroy(&Logger:logger);

native Severity:LoggerGetVerbosity(Logger:logger);
native Severity:LoggerSetVerbosity(Logger:logger, Severity:verbosity);

native bool:LoggerIsLogging(Logger:logger);
native bool:LoggerSetLogging(Logger:logger, bool:logging);

native LoggerLogDebug(Logger:logger, const format[], any:...);
native LoggerLogInfo(Logger:logger, const format[], any:...);
native LoggerLogWarn(Logger:logger, const format[], any:...);
native LoggerLogError(Logger:logger, const format[], any:...);

native LoggerLog(Logger:logger, Severity:severity, const format[], any:...);
