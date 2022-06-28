from scipy import misc
from scipy import ndimage
from PIL import Image
import argparse
import sys


parser = argparse.ArgumentParser()
parser.add_argument('INPUT')
parser.add_argument('-o', '--output', default = "lightmap.png")
parser.add_argument('--min', default = 0., type = float)
parser.add_argument('--max', default = 255., type = float)
parser.add_argument('--threshold', default = 0., type = float)

args = parser.parse_args();

input = args.INPUT
output = args.output
low_threshold = args.threshold
high_threshold = 255.
output_min = args.min
output_max = args.max


input_img = Image.open(input)
newimage = Image.new('L', (input_img.width, input_img.height), "black")
pixels = newimage.load()

for row in range(input_img.height):
    if (row % 100 == 0):
        print(row)
    for col in range(input_img.width):
        pel = input_img.getpixel((col, row))
        tmp = int(pel[0] * .299 + pel[1] * .587 + pel[2] * .114)
        
        a =  max(tmp - low_threshold, 0.) / (255 - low_threshold)
        
        a = a * (output_max - output_min) + output_min
        
        pixels[col,row] = int(a)

print ("Saving new image")
misc.imsave(output, newimage) # uses the Image module (PIL)

