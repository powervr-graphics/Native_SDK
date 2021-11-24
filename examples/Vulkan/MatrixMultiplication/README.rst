=====================
MatrixMultiplication
=====================

This command line tool demonstrates the computation of the product of two matrices using various methods for Single Precision General Matrix Multiplication (SGEMM)

API
---
* Vulkan

Description
------------
This command line tool implements multiple methods for matrix multiplication using Vulkan compute. The example is headless, meaning no surface or frame is generated; as a result, the example does not use PVR::Shell. The different implementations use different methods and optimisations to compute the product, this includes using different memory layouts and utilisation of shared memory. 

Each implementation has its own shader, as multiple entry points per shader are not supported. by default the application will run in *"demo mode"* which will run all of the shaders one after another in sequence, timing each one. To get accurate test results the shaders are ran twice, and only the second time is recorded, this is to stabilise the results, as the first run can sometimes be inconsistent. 

Command line arguments can be passed to the example to change different aspects of the demo, including manually specifing which tests to run, and changing the size of the matrices, workgroups and other settings. pass the *"-h"* argument to see the full list of settings 


Any variables that are not set through the command-line will be set to their default values. default values can be seen with the "-default" option. This includes
the size of workgroups launched, in some cases the default sizes can be too large for some devices and will therefore need to be scaled down.

Notes
-----
The Workgroup sizes have been optimised for PowerVR hardware, meaning that the workgroup sizes are set to 32. However, this can still be changed to suit the platform via a command line argument. On Windows, when computing the product of very large matrices, the computation can take too long and the driver will *"loose"* the Vulkan device, so just be weary of the sizes you pick.
Unlike most examples in the SDK, this demo is entirely command line based, so you might find that you require different instructions to run it. On android this does not compile into a .apk but instead to a resource folder and an executable, this is more similar to a linux executable the SDK builds and as such the instructions to run it are a bit more involved.

1. first push it onto the device:

   ```shell
   adb push VulkanMatrixMultiplicaion /data/local/tmp/
   adb push Assets_VulkanMatrixMultiplication /data/local/tmp/
   ```

2. Enter into the shell of the device

   ```shell
   adb shell 
   ```

3. Make the executable runnable

   ```shell
   chmod 777 /data/local/tmp/VulkanMatrixMultiplication
   ```

4. Run it with desired command line arguments 

   ```shell
   /data/local/tmp/VulkanMatrixMultiplication -va
   ```

   

On MacOS the project is build into a .app file, and you can run it by simply double clicking, however MacOS will not spawn a terminal for you to see the output. If you wanted to see the results on the command line or specify any extra command line options, you can run the app using an already opened terminal. In this case you actually launch the executable which is inside the app package

```shell
VulkanMatrixMultiplication.app/Contents/MacOS/VulkanMatrixMultiplication -va
```

An interesting thing to note is that you must type the path case sensitive. MacOS will find the executable even if case isn't taken into account. However, the application will not be able to find the shader source code packaged into the app. This is because file loading from within the app is case sensitive, and because the application cannot use PVR::Shell it relies on the path supplied to it to find the files.  