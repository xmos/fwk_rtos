####################
Documentation Source
####################

**********************
Building Documentation
**********************

=============
Prerequisites
=============

Install `Docker <https://www.docker.com/>`_.

Pull the docker container:

.. code-block:: console

    $ docker pull ghcr.io/xmos/doc_builder:latest

========
Building
========

To build the documentation, run the following command in the root of the repository:

.. code-block:: console

    $ docker run --rm -t -u "$(id -u):$(id -g)" -v $(pwd):/build -e REPO:/build -e DOXYGEN_INCLUDE=/build/doc/Doxyfile.inc -e EXCLUDE_PATTERNS=/build/doc/exclude_patterns.inc -e DOXYGEN_INPUT=ignore ghcr.io/xmos/doc_builder:latest
