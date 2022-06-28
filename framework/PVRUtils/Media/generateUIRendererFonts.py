import os
import sys
import subprocess

###
def get_platform():
    platforms = {
        'linux1' : 'Linux',
        'linux2' : 'Linux',
        'darwin' : 'macOS',
        'win32' : 'Windows'
    }
    if sys.platform not in platforms:
        return sys.platform

    return platforms[sys.platform]
###

###
def flip_colorSpace_bit_to_sRGB(filename):
    colorspace = b'\x01\x00\x00\x00'
    with open(filename, "rb") as file:
        head = file.read(4+4+8)
        file.read(4)
        tail = file.read()
    with open(filename, "wb") as file:
        file.write(head)
        file.write(colorspace)
        file.write(tail)
        print ("Flipping " + filename + " colorspace to sRGB")
###

###
def generateUIRendererFonts():
    execRoot = os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..','..','..','..', "Utilities"))
    outFileRoot = os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..'))

    dirPVRFontBuilder = os.path.join(execRoot, "PVRFontBuilder")
    dirPVRTexTool = os.path.join(execRoot, "PVRTexTool", "CLI")
    dirFileWrap = os.path.join(execRoot, "Filewrap")

    platformName = get_platform()
    if platformName == "Linux":
        dirPVRFontBuilder = os.path.join(dirPVRFontBuilder, "Linux_x86_64","PVRFontBuilderCL")
        dirPVRTexTool = os.path.join(dirPVRTexTool, "Linux_x86_64","PVRTexToolCLI")
        dirFileWrap = os.path.join(dirFileWrap, "Linux_x86_64","Filewrap")
    elif platformName == "Windows":
        dirPVRFontBuilder = os.path.join(dirPVRFontBuilder, "Windows_x86_32", "PVRFontBuilderCL.exe")
        dirPVRTexTool = os.path.join(dirPVRTexTool, "Windows_x86_64","PVRTexToolCLI.exe")
        dirFileWrap = os.path.join(dirFileWrap, "Windows_x86_32","Filewrap.exe")
    elif platformName == "macOS":
        dirPVRFontBuilder = os.path.join(dirPVRFontBuilder, "OSX_x86", "PVRFontBuilderCL")
        dirPVRTexTool = os.path.join(dirPVRTexTool, "OSX_x86","PVRTexToolCLI")
        dirFileWrap = os.path.join(dirFileWrap, "OSX_x86","Filewrap")

    subprocess.call(dirPVRFontBuilder + " -f arialbd.ttf -s 36", shell=True)
    subprocess.call(dirPVRFontBuilder + " -f arialbd.ttf -s 46", shell=True)
    subprocess.call(dirPVRFontBuilder + " -f arialbd.ttf -s 56", shell=True)

    #################################################################################################
    # NOTE: OpenglES does not support single channel sRGB texture format therefore it is store as
    #       RGB888 sRGB texture. For Vulkan  the texture is stored as R8 sRGB texture.
    #################################################################################################

    ### RGB888
    subprocess.call(dirPVRTexTool + " -i arialbd_36.pvr -f a8 -alpha arialbd_36.pvr,a -o arialbd_36_a8.pvr", shell=True)
    subprocess.call(dirPVRTexTool + " -i arialbd_46.pvr -f a8 -alpha arialbd_46.pvr,a -o arialbd_46_a8.pvr", shell=True)
    subprocess.call(dirPVRTexTool + " -i arialbd_56.pvr -f a8 -alpha arialbd_56.pvr,a -o arialbd_56_a8.pvr", shell=True)

    subprocess.call(dirFileWrap + " -h -o " +  os.path.join(os.path.abspath(outFileRoot), "ArialBoldFont.h arialbd_36_a8.pvr"), shell=True)
    subprocess.call(dirFileWrap + " -h -oa " + os.path.join(os.path.abspath(outFileRoot), "ArialBoldFont.h arialbd_46_a8.pvr"), shell=True)
    subprocess.call(dirFileWrap + " -h -oa " + os.path.join(os.path.abspath(outFileRoot), "ArialBoldFont.h arialbd_56_a8.pvr"), shell=True)

#    os.remove("arialbd_36_a8.pvr")
#    os.remove("arialbd_46_a8.pvr")
#    os.remove("arialbd_56_a8.pvr")

    ### R8
    subprocess.call(dirPVRTexTool + " -i arialbd_36.pvr -f r8 -red arialbd_36.pvr,a -o arialbd_36_r8.pvr", shell=True)
    subprocess.call(dirPVRTexTool + " -i arialbd_46.pvr -f r8 -red arialbd_46.pvr,a -o arialbd_46_r8.pvr", shell=True)
    subprocess.call(dirPVRTexTool + " -i arialbd_56.pvr -f r8 -red arialbd_56.pvr,a -o arialbd_56_r8.pvr", shell=True)

    subprocess.call(dirFileWrap + " -h -oa " + os.path.join(os.path.abspath(outFileRoot), "ArialBoldFont.h arialbd_36_r8.pvr"), shell=True)
    subprocess.call(dirFileWrap + " -h -oa " + os.path.join(os.path.abspath(outFileRoot), "ArialBoldFont.h arialbd_46_r8.pvr"), shell=True)
    subprocess.call(dirFileWrap + " -h -oa " + os.path.join(os.path.abspath(outFileRoot), "ArialBoldFont.h arialbd_56_r8.pvr"), shell=True)

    os.remove("arialbd_36_r8.pvr")
    os.remove("arialbd_46_r8.pvr")
    os.remove("arialbd_56_r8.pvr")

# The entry point
if __name__ == "__main__":
    generateUIRendererFonts()
