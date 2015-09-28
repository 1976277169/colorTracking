# colorTracking
This project is an exploration of color segmentation with application towards tracking the position of an orange ball moving between different colored cups. The output shows the position of the ball even when it is under one of the cups. The program can handle up to three cups colored red, green and blue.

## Requirements
- openCV 3.0
- Visual studios 2013

## Notes
This project requires OpenCV to be installed and OPENCV_DIR system variable to
be defined. Make sure you change settings for C++ and Linker to include these
additional  include directories

- $(OPENCV_DIR)\..\..\include
- $(OPENCV_DIR)\lib

Add the following Additional Dependencies to the Linker input
- opencv_ts300d.lib
- opencv_world300d.lib

## Test videos
3 videos with two cups
3 videos with three cups.
