#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include "rmvl/camera/camutils.hpp"
#include "rmvl/camera/hik_camera.h"
#include "rmvl/core/util.hpp"

int main() {
    rm::HikCamera capture(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV));

    cv::Mat tmp;
    while (!capture.read(tmp))
        ERROR_("fail to read the image.");
    cv::VideoWriter writer("ts.avi", cv::VideoWriter::fourcc('F', 'L', 'V', '1'), 40, tmp.size());

    float exposure = 1000;
    float gain = 64;
    int r_gain = 1200;
    int g_gain = 1200;
    int b_gain = 1200;
    // Load the last parameters
    cv::FileStorage fs("out_para.yml", cv::FileStorage::READ);
    if (fs.isOpened()) {
        fs["exposure"].isNone() ? void(0) : (fs["exposure"] >> exposure);
        fs["gain"].isNone() ? void(0) : (fs["gain"] >> gain);
        fs["r_gain"].isNone() ? void(0) : (fs["r_gain"] >> r_gain);
        fs["g_gain"].isNone() ? void(0) : (fs["g_gain"] >> g_gain);
        fs["b_gain"].isNone() ? void(0) : (fs["b_gain"] >> b_gain);
    }

    capture.set(rm::CameraProperties::auto_exposure, false);
    capture.set(rm::CameraProperties::exposure, exposure);
    capture.set(rm::CameraProperties::gain, gain);
    capture.set(rm::CameraProperties::auto_wb, false);
    capture.set(rm::CameraProperties::wb_rgain, static_cast<uint32_t>(r_gain));
    capture.set(rm::CameraProperties::wb_ggain, static_cast<uint32_t>(g_gain));
    capture.set(rm::CameraProperties::wb_bgain, static_cast<uint32_t>(b_gain));

    cv::Mat frame;
    while (capture.read(frame)) {
        imshow("frame", frame);
        writer.write(frame);
        if (cv::waitKey(1) == 27)
            if (cv::waitKey(0) == 27)
                break;
    }
}
