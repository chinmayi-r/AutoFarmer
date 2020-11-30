######## Webcam Weed Detection Using a Tensorflow Lite model on a Raspberry Pi #########
#
# Description: 
# This program uses a TensorFlow Lite object detection model to perform object 
# detection on images taken live by the raspberry pi camera. It forms boxes and gives x and y coordinates and scores
# around the objects of interest in each image.
# Calculates how much weedicides to put on each weed seen in the image and sends this information along with the
# x and y coordinates over serial communication to a arduino that controls the motors and the the weedicide dispencing mechanism
#
# This code is based off the TensorFlow Lite image classification example at:
# https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/examples/python/label_image.py


# Import packages
import os
import argparse
import cv2
import numpy as np
import sys
import glob
import importlib.util
import serial
import time

def take_pic():
    pic_path='./images/image.jpg'
    os.system('fswebcam ~/tflite/images/image.jpg')
    return pic_path

def sort_arr(arr):

    # sorts x and y coordinates of weeds in acsending order so that the robot does not need to waste energy
    # moving back and forth

    print(arr)
    arr_sorted = sorted(xy_arr, key=lambda k: [k[1],k[0]])
    print(arr_sorted)
    return arr_sorted
    
def waitForArduino():

   # wait until the Arduino sends 'Arduino Ready' - allows time for Arduino reset
   # it also ensures that any bytes left over from a previous message are discarded

    msg = ""
    while msg.find("Arduino is ready") == -1:

      while ser.inWaiting() == 0:
        pass

      msg = recvFromArduino()

      print(msg)
      print()

    
def sendToArduino(sendStr):
    ser.write(sendStr)
    
def recvFromArduino():

  encoding = 'utf-8'
  ck = ""
  x = "" # message from arduino to be reieved here
  byteCount = -1 # to allow for the fact that the last increment will be one too many

  # wait for the start character
  while  ord(x) != startMarker:
    x = ser.read()

  # save data until the end marker is found
  while ord(x) != endMarker:
    if ord(x) != startMarker:
      ck = ck + str(x, encoding)
      byteCount += 1
    x = ser.read()

  return(ck)

def send_arr(td):
    numLoops = len(td)
    waitingForReply = False
    n = 0
    while n < numLoops:
        teststr = td[n]
        if waitingForReply == False:
            teststr=str(teststr).replace(']','>').replace('[','<')
            teststr_encoded = str.encode(teststr)
            sendToArduino(teststr_encoded)
            print("Sent from Raspberry Pi" + str(n) + "string: " + teststr)
            waitingForReply = True
        if waitingForReply == True:
            while ser.inWaiting() == 0:
                pass
        dataRecvd = recvFromArduino()
        print("Reply Received  " + dataRecvd)
        n += 1
        waitingForReply = False
        print()
        time.sleep(5)

# Define and parse input arguments
parser = argparse.ArgumentParser()
parser.add_argument('--modeldir', help='Folder the .tflite file is located in',
                    required=True)
parser.add_argument('--graph', help='Name of the .tflite file, if different than detect.tflite',
                    default='detect.tflite')
parser.add_argument('--labels', help='Name of the labelmap file, if different than labelmap.txt',
                    default='labelmap.txt')
parser.add_argument('--threshold', help='Minimum confidence threshold for displaying detected objects',
                    default=0.5)
parser.add_argument('--edgetpu', help='Use Coral Edge TPU Accelerator to speed up detection',
                    action='store_true')

args = parser.parse_args()

MODEL_NAME = args.modeldir
GRAPH_NAME = args.graph
LABELMAP_NAME = args.labels
min_conf_threshold = float(args.threshold)
use_TPU = args.edgetpu

serPort = "/dev/ttyACM0"        # Serial port of arduino
baudRate = 9600                 # baudRate of communication with the arduino

# Import TensorFlow libraries
# If tflite_runtime is installed, import interpreter from tflite_runtime, else import from regular tensorflow
# If using Coral Edge TPU, import the load_delegate library
pkg = importlib.util.find_spec('tflite_runtime')
if pkg:
    from tflite_runtime.interpreter import Interpreter
    if use_TPU:
        from tflite_runtime.interpreter import load_delegate
