########################
Generic Pipeline
########################

The generic pipeline service provides a generic construct to create multithreaded pipelines. This can be used to create a variety of sequential operations on data, such as an audio processing pipeline.

The `generic_pipeline_init()` creates `stage_count` tasks. In the first stage the application provided `input_data` function pointer is called. The data then is passed to the first `stage_function`. After the first state function the data is passed by an RTOS queue to the subsequent stage function. Middle stage functions receive from the previous stage queue, call the stage function, and output to the next stage queue. The last stage function will receive from the previous stage queue, call the stage function, and then call the `output_data` function pointer.

.. toctree::
   :maxdepth: 1

   generic_pipeline_example
   generic_pipeline_api
