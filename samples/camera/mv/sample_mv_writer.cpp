#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "rmvl/camera/mv_camera.h"

using namespace rm;
using namespace std;
using namespace cv;

int main()
{
    MvCamera capture(rm::CameraConfig{}.set(rm::GrabMode::Continuous).set(rm::RetrieveMode::OpenCV));

    Mat tmp;
    while (!capture.read(tmp))
        ERROR_("fail to read the image.");
    VideoWriter writer("ts.avi", VideoWriter::fourcc('F', 'L', 'V', '1'), 40, tmp.size());

    int exposure = 1000;
    int gain = 64;
    int r_gain = 100;
    int g_gain = 100;
    int b_gain = 100;

    // Load the last parameters
    FileStorage fs("out_para.yml", FileStorage::READ);
    if (fs.isOpened())
    {
        fs["exposure"].isNone() ? void(0) : (fs["exposure"] >> exposure);
        fs["gain"].isNone() ? void(0) : (fs["gain"] >> gain);
        fs["r_gain"].isNone() ? void(0) : (fs["r_gain"] >> r_gain);
        fs["g_gain"].isNone() ? void(0) : (fs["g_gain"] >> g_gain);
        fs["b_gain"].isNone() ? void(0) : (fs["b_gain"] >> b_gain);
    }

    capture.set(CAMERA_MANUAL_EXPOSURE);
    capture.set(CAMERA_EXPOSURE, exposure);
    capture.set(CAMERA_GAIN, gain);
    capture.set(CAMERA_MANUAL_WB);
    capture.set(CAMERA_WB_RGAIN, r_gain);
    capture.set(CAMERA_WB_GGAIN, g_gain);
    capture.set(CAMERA_WB_BGAIN, b_gain);

    Mat frame;
    while (capture.read(frame))
    {
        imshow("frame", frame);
        writer.write(frame);
        if (waitKey(1) == 27)
            if (waitKey(0) == 27)
                break;
    }
}
