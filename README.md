# Diploma thesis "Head Position Invariant Gaze Tracking"


## Included software
A more detailed description on each application included in this repository is given in the thesis document, section 4.2, Pages 78-87.

### WeGA - The Welfenlab Gaze API
This is the core library all subsequent applications build upon.

##### Building WeGA
libWeGA uses the qmake Buidsystem and can be build using the following commands:
```shell
mkdir libWeGA-build
cd libWeGA-build
qmake ../WeGA
make && make install
```

The following libraries are needed to successfully build the library:
- Qt
- [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
- [OpenCV 2.4](https://opencv.org/opencv-2-4-8.html)
- [OpenCVBlobsLib](https://github.com/OpenCVBlobsLib/opencvblobslib)
- [CGAL](https://www.cgal.org/) (probably needs an old release from 2011)



_ _ _



### Testing framework
All applications can be build using qmake, analogous to the build instructions given for WeGA.

#### CalibrationTest 
This application allows to define two test patterns that are used for calibration of the system (training data) and for testing its performance (evaluation data). The program consists of a graphical user interface for entering the probands data (like name, distance to monitor, etc.) and shows the test patterns on a full-screen window afterwards. During this process, the eye movement of the user is recorded to a video file. Additionally, a file is created that contains information about the screen-coordinates the user should have been looked at for the specific frames of the video file.

#### AlgorithmTest
The algorithm test consists of two separate programs. The programs implement either the fixed-head case or the head position invariant case of the tracking procedure. By setting various command-line arguments when executing the programs, different video resolutions, algorithm combinations and other parameters can be specified. Given these arguments and the output files of the CalibrationTest application, the programs process the video files and return the tracking results for each frame.

#### MappingTest
The tracking results are processed by two programs that implement the mapping techniques for the fixed-head and the invariant case. In combination with the data given by the calibration procedure, the error vectors between the estimated gaze-position and the position where the user did look to are returned. The behaviour of the algorithms can be specified by using command-line arguments.

_ _ _



### User interfaces
All applications can be build using qmake, analogous to the build instructions given for WeGA.

#### CameraCalibration
This program uses v4l2 (Video4Linux version 2) for controlling the focus mechanics of the camera used in the second hardware prototype. The application automatically focuses on the user’s pupil region by choosing the focus setting that generates the highest contrast around the pupil area.

#### CalibrationDaemon
This program runs as a background daemon on the user's system. Each time the user performs a mouse-click, the actual camera image and the position of the mouse-cursor is captured. Assuming the user fixates on the cursor – or at least next to it – when performing a click action, this program generates a continuously growing database of training-vectors that can be used for the mapping procedure. Technically, this applications uses the XRecord extension of the X-server and is therefore restricted to Linux/Unix platforms.

#### LiveDemonstration
This application acts as a short demonstration of the fixed-head tracking algorithms. At first it shows 9 calibration points arranged in a 3 × 3 pattern. Afterwards, a picture is shown and a red spot follows the user's fixation point interactively.