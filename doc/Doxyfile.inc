# This file provides overrides to the Doxyfile configuration

PROJECT_NAME = "XCORE RTOS Framework"
PROJECT_BRIEF = "XCORE RTOS Framework"

PREDEFINED = __DOXYGEN__=1 
PREDEFINED += DWORD_ALIGNED=
PREDEFINED += __attribute__((weak))=

# RTOS driver APIs
INPUT += ../modules/drivers 

# RTOS SW Services
INPUT += ../modules/sw_services/device_control/host ../modules/sw_services/device_control/api 
INPUT += ../modules/sw_services/generic_pipeline/api 
INPUT += ../modules/sw_services/concurrency_support/api 

ALIASES = "beginrst=^^\verbatim embed:rst^^"
ALIASES += "endrst=\endverbatim"
