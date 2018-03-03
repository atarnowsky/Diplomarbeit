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

### User interfaces
All applications can be build using qmake, analogous to the build instructions given for WeGA.

#### CameraCalibration
This program uses v4l2 (Video4Linux version 2) for controlling the focus mechanics of the camera used in the second hardware prototype. The application automatically focuses on the user’s pupil region by choosing the focus setting that generates the highest contrast around the pupil area.

#### CalibrationDaemon
This program runs as a background daemon on the user's system. Each time the user performs a mouse-click, the actual camera image and the position of the mouse-cursor is captured. Assuming the user fixates on the cursor – or at least next to it – when performing a click action, this program generates a continuously growing database of training-vectors that can be used for the mapping procedure. Technically, this applications uses the XRecord extension of the X-server and is therefore restricted to Linux/Unix platforms.

#### LiveDemonstration
This application acts as a short demonstration of the fixed-head tracking algorithms. At first it shows 9 calibration points arranged in a 3 × 3 pattern. Afterwards, a picture is shown and a red spot follows the user's fixation point interactively.