# Running the code for the AutoFarmer Robot on a Raspberry Pi 4B

## Introduction
Our robot has two parts:
1. A raspberry pi 4B that does all the processing for the robot. It takes pictures of the crop the robot is placed over, and searches the picture for weeds using a TensorFlow Lite machine learning model. TensorFlow Lite is an optimized framework for deploying lightweight deep learning models on resource-constrained edge devices. TensorFlow Lite models have faster inference time and require less processing power, so they can be used to obtain faster performance in realtime applications. It identifies the exact coordinates of each of the weeds in the picture and relays this information to the arduino over serial communication. It also identifies the type of weed and calculates the exact amount of weedicide to be sprayed on it so that too much is not dispenced. This makes sure that the soil does not turn acidic. This is not possible using the present method of dispencing weedicides, where the farmer sprays the weeedicide evenly over the field. The farmer may spray more weedicide than required and destroy the fertility of his soil.
2. An Arduino Mega controls the movements of the robot. Based on the x and y coordinates it recieves from the rpi, it moves the wheels of the robot in the y direction and moves a stepper motor connected to a lead screw in the x direction. In this way, it moves to each of the weeds in the picture and sprays a biological weedicide on it in the required amount. The amount to be dispenced is relayed to it through serial from the rpi. The weedicide is dispenced with a submersible pump placed in the bottle of weedicide the robot carries on it.

On a raspberry pi 4B:
1. The Raspberry Pi needs to be fully updated. Open a terminal and issue:
```
sudo apt-get update
sudo apt-get dist-upgrade
```
2. Next, clone this GitHub repository by issuing the following command. The repository contains the scripts needed to run TensorFlow Lite, as well as a shell script to install all required repositaries. Issue:

```
git clone https://github.com/chinmayi-r/AutoFarmer.git
```
This downloads everything into a folder called AutoFarmer. The code for the Rpi and the Mega along with the TensorFlow Lite model is  the terminal, cd into it:
```
cd AutoFarmer
```
3. Run the downloaded shell script to install all required packages and dependencies:
```
bash get_pi_requirements.sh
```
4. Connect an Arduino Mega over USB to the rpi

5. Open the file Arduino_serial.ino in the folder of the same name in the arduino IDE and run it on the arduino. This is the program we run on the Mega to control all movements on the robot

In the terminal, issue:
```
python3 rpi_tflite --modeldir=TFLite_model
```
This is the python script we run on the rpi of the robot to do all the processing and run our machine learning model.

