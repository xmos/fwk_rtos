===================================
FAT File System Image Creation Tool
===================================

This is the FAT file system image creation tool. This tool creates a FAT filesystem image that is populated with the contents of the specified directory. All filenames must be in 8.3 format.


************************
Building the Application
************************

This application is typically built and installed from tools/install.

However, if you are modifying the application, it is possible to build the project using CMake. To build this application on Linux and MacOS, run the following commands:


.. code-block:: console

    cmake -B build
    cd build
    make -j

.. note::

   You may need to run the ``make -j`` command as ``sudo``.

Windows users must run the x86 native tools command prompt from Visual Studio and we recommend the use of the Ninja build system.

To install *Ninja* follow install instructions at https://ninja-build.org/ or on Windows
install with ``winget`` by running the following commands in *PowerShell*:

.. code-block:: PowerShell

    # Install
    winget install Ninja-build.ninja
    # Reload user Path
    $env:Path=[System.Environment]::GetEnvironmentVariable("Path","User")

To build this application on Windows, run the following commands:

.. code-block:: console

    cmake -G Ninja -B build
    cd build
    ninja
