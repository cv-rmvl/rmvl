#include <iostream>

#include <opencv2/highgui.hpp>

#include "rmvl/camera/mv_camera.h"

#include <CameraApi.h>

int main()
{
    INT camera_counts = 8;
    std::vector<tSdkCameraDevInfo> camera_list(camera_counts);
    auto status = CameraEnumerateDevice(camera_list.data(), &camera_counts);
    if (status != CAMERA_STATUS_SUCCESS)
    {
        printf("Failed to enumerate the camera decive!\n");
        return 0;
    }

    printf("┌──────┬──────────────┬────────────────┬─────────────────────┬────────────────┐\n");
    printf("│ 索引 │  相机序列号  │    产品型号    │     传感器类型      │    通信协议    │\n");
    printf("├──────┼──────────────┼────────────────┼─────────────────────┼────────────────┤\n");
    for (int i = 0; i < camera_counts; ++i)
        printf("│  %02d  │ %-12.12s │ %-14.14s │ %-19.19s │ %-14.14s │\n",
               i, camera_list[i].acSn, camera_list[i].acProductName,
               camera_list[i].acSensorType, camera_list[i].acPortType);
    printf("└──────┴──────────────┴────────────────┴─────────────────────┴────────────────┘\n");
    std::cout << "\033[33m输入相机序列号, 退出输入 \"q\": \033[0m";
    std::string sn;
    std::cin >> sn;
    if (sn == "q")
        return 0;
    rm::MvCamera capture(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV), sn.c_str());

    int exposure = 1000;
    int gain = 64;
    int r_gain = 100;
    int g_gain = 100;
    int b_gain = 100;

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

    capture.set(rm::CAMERA_MANUAL_EXPOSURE);
    capture.set(rm::CAMERA_EXPOSURE, exposure);
    capture.set(rm::CAMERA_GAIN, gain);
    capture.set(rm::CAMERA_MANUAL_WB);
    capture.set(rm::CAMERA_WB_RGAIN, r_gain);
    capture.set(rm::CAMERA_WB_GGAIN, g_gain);
    capture.set(rm::CAMERA_WB_BGAIN, b_gain);

    namedWindow("frame", cv::WINDOW_NORMAL);

    cv::Mat frame;
    if (!capture.read(frame))
        return -1;
    resizeWindow("frame", cv::Size(frame.cols * 0.8, frame.rows * 0.8));

    while (capture.read(frame))
    {
        cv::imshow("frame", frame);
        if (cv::waitKey(1) == 27)
            if (cv::waitKey(0) == 27)
                break;
    }

    return 0;
}
