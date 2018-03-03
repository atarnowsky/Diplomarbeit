# Diploma thesis "Head Position Invariant Gaze Tracking"


## Included software

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
