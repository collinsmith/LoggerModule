#include "public\sdk\amxxmodule.h"
#include "public\sdk\moduleconfig.h"

#include "include\logger.h"

//NativeHandle<Logger> LoggerHandles;

void OnAmxxAttach() {
	//...
}

void OnAmxxDetach() {
	//...
}

static cell AMX_NATIVE_CALL LoggerCreate(AMX* amx, cell* params) {
	MF_LogError(amx, AMX_ERR_NATIVE, "");
}