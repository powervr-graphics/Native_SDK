#!/usr/bin/python3 -i
#
# copyright Copyright (c) Imagination Technologies Limited.

import fileinput
import os,re,sys
import ntpath
import xml.etree.ElementTree as etree
import vk_auto_gen_config
sys.path.append(vk_auto_gen_config.lunarg_scripts)

from generator import *
from common_codegen import *
from collections import namedtuple

#
# VkBindingsGeneratorOptions - subclass of GeneratorOptions.
class VkBindingsValidatorGeneratorOptions(GeneratorOptions):
	def __init__(self,
				 conventions = None,
				 filename = None,
				 directory = '.',
				 apiname = None,
				 profile = None,
				 versions = '.*',
				 emitversions = '.*',
				 defaultExtensions = None,
				 addExtensions = None,
				 removeExtensions = None,
				 emitExtensions = None,
				 sortProcedure = regSortFeatures,
				 prefixText = "",
				 genFuncPointers = True,
				 apicall = '',
				 apientry = '',
				 apientryp = '',
				 alignFuncParam = 0,
				 expandEnumerants = True):

		GeneratorOptions.__init__(self, conventions, filename, directory, apiname, profile,
								  versions, emitversions, defaultExtensions,
								  addExtensions, removeExtensions, emitExtensions, sortProcedure)
		self.prefixText	  = prefixText
		self.genFuncPointers = genFuncPointers
		self.prefixText	  = None
		self.apicall		 = apicall
		self.apientry		= apientry
		self.apientryp		= apientryp
		self.alignFuncParam  = alignFuncParam
