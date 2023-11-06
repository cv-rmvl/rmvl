#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "rmvl/camera/hik_camera.h"
#include "rmvlpara/loader.hpp"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

int main()
{
    HikCamera capture(GRAB_CONTINUOUS, RETRIEVE_CV);

    Mat tmp;
    while (!capture.read(tmp))
        ERROR_("fail to read the image.");
    VideoWriter writer("ts.avi", VideoWriter::fourcc('F', 'L', 'V', '1'), 40, tmp.size());

    int exposure = 1000;
    int gain = 64;
    int r_gain = 1200;
    int g_gain = 1200;
    int b_gain = 1200;

    // Load the last parameters
    FileStorage fs("out_para.yml", FileStorage::READ);
    if (fs.isOpened())
    {
        readExcludeNone(fs["exposure"], exposure);
        readExcludeNone(fs["gain"], gain);
        readExcludeNone(fs["r_gain"], r_gain);
        readExcludeNone(fs["g_gain"], g_gain);
        readExcludeNone(fs["b_gain"], b_gain);
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