else:
    from tensorflow.lite.python.interpreter import Interpreter
    if use_TPU:
        from tensorflow.lite.python.interpreter import load_delegate

# If using Edge TPU, assign filename for Edge TPU model
if use_TPU:
    # If user has specified the name of the .tflite file, use that name, otherwise use default 'edgetpu.tflite'
    if (GRAPH_NAME == 'detect.tflite'):
         GRAPH_NAME = 'edgetpu.tflite'

ser = serial.Serial(serPort, baudRate)
print("Serial port " + serPort + " opened  Baudrate " + str(baudRate))


startMarker = 60        # Encoding for '<', which is the start marker of a message
endMarker = 62          # Encoding for '>', which is the end marker of a message


waitForArduino()
 
while True:
    IM_NAME = take_pic()
    # Get path to current working directory
    CWD_PATH = os.getcwd()
    PATH_TO_IMAGES = os.path.join(CWD_PATH,IM_NAME)
    images = glob.glob(PATH_TO_IMAGES)
    # Path to .tflite file, which contains the model that is used for object detection
    PATH_TO_CKPT = os.path.join(CWD_PATH,MODEL_NAME,GRAPH_NAME)

    # Path to label map file
    PATH_TO_LABELS = os.path.join(CWD_PATH,MODEL_NAME,LABELMAP_NAME)

    # Load the label map
    with open(PATH_TO_LABELS, 'r') as f:
        labels = [line.strip() for line in f.readlines()]

    # Load the Tensorflow Lite model.
    # If using Edge TPU, use special load_delegate argument
    if use_TPU:
        interpreter = Interpreter(model_path=PATH_TO_CKPT,
                                  experimental_delegates=[load_delegate('libedgetpu.so.1.0')])
        print(PATH_TO_CKPT)
    else:
        interpreter = Interpreter(model_path=PATH_TO_CKPT)

    interpreter.allocate_tensors()

    # Get model details
    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()
    height = input_details[0]['shape'][1]
    width = input_details[0]['shape'][2]

    floating_model = (input_details[0]['dtype'] == np.float32)

    input_mean = 127.5
    input_std = 127.5

    # Loop over every image and perform detection
    for image_path in images:
        # Load image and resize to expected shape [1xHxWx3]
        image = cv2.imread(image_path)
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        imH, imW, _ = image.shape 
        image_resized = cv2.resize(image_rgb, (width, height))
        input_data = np.expand_dims(image_resized, axis=0)

        # Normalize pixel values if using a floating model (i.e. if model is non-quantized)
        if floating_model:
            input_data = (np.float32(input_data) - input_mean) / input_std

        # Perform the actual detection by running the model with the image as input
        interpreter.set_tensor(input_details[0]['index'],input_data)
        interpreter.invoke()

        # Retrieve detection results
        boxes = interpreter.get_tensor(output_details[0]['index'])[0] # Bounding box coordinates of detected objects
        classes = interpreter.get_tensor(output_details[1]['index'])[0] # Class index of detected objects
        scores = interpreter.get_tensor(output_details[2]['index'])[0] # Confidence of detected objects
        #num = interpreter.get_tensor(output_details[3]['index'])[0]  # Total number of detected objects (inaccurate and not needed)

        # Loop over all detections and draw detection box if confidence is above minimum threshold
     
        xy_arr=[]
          
        for i in range(len(scores)):
            if ((scores[i] > min_conf_threshold) and (scores[i] <= 1.0)):
                # Get bounding box coordinates and draw box
                # Interpreter can return coordinates that are outside of image dimensions, need to force them to be within image using max() and min()
                ymin = int(max(1,(boxes[i][0] * imH)))
                xmin = int(max(1,(boxes[i][1] * imW)))
                ymax = int(min(imH,(boxes[i][2] * imH)))
                xmax = int(min(imW,(boxes[i][3] * imW)))
                object_name = labels[int(classes[i])]
                if object_name=='Weed':
                    dispense_amt=10
                elif object_name=='Chilli':
                    dispense_amt=0

                xy_arr.append([int((xmin+xmax)/2),int((ymin+ymax)/2), dispense_amt])
        xy_arr_sorted=sort_arr(xy_arr)
        xy_arr_sorted.append('done')
        send_arr(xy_arr_sorted)
