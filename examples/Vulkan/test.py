import argparse
import os
import sys
import shutil
import subprocess

def run_example_test(example_name, dir, extra_call_params):
	call_string = "xvfb-run -a " + os.path.join(dir, example_name) + extra_call_params
	try:
		print("SwiftShader> === Running " + example_name + " ===")
		print("SwiftShader> Executing: " + call_string)
		subprocess.check_call(call_string, shell=True)
		print("SwiftShader> " + example_name + " - SUCCESS")
		print()
		return True
	except subprocess.CalledProcessError as e:
		print("SwiftShader> * Call: '" + call_string + "' raised an exception: \n" + str(e) + " ***")
		print("SwiftShader> * " + example_name + " - FAILED")
		print()

		return False

if __name__ == '__main__':
	parser = argparse.ArgumentParser()

	parser.add_argument('-binarydir', action='store', required=True, help='The location of binaries to test')
	succeeded = []
	failed = []
	args = parser.parse_args()
	result = True

	example = "VulkanHelloAPI"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanIntroducingPVRShell"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanIntroducingPVRVk"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanIntroducingPVRUtils"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanIntroducingUIRenderer"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanSkinning"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanMultiSampling"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanBumpmap"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanGaussianBlur"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanNavigation2D"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanNavigation3D"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanGlass"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanPVRScopeExample"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanPVRScopeRemote"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanImageBasedLighting"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanPipelineCache"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanHelloRayTracing"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanHybridReflections"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanHybridRefractions"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult
	example = "VulkanAmbientOcclusion"
	exampleresult = run_example_test(example, args.binarydir, " -qaf=5 -fft=100 -c=0-0")
	succeeded.append(example) if exampleresult else failed.append(example)
	result &= exampleresult

	print("Swiftshader> -------------- RESULTS ----------------")
	for example in succeeded:
		print("SwiftShader> " + example + "    - OK")
	for example in failed:
		print("SwiftShader> " + example + "    - FAILURE")
	if not succeeded and not failed: print("SwiftShader> WARNING: No tests have been run")
	else: 
		print()
		if not failed: print("SwiftShader> %%% ALL TESTS SUCCESSFUL %%%")
		if not succeeded: print("SwiftShader> !!! ALL EXAMPLES FAILED !!!" + str())
	print()
	print("Swiftshader> ----------------------------- ---------")

	if result:
		os._exit(0)
	else:
		os._exit(1)
