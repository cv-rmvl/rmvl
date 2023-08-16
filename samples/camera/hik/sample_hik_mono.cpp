#include <unistd.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/camera/hik_video_capture.h"
#include "rmvlpara/loader.hpp"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

int exposure = 1000;
int gain = 0;
int r_gain = 1200;
int g_gain = 1200;
int b_gain = 1200;

HikVideoCapture cap(GRAB_CONTINUOUS, RETRIEVE_CV);

inline void exposureCallBack(int pos, void *) { exposure = pos, cap.set(CAP_PROP_RM_EXPOSURE, exposure); }
inline void gainCallBack(int pos, void *) { gain = pos, cap.set(CAP_PROP_RM_GAIN, gain); }
inline void rGainCallBack(int pos, void *) { r_gain = pos, cap.set(CAP_PROP_RM_WB_RGAIN, r_gain); }
inline void gGainCallBack(int pos, void *) { g_gain = pos, cap.set(CAP_PROP_RM_WB_GGAIN, g_gain); }
inline void bGainCallBack(int pos, void *) { b_gain = pos, cap.set(CAP_PROP_RM_WB_BGAIN, b_gain); }

int main()
{
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

    cap.set(CAP_PROP_RM_MANUAL_EXPOSURE);
    cap.set(CAP_PROP_RM_EXPOSURE, exposure);
    cap.set(CAP_PROP_RM_GAIN, gain);
    cap.set(CAP_PROP_RM_MANUAL_WB);
    cap.set(CAP_PROP_RM_WB_RGAIN, r_gain);
    cap.set(CAP_PROP_RM_WB_GGAIN, g_gain);
    cap.set(CAP_PROP_RM_WB_BGAIN, b_gain);

    namedWindow("图像画面", WINDOW_NORMAL);
    resizeWindow("图像画面", Size(1000, 750));
    namedWindow("控制面板", WINDOW_AUTOSIZE);
    Mat track_bar_img(Size(750, 1), CV_8UC1, Scalar(60, 60, 60));

    createTrackbar("曝光值", "控制面板", nullptr, 10000, exposureCallBack, nullptr);
    setTrackbarPos("曝光值", "控制面板", exposure);
    createTrackbar("增益值", "控制面板", nullptr, 16, gainCallBack, nullptr);
    setTrackbarPos("增益值", "控制面板", gain);
    createTrackbar("红通道", "控制面板", nullptr, 3000, rGainCallBack, nullptr);
    setTrackbarPos("红通道", "控制面板", r_gain);
    createTrackbar("绿通道", "控制面板", nullptr, 3000, gGainCallBack, nullptr);
    setTrackbarPos("绿通道", "控制面板", g_gain);
    createTrackbar("蓝通道", "控制面板", nullptr, 3000, bGainCallBack, nullptr);
    setTrackbarPos("蓝通道", "控制面板", b_gain);

    sleep(1);

    [[maybe_unused]] int ch = system("clear");

    const char *file_name = "out_para.yml";
    printf("Press the 's' key to save the parameters to the yaml file: \033[33m%s\033[0m\n", file_name);

    Mat frame;
    while (true)
    {
        if (!cap.read(frame))
            continue;
        imshow("图像画面", frame);
        imshow("控制面板", track_bar_img);
        char c = waitKey(1);
        if (c == 27)
        {
            if (waitKey(0) == 27)
                break;
        }
        else if (c == 's')
        {
            FileStorage fs(file_name, FileStorage::WRITE);
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