#
# VkBindingsValidatorGenerator - subclass of OutputGenerator.
# Validates vk functions support in PVRVk
class VkBindingsValidatorGenerator(OutputGenerator):
	"""Generate files showing unimplemented bindings based on XML element attributes"""
	def __init__(self,
				 errFile = sys.stderr,
				 warnFile = sys.stderr,
				 diagFile = sys.stdout):
		OutputGenerator.__init__(self, errFile, warnFile, diagFile)

		# Internal state - accumulators for different inner block text
		self.ext_instance_dispatch_list = []  # List of extension entries for instance dispatch list
		self.ext_device_dispatch_list = []	# List of extension entries for device dispatch list
		self.core_commands = []				# List of CommandData records for core Vulkan commands
		self.ext_commands = []				# List of CommandData records for extension Vulkan commands
		self.CommandParam = namedtuple('CommandParam', ['type', 'name', 'cdecl'])
		self.CommandData = namedtuple('CommandData', ['name', 'ext_name', 'ext_type', 'return_type', 'handle_type', 'params', 'cdecl'])
		self.instanceExtensions = []
		self.ExtensionData = namedtuple('ExtensionData', ['name', 'type', 'define', 'num_commands'])

	#
	# Called once at the beginning of each run
	def beginFile(self, genOpts):
		OutputGenerator.beginFile(self, genOpts)

	#
	# Write generate and write function pointer tables to output file
	def endFile(self):
		file_data = ''

		file_data += self.OutputUnimplementedInstanceBindings()
		file_data += "\n"
		file_data += self.OutputUnimplementedDeviceBindings()

		write(file_data, file=self.outFile);

		# Finish processing in superclass
		OutputGenerator.endFile(self)

	def beginFeature(self, interface, emit):
		# Start processing in superclass
		OutputGenerator.beginFeature(self, interface, emit)

		enums = interface[0].findall('enum')
		self.currentExtension = ''
		self.name_definition = ''

		for item in enums:
			name_definition = item.get('name')
			if 'EXTENSION_NAME' in name_definition:
				self.name_definition = name_definition

		self.type = interface.get('type')
		self.num_commands = 0
		name = interface.get('name')
		self.currentExtension = name

	#
	# Process commands, adding to appropriate dispatch tables
	def genCmd(self, cmdinfo, name, alias):
		OutputGenerator.genCmd(self, cmdinfo, name, alias)

		# Get first param type
		params = cmdinfo.elem.findall('param')
		info = self.getTypeNameTuple(params[0])

		self.num_commands += 1

		self.AddCommandToDispatchList(self.currentExtension, self.type, name, cmdinfo, info[0])

	def endFeature(self):

		self.instanceExtensions.append(self.ExtensionData(name=self.currentExtension,
															type=self.type,
															define=self.name_definition,
															num_commands=self.num_commands))

		# Finish processing in superclass
		OutputGenerator.endFeature(self)

	#
	# Retrieve the value of the len tag
	def getLen(self, param):
		result = None
		len = param.attrib.get('len')
		if len and len != 'null-terminated':
			# For string arrays, 'len' can look like 'count,null-terminated',
			# indicating that we have a null terminated array of strings.  We
			# strip the null-terminated from the 'len' field and only return
			# the parameter specifying the string count
			if 'null-terminated' in len:
				result = len.split(',')[0]
			else:
				result = len
			result = str(result).replace('::', '->')
		return result

	#
	# Determine if this API should be ignored or added to the instance or device function pointer table
	def AddCommandToDispatchList(self, extension_name, extension_type, name, cmdinfo, handle_type):
		handle = self.registry.tree.find("types/type/[name='" + handle_type + "'][@category='handle']")

		return_type =  cmdinfo.elem.find('proto/type')
		if (return_type != None and return_type.text == 'void'):
			return_type = None

		cmd_params = []

		# Generate a list of commands for use in printing the necessary
		# core instance terminator prototypes
		params = cmdinfo.elem.findall('param')
		lens = set()
		for param in params:
			len = self.getLen(param)
			if len:
				lens.add(len)
		paramsInfo = []
		for param in params:
			paramInfo = self.getTypeNameTuple(param)
			param_type = paramInfo[0]
			param_name = paramInfo[1]
			param_cdecl = self.makeCParamDecl(param, 0)
			cmd_params.append(self.CommandParam(type=param_type, name=param_name,
												cdecl=param_cdecl))

		if handle != None and handle_type != 'VkInstance' and handle_type != 'VkPhysicalDevice':
			# The Core Vulkan code will be wrapped in a feature called VK_VERSION_#_#
			# For example: VK_VERSION_1_0 wraps the core 1.0 Vulkan functionality
			if 'VK_VERSION_' in extension_name:
				self.core_commands.append(
					self.CommandData(name=name, ext_name=extension_name,
									 ext_type='device',
									 return_type = return_type,
									 handle_type = handle_type,
									 params = cmd_params,
									 cdecl=self.makeCDecls(cmdinfo.elem)[0]))
			else:
				self.ext_device_dispatch_list.append(name)
				self.ext_commands.append(
					self.CommandData(name=name, ext_name=extension_name,
									 ext_type=extension_type,
									 return_type = return_type,
									 handle_type = handle_type,
									 params = cmd_params,
									 cdecl=self.makeCDecls(cmdinfo.elem)[0]))
		else:
			# The Core Vulkan code will be wrapped in a feature called VK_VERSION_#_#
			# For example: VK_VERSION_1_0 wraps the core 1.0 Vulkan functionality
			if 'VK_VERSION_' in extension_name:
				self.core_commands.append(
					self.CommandData(name=name, ext_name=extension_name,
									 ext_type='instance',
									 return_type = return_type,
									 handle_type = handle_type,
									 params = cmd_params,
									 cdecl=self.makeCDecls(cmdinfo.elem)[0]))

			else:
				self.ext_instance_dispatch_list.append(name)
				self.ext_commands.append(
					self.CommandData(name=name, ext_name=extension_name,
									 ext_type=extension_type,
									 return_type = return_type,
									 handle_type = handle_type,
									 params = cmd_params,
									 cdecl=self.makeCDecls(cmdinfo.elem)[0]))

	#
	# Retrieve the type and name for a parameter
	def getTypeNameTuple(self, param):
		type = ''
		name = ''
		for elem in param:
			if elem.tag == 'type':
				type = noneStr(elem.text)
			elif elem.tag == 'name':
				name = noneStr(elem.text)
		return (type, name)

	#
	#
	def find_function_in_pvrvk(self, function_name):
		current_directory = os.path.realpath(os.path.dirname(self.outFile.name))
		pvrvk_files = [f for f in os.listdir(current_directory) if os.path.isfile(os.path.join(current_directory, f))]

		# remove pvrvk_vulkan_wrapper.h if it exists
		for file_path in pvrvk_files:
			file_name = ntpath.basename(file_path)
			if file_name == "pvrvk_vulkan_wrapper.h":
				pvrvk_files.remove(file_path)
				break

		for filename in pvrvk_files:
				if filename.endswith('.h') or filename.endswith('.cpp'):
					with open(os.path.join(current_directory, filename), 'r') as input:
						for line in input:
							if len(line.split(".vk", 1)) > 1:
								if function_name.split("vk", 1)[1] == line.split(".vk", 1)[1].split("(")[0]:
									return True

		return False

	#
	# Create an instance function pointer table from the appropriate list and return it as a string
	def OutputUnimplementedInstanceBindings(self):
		commands = []
		unimplemented_instance_functions = '// Unimplemented Instance functions\n'

		for x in range(0, 2):
			if x == 0:
				commands = self.core_commands
			else:
				commands = self.ext_commands

			for cur_cmd in commands:
				is_inst_handle_type = cur_cmd.ext_type == 'instance' or cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice'
				is_get_device_proc_address = cur_cmd.name == "vkGetDeviceProcAddr"
				if is_inst_handle_type or is_get_device_proc_address:

					if not self.find_function_in_pvrvk(cur_cmd.name):
						unimplemented_instance_functions += '	%s;\n' % cur_cmd.name

		return unimplemented_instance_functions

	#
	# Create a device function pointer table from the appropriate list and return it as a string
	def OutputUnimplementedDeviceBindings(self):
		commands = []
		unimplemented_device_functions = '// Unimplemented Device functions\n'

		for x in range(0, 2):
			if x == 0:
				commands = self.core_commands
			else:
				commands = self.ext_commands

			for cur_cmd in commands:
				is_inst_handle_type = cur_cmd.ext_type == 'instance' or cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice'
				if not is_inst_handle_type:
					if not self.find_function_in_pvrvk(cur_cmd.name):
						unimplemented_device_functions += '	%s;\n' % cur_cmd.name

		return unimplemented_device_functions