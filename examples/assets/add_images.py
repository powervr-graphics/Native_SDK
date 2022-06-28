from scipy import misc
from scipy import ndimage
from PIL import Image
import argparse
import sys

parser = argparse.ArgumentParser()
parser.add_argument('-i', '--inputs', required=True, type=str)
parser.add_argument('-o', '--output', default="addition.png", type=str)

args = parser.parse_args();

inputs = args.inputs.split(',')
output = args.output
if (len(inputs) < 2 ):
    exit()

input_imgs = [Image.open(input) for input in inputs]
newimage = Image.new(input_imgs[0].mode, (input_imgs[0].width, input_imgs[0].height), "black")
pixels = newimage.load()

for row in range(input_imgs[0].height):
    for col in range(input_imgs[0].width):
        for input in input_imgs:
           pixels[col,row] +=  input.getpixel((col, row))

print ("Saving new image")
misc.imsave(output, newimage) # uses the Image module (PIL)

