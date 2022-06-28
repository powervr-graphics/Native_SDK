import os

colorspace = b'\x01\x00\x00\x00'
for dir, subdirs, filenames in os.walk("."):
    print (filenames)
    for filename in [ x for x in filenames if len(x) > 4 and x[-4:] == ".pvr"]:
        print (os.path.join(dir, filename))
        with open(os.path.join(dir, filename), "rb") as file:
            head = file.read(4+4+8)
            file.read(4)
            tail = file.read()
        with open(os.path.join(dir, filename), "wb") as file:
            file.write(head)
            file.write(colorspace)
            file.write(tail)