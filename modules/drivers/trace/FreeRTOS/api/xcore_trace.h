// Copyright 2020-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef XCORE_TRACE_H_
#define XCORE_TRACE_H_

/* List of available trace modes */
#define TRACE_MODE_DISABLED                     0
#define TRACE_MODE_XSCOPE_ASCII                 1
#define TRACE_MODE_TRACEALYZER_STREAMING        2

/* Set the desired trace mode. To be set in application-level configuration */
#ifndef USE_TRACE_MODE
#define USE_TRACE_MODE                          TRACE_MODE_DISABLED
#endif

#if (USE_TRACE_MODE != TRACE_MODE_DISABLED)
#include "FreeRTOSConfig.h"
#if configUSE_TRACE_FACILITY == 0
#error configUSE_TRACE_FACILITY must be enabled to trace
#endif

#if (configGENERATE_RUN_TIME_STATS == 0)
#error configGENERATE_RUN_TIME_STATS must be enabled to trace
#endif
#endif /* (USE_TRACE_MODE != TRACE_MODE_DISABLED) */

/* Enable required define for xscope ascii_trace functionality */
#define ENABLE_RTOS_XSCOPE_TRACE                (USE_TRACE_MODE == TRACE_MODE_XSCOPE_ASCII)

/* Configuration of the trace mode selected (via USE_TRACE_MODE). */
#if (USE_TRACE_MODE == TRACE_MODE_TRACEALYZER_STREAMING)

#ifndef __XC__
#include "trcRecorder.h"
#endif

#elif (USE_TRACE_MODE == TRACE_MODE_XSCOPE_ASCII)

#include <xscope.h>

/* Set defaults config values */
#ifndef xcoretraceconfigXSCOPE_TRACE_BUFFER
#define xcoretraceconfigXSCOPE_TRACE_BUFFER         200
#endif

#ifndef xcoretraceconfigXSCOPE_TRACE_RAW_BYTES
#define xcoretraceconfigXSCOPE_TRACE_RAW_BYTES      0
#endif

#if( !( __XC__ ) )
#include "ascii_trace.h"
#endif /* __XC__ */

#endif /* USE_TRACE_MODE */

#endif /* XCORE_TRACE_H_ */
