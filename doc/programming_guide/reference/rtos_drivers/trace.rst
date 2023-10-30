############
Trace Driver
############

This driver can be used to instantiate an xscope-based trace module in an RTOS
application. The trace module currently supports both a demonstrative ASCII-mode
and Percepio's Tracealzyer on FreeRTOS. Both modes are dependent on
RTOS-specific hooks/macros to handle the majority of RTOS event recording and
integration.

For general usage of the FreeRTOS trace functionality please refer to FreeRTOS'
documentation here:
`RTOS Trace Macros <https://www.freertos.org/rtos-trace-macros.html>`_

For basic information on printf debugging using xscope please refer to the tools
guide here:
`XSCOPE debugging <https://www.xmos.ai/documentation/XM-014363-PC-6/html/tools-guide/quick-start/fast-printf.html>`_

*******************
Trace Configuration
*******************

In order to use the trace driver module, the following common steps must be
performed:

1. Add `rtos::drivers::trace` as a linked library for the desired CMake target
   application.
2. The target application's compiler arguments must include the `-fxscope` option.
3. The target application's list of sources must include an `.xscope` file with
   the first probe specified as:

   .. code-block:: xml

        <Probe name="freertos_trace" type="CONTINUOUS" datatype="NONE" units="NONE" enabled="true"/>

4. Include `xcore_trace.h` at the end of the RTOS configuration file
   (i.e. `FreeRTOSConfig.h`).
5. Enable both `configUSE_TRACE_FACILITY` and `configGENERATE_RUN_TIME_STATS`
   in `FreeRTOSConfig.h`.
6. Continue reading the following sections based on which trace mode is to be
   used. Additional configuration steps are required.

****************
Tracealyzer Mode
****************

The trace driver supports Percepio's Tracealyzer, a feature rich tool for
working with trace files. This implementation supports Tracealyzer's
`streaming mode`; currently, `snapshot mode` is not supported. The current
underlying trace recording implementation interfaces with the
`xscope_core_bytes` API function (on Probe 0).

To select Tracealyzer as the trace module's event recorder, the following must
be set. This can be applied at the CMake project level:

.. code-block:: c

    #define USE_TRACE_MODE TRACE_MODE_TRACEALYZER_STREAMING

.. note::
    `xcore_trace.h` contains the definition for these modes.

**************************
Tracealyzer Initialization
**************************

In addition to the configuration steps outlined above, Percepio's Tracealyzer
streaming mode needs additional function calls to start recording trace data. In
the most basic use-case, the following functions should be called on the XCORE
tile that is to record trace data:

.. code-block:: c

    xTraceInitialize();
    xTraceEnable(TRC_START);

.. note::

    `xTraceInitialize` must be called before any RTOS interaction
    (before any traced objects are being interacted with). It is advisable to
    call it as soon as possible in the application.

*****************
Tracealyzer Usage
*****************

The Percepio's Tracealzyer C-unit outputs to a stream-able file format called
Percepio Streaming Format (PSF). The `xscope2psf` utility aids in the extraction
of the PSF file from the underlying xscope communication (making it readily
available on the host's filesystem). This tool can be configured to read from a
VCD (value change dump) file that is generated when specifying the `xgdb` option
`--xscope-port <ip:port>`, or it can be configured as an xscope-endpoint when
specifying the `--xscope-port <ip:port>` option. Both options can be processed
by the Tracealyzer graphical tool either as a post processing step or live.

.. note::
    `xscope2psf` currently resides in a Tracealyzer example application here:
    `example <https://github.com/xmos/xcore_sdk/tree/main/examples/freertos/tracealyzer>`_.
    This is likely to change in the future. Refer to either the README or the
    application's help documentation for usage details.

.. note::
    Currently, the only supported PSF Streaming `target connection` type is
    `File System`. Ensure this connection type is specified under Tracealyzer's
    `Recording Settings`.

For general usage of Tracealyzer please refer to the Percepio's documentation here:
`Manual <https://percepio.com/getstarted/latest/html/freertos.html>`_

**********
ASCII Mode
**********

The trace driver supports a basic ASCII mode that is primarily meant as an
example for expanding support to other tracing tools/frameworks. In this mode,
only the following FreeRTOS trace hooks are supported:

- `traceTASK_SWITCHED_IN`
- `traceTASK_SWITCHED_OUT`

This implementation will produce xscope logs for the RTOS task switching. The
underlying xscope API `xscope_core_bytes` is used for communicating this
information.

To select ASCII mode as the trace module's event recorder, the following must
be set. This can be applied at the CMake project level:

.. code-block:: c

    #define USE_TRACE_MODE TRACE_MODE_XSCOPE_ASCII

.. note::
    `xcore_trace.h` contains the definition for these modes.

*************************
ASCII Mode Initialization
*************************

No additional steps are required for ASCII mode to start recording trace events
to xscope.

*****************
ASCII Mode Usage
*****************

To begin capturing ASCII mode traces, run `xgdb` with the `--xscope-file`
option. Task switching events will be recorded to the specified VCD (value
change dump) file.
