########################
Generic Pipeline Example
########################

This code snippet is an example of creating a pipeline to consume a buffer.

.. code-block:: c
    :caption: Example generic pipeline use

    static void *input_func(void *input_app_data)
    {
        uint32_t* data = pvPortMalloc(100 * sizeof(uint32_t));

        /* Populate some dummy data */
        for(int i=0; i<100; i++)
        {
            data[i] = i;
        }

        return data;
    }

    static void *output_func(void *data, void *output_app_data)
    {
        /* Use data here */
        for(int i=0; i<100; i++)
        {
            rtos_printf("val[%d] = %d\n", i, (uint32_t*)data[i]);
        }

        return 1;   /* Return nonzero value for generic pipeline to implicitly free the packet */
    }

    static void stage0(void *data)
    {
        /* Perform operation on data here*/
        ;
    }

    static void stage1(void *data)
    {
        /* Perform operation on data here*/
        ;
    }

    static void stage2(void *data)
    {
        /* Perform operation on data here*/
        ;
    }


.. code-block:: c
    :caption: Example generic pipeline use


    const pipeline_stage_t stages[] = {
        (pipeline_stage_t)stage0,
        (pipeline_stage_t)stage1,
        (pipeline_stage_t)stage2,
    };

    const configSTACK_DEPTH_TYPE stage_stack_sizes[] = {
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage0) + RTOS_THREAD_STACK_SIZE(input_func),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage1),
        configMINIMAL_STACK_SIZE + RTOS_THREAD_STACK_SIZE(stage2) + RTOS_THREAD_STACK_SIZE(output_func),
    };

    generic_pipeline_init((pipeline_input_t)input_func,
                        (pipeline_output_t)output_func,
                        NULL,
                        NULL,
                        stages,
                        (const size_t*) stage_stack_sizes,
                        configMAX_PRIORITIES,
                        stage_count);