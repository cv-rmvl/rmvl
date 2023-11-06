#include <iostream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/camera/hik_camera.h"
#include "rmvlpara/loader.hpp"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

int main()
{
    int ret = MV_OK;
    MV_CC_DEVICE_INFO_LIST camera_list;
    ret = MV_CC_EnumDevices(MV_USB_DEVICE, &camera_list);
    if (ret != MV_OK)
        INFO_("cam - failed to enum camera devices");
    unsigned int nums = camera_list.nDeviceNum;
    auto info = camera_list.pDeviceInfo;
    INFO_("cam - camera quantity: %u", nums);

    printf("┌──────┬──────────────┬────────────────┬─────────────────────┬────────────────┐\n");
    printf("│ 索引 │  相机序列号  │    型号名字    │     设备版本号      │    通信协议    │\n");
    printf("├──────┼──────────────┼────────────────┼─────────────────────┼────────────────┤\n");
    unordered_map<unsigned int, string> layer_type_t;
    layer_type_t[MV_UNKNOW_DEVICE] = "Unknown device";
    layer_type_t[MV_GIGE_DEVICE] = "GigE device";
    layer_type_t[MV_1394_DEVICE] = "1394 device";
    layer_type_t[MV_USB_DEVICE] = "USB device";
    layer_type_t[MV_CAMERALINK_DEVICE] = "CameraLink device";
    for (unsigned int i = 0; i < nums; ++i)
    {
        const auto &usb_info = info[i]->SpecialInfo.stUsb3VInfo;
        printf("│  %02d  │ %-12.12s │ %-14.14s │ %-19.19s │ %-14.14s │\n",
               i, usb_info.chSerialNumber, usb_info.chModelName, usb_info.chDeviceVersion,
               layer_type_t[info[i]->nTLayerType].c_str());
    }
    printf("└──────┴──────────────┴────────────────┴─────────────────────┴────────────────┘\n");
    printf("\033[33m输入相机序列号, 退出输入 \"q\": \033[0m");
    string sn;
    cin >> sn;
    if (sn == "q")
        return 0;

    HikCamera capture(GRAB_CONTINUOUS, RETRIEVE_CV, sn.c_str());

    int exposure = 1000;
    int gain = 0;
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

    namedWindow("图像画面", WINDOW_NORMAL);
    resizeWindow("图像画面", Size(640, 480));

    Mat frame;
    while (capture.read(frame))
    {
        imshow("图像画面", frame);
        if (waitKey(1) == 27)
            if (waitKey(0) == 27)
                break;
    }

    return 0;
}
