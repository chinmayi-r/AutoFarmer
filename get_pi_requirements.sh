#!/bin/bash

# Get packages required for TensorFlow
# Using the tflite_runtime packages available at https://www.tensorflow.org/lite/guide/python
# Will change to just 'pip3 install tensorflow' once newer versions of TF are added to piwheels

#pip3 install tensorflow

version=$(python -c 'import sys; print(".".join(map(str, sys.version_info[:2])))')

if [ $version == "3.7" ]; then
pip3 install https://dl.google.com/coral/python/tflite_runtime-2.1.0.post1-cp37-cp37m-linux_armv7l.whl
fi

if [ $version == "3.5" ]; then
pip3 install https://dl.google.com/coral/python/tflite_runtime-2.1.0.post1-cp35-cp35m-linux_armv7l.whl
fi

