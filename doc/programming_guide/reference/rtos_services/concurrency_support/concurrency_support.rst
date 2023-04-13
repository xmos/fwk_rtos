########################
Concurrency Support
########################

The concurrency support sw_service contains a multiple reader single writer lock to support multitheaded applications that need to safely support shared access to a single hardware or software resource. This implementation supports either reader preferred or writer preferred locks.

.. toctree::
   :maxdepth: 1

   concurrency_support_api
