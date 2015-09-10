#if defined _logger_stocks_included
    #endinput
#endif
#define _logger_stocks_included

#include <amxconst>
#include "include/logger_const.inc"
#include "include/logger.inc"

/*******************************************************************************
 * STOCKS
 ******************************************************************************/

stock isValidSeverity(Severity:severity) {
    return Severity_All < severity && severity < Severity_None;
}

stock isLoggableSeverity(Severity:severity) {
    return Severity_All <= severity && severity <= Severity_None;
}

stock getSeverityName(Severity:severity, dst[], len) {
    switch (severity) {
        case Severity_Invalid: return copy(dst, len, SEVERITY_INVALID_STRING);
        case Severity_All: return copy(dst, len, SEVERITY_ALL_STRING);
        case Severity_Debug: return copy(dst, len, SEVERITY_DEBUG_STRING);
        case Severity_Info: return copy(dst, len, SEVERITY_INFO_STRING);
        case Severity_Warn: return copy(dst, len, SEVERITY_WARN_STRING);
        case Severity_Error: return copy(dst, len, SEVERITY_ERROR_STRING);
        case Severity_None: return copy(dst, len, SEVERITY_NONE_STRING);
        default: return -1;
    }

    return -1;
}