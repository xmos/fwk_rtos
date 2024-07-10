To generate the files in this folder, use a Windows machine and follow the steps below:

  1. install VisualStudio 2022 tools
  2. download the libusb-1.0 source code from https://github.com/libusb/libusb/archive/refs/tags/v1.0.27.zip
  3. unzip the downloaded zip archive
  4. copy from the unzipped files libusb-1.0.27\libusb\libusb.h to this folder
  5. open an “x86 Native Tools Command Prompt for VS 2022“
  6. run from the folder libusb-1.0.27\msvc in the unzipped files the command:

       msbuild -p:PlatformToolset=v143,Platform=win32,Configuration=Release libusb.sln

  7. copy the newly created file libusb-1.0.27\build\v143\Win32\Release\lib\libusb-1.0.lib to this folder