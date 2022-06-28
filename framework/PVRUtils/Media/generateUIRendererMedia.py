import os
import sys
import subprocess
import generateUIRendererFonts

def generateUIRendererMedia():
    sdk_root = os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..','..','..','..'))
    utilities_root = os.path.join(sdk_root, "Utilities")
    textool_dir = os.path.join(utilities_root, "PVRTexTool", "CLI")
    filewrap_dir = os.path.join(utilities_root, "Filewrap")

    platformName = generateUIRendererFonts.get_platform()
    if platformName == "Linux":
        textool_dir = os.path.join(textool_dir, "Linux_x86_64", "PVRTexToolCLI")
        filewrap_dir = os.path.join(filewrap_dir, "Linux_x86_64", "Filewrap")
    elif platformName == "Windows":
        textool_dir = os.path.join(textool_dir, "Windows_x86_64", "PVRTexToolCLI.exe")
        filewrap_dir = os.path.join(filewrap_dir, "Windows_x86_32", "Filewrap.exe")
    elif platformName == "macOS":
        textool_dir = os.path.join(textool_dir, "OS_x86", "PVRTexToolCLI")
        filewrap_dir = os.path.join(filewrap_dir, "OS_x86", "Filewrap")

    imagination_logo = os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..', "ImaginationLogo.h"))

    pvrtextool_command = textool_dir + " -i Imagination_Logo_vert_wht.png -flip y -f r8g8b8a8 -mfilter cubic -rfilter cubic -r 401,222 -m -o Imagination_Logo_RGBA.pvr"
    print("Generating pvr: '" + pvrtextool_command + "'")
    subprocess.call(pvrtextool_command, shell=True)
    generateUIRendererFonts.flip_colorSpace_bit_to_sRGB("Imagination_Logo_RGBA.pvr")

    filewrap_command = filewrap_dir + " -h -o " + imagination_logo + " Imagination_Logo_RGBA.pvr"
    print("Filewrapping pvr: '" + filewrap_command + "'")
    subprocess.call(filewrap_command, shell=True)

# The entry point
if __name__ == "__main__":
    generateUIRendererMedia()
    generateUIRendererFonts.generateUIRendererFonts()