=====================
MatrixMultiplication
=====================

This command line tool demonstrates the computation of the product of two matrices using various methods for Single Precision General Matrix Multiplication (SGEMM)

API
---
* Vulkan

Description
------------
This command line tool implements multiple methods for matrix multiplication using vulkan compute. The example is headless meaning no surface or frame is generated
The different implementations try different methods of memeory layout and some different uses of shared / local memory. Each implementation has its own shader as 
multiple entry points per shader are not supported. Individual shaders can be choosen to run with different configuration settings. To list the commands for this
program run with the "-h" option ie "./VulkanMatrixMultiplication.exe -h".
If no command line options are supplied then all benchmarks run in a demo mode.

Any variables that are not set through the commandline will be set to their default values. default values can be seen with the "-default" option. This includes
the size of workgroups launched, in some cases the default sizes can be too large for some devices and will therefore need to be scaled down.

On android this does not compile into a .apk but instead to a resource folder and an executable, which can be found in bin/(Debug or Release)/(Architecture)/
to run this command line example on an android we use adb which is provided by the android SDK.
first push it onto the device         : /adb push C:/(MatMulDirectory) /data/local/tmp/
then we enter the shell of the device : /adb shell 
make the executable runnable          : chmod 777 /data/local/tmp/arm64-v8a/VulkanMatrixMultiplication
finally run it                        : /data/local/tmp/arm64-v8a/VulkanMatrixMultiplication

On MacOS the project is build into a .app file. However since it commandline, the app will not launch a console when opened, However it will actually run in the
background without actually displaying the results. If you wanted to see the results on the command line or specify any extra command line options, you can run 
the app using a terminal. Instead of launching the app you actually launch /(Path-To-App).app/Contents/MacOS/VulkanMatrixMultiplication (cmdline options).
What you are doing is launching a unix executable which has been packaged into the app, the app package also contains vulkan libraries and resources such as the
compute shaders' source code.