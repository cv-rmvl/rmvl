#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/camera/hik_camera.h"
#include "rmvl/core/timer.hpp"

using namespace std::chrono_literals;

static int exposure = 5000;
static int gain = 0;
static int r_gain = 1200;
static int g_gain = 1200;
static int b_gain = 1200;

static rm::HikCamera cap(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV));

inline void exposureCallBack(int pos, void *) { exposure = pos, cap.set(rm::CAMERA_EXPOSURE, exposure); }
inline void gainCallBack(int pos, void *) { gain = pos, cap.set(rm::CAMERA_GAIN, gain); }
inline void rGainCallBack(int pos, void *) { r_gain = pos, cap.set(rm::CAMERA_WB_RGAIN, r_gain); }
inline void gGainCallBack(int pos, void *) { g_gain = pos, cap.set(rm::CAMERA_WB_GGAIN, g_gain); }
inline void bGainCallBack(int pos, void *) { b_gain = pos, cap.set(rm::CAMERA_WB_BGAIN, b_gain); }

int main()
{
    // Load the last parameters
    cv::FileStorage fs("out_para.yml", cv::FileStorage::READ);
    if (fs.isOpened())
    {
        fs["exposure"].isNone() ? void(0) : (fs["exposure"] >> exposure);
        fs["gain"].isNone() ? void(0) : (fs["gain"] >> gain);
        fs["r_gain"].isNone() ? void(0) : (fs["r_gain"] >> r_gain);
        fs["g_gain"].isNone() ? void(0) : (fs["g_gain"] >> g_gain);
        fs["b_gain"].isNone() ? void(0) : (fs["b_gain"] >> b_gain);
    }

    cap.set(rm::CAMERA_MANUAL_EXPOSURE);
    cap.set(rm::CAMERA_EXPOSURE, exposure);
    cap.set(rm::CAMERA_GAIN, gain);
    cap.set(rm::CAMERA_MANUAL_WB);
    cap.set(rm::CAMERA_WB_RGAIN, r_gain);
    cap.set(rm::CAMERA_WB_GGAIN, g_gain);
    cap.set(rm::CAMERA_WB_BGAIN, b_gain);

    namedWindow("图像画面", cv::WINDOW_NORMAL);
    resizeWindow("图像画面", cv::Size(1000, 750));
    namedWindow("控制面板", cv::WINDOW_AUTOSIZE);
    cv::Mat track_bar_img(cv::Size(750, 1), CV_8UC1, cv::Scalar(60, 60, 60));

    cv::createTrackbar("曝光值", "控制面板", nullptr, 70000, exposureCallBack, nullptr);
    cv::setTrackbarPos("曝光值", "控制面板", exposure);
    cv::createTrackbar("增益值", "控制面板", nullptr, 30, gainCallBack, nullptr);
    cv::setTrackbarPos("增益值", "控制面板", gain);
    cv::createTrackbar("红通道", "控制面板", nullptr, 3000, rGainCallBack, nullptr);
    cv::setTrackbarPos("红通道", "控制面板", r_gain);
    cv::createTrackbar("绿通道", "控制面板", nullptr, 3000, gGainCallBack, nullptr);
    cv::setTrackbarPos("绿通道", "控制面板", g_gain);
    cv::createTrackbar("蓝通道", "控制面板", nullptr, 3000, bGainCallBack, nullptr);
    cv::setTrackbarPos("蓝通道", "控制面板", b_gain);

    rm::Timer::sleep_for(1000);

    const char *file_name = "out_para.yml";
    printf("Press the 's' key to save the parameters to the yaml file: \033[33m%s\033[0m\n", file_name);

    cv::Mat frame;
    while (true)
    {
        if (!cap.read(frame))
            continue;
        imshow("图像画面", frame);
        imshow("控制面板", track_bar_img);
        char c = cv::waitKey(1);
        if (c == 27)
        {
            if (cv::waitKey(0) == 27)
                break;
        }
        else if (c == 's')
        {
            cv::FileStorage fs(file_name, cv::FileStorage::WRITE);
            fs.write("exposure", exposure);
            fs.write("gain", gain);
            fs.write("r_gain", r_gain);
            fs.write("g_gain", g_gain);
            fs.write("b_gain", b_gain);

            printf("\033[32mSuccess to write the parameters into \"%s\"\033[0m\n", file_name);
            printf(" -- exposure: %d\n", exposure);
            printf(" -- gain: %d\n", gain);
            printf(" -- r_gain: %d\n", r_gain);
            printf(" -- g_gain: %d\n", g_gain);
            printf(" -- b_gain: %d\n", b_gain);
        }
    }

    return 0;
}
