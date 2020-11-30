import os
import time

x=0

def take_pic(file_no):
    os.system('fswebcam ~/tflite/images/image{i}.jpg'.format(i=file_no))
    return

while True:
    x=x+1
    take_pic(str(x))
