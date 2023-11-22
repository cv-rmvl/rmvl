#include <iostream>
#include <unistd.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/camera/mv_camera.h"

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

// 内参矩阵
static Matx<double, 3, 3> cameraMatrix = {1250, 0, 640,
                                          0, 1250, 512,
                                          0, 0, 1};
// 畸变系数
static Matx<double, 5, 1> distCoeffs = {0, 0, 0, 0, 0};

static MvCamera capture(GRAB_CONTINUOUS, RETRIEVE_CV);

void cameraMatrixCallBack(int pos, void *mat_pos_)
{
    Point *mat_pos = static_cast<Point *>(mat_pos_);
    // 内参矩阵
    cameraMatrix(mat_pos->x, mat_pos->y) = pos;
}

void distCoeffCallBack(int pos, void *mat_pos_)
{
    Point *mat_pos = static_cast<Point *>(mat_pos_);
    // 畸变系数
    if (mat_pos->x == 0 || mat_pos->x == 1)
        distCoeffs(mat_pos->x, mat_pos->y) = static_cast<double>(-5000. + pos) / 5000.;
    if (mat_pos->x == 2 || mat_pos->x == 3 || mat_pos->x == 4)
        distCoeffs(mat_pos->x, mat_pos->y) = static_cast<double>(-500. + pos) / 5000.;
}

int main()
{
    // 读取相机内参、畸变系数
    const char *file_name = "out_calibration.yml";
    FileStorage fs_mv_in(file_name, FileStorage::READ);
    fs_mv_in["cameraMatrix"].isNone() ? void(0) : (fs_mv_in["cameraMatrix"] >> cameraMatrix);
    fs_mv_in["distCoeffs"].isNone() ? void(0) : (fs_mv_in["distCoeffs"] >> distCoeffs);

    int exposure = 10000;
    int gain = 64;
    int r_gain = 100;
    int g_gain = 100;
    int b_gain = 100;

    // 设置相机参数
    FileStorage fs_mv_set("out_para.yml", FileStorage::READ);
    if (fs_mv_set.isOpened())
    {
        fs_mv_set["exposure"].isNone() ? void(0) : (fs_mv_set["exposure"] >> exposure);
        fs_mv_set["gain"].isNone() ? void(0) : (fs_mv_set["gain"] >> gain);
        fs_mv_set["r_gain"].isNone() ? void(0) : (fs_mv_set["r_gain"] >> r_gain);
        fs_mv_set["g_gain"].isNone() ? void(0) : (fs_mv_set["g_gain"] >> g_gain);
        fs_mv_set["b_gain"].isNone() ? void(0) : (fs_mv_set["b_gain"] >> b_gain);
    }

    capture.set(CAMERA_MANUAL_EXPOSURE);
    capture.set(CAMERA_EXPOSURE, exposure);
    capture.set(CAMERA_GAIN, gain);
    capture.set(CAMERA_MANUAL_WB);
    capture.set(CAMERA_WB_RGAIN, r_gain);
    capture.set(CAMERA_WB_GGAIN, g_gain);
    capture.set(CAMERA_WB_BGAIN, b_gain);

    namedWindow("图像画面", WINDOW_NORMAL);
    namedWindow("控制面板", WINDOW_AUTOSIZE);
    Mat track_bar_img = Mat::zeros(Size(800, 1), CV_8UC1);

    vector<Point> matrix_pose; // 设置内参矩阵对应的参数位置
    vector<Point> dist_pose;   // 设置畸变系数对应的参数位置
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            matrix_pose.emplace_back(i, j);
    for (int i = 0; i < 6; i++)
        dist_pose.emplace_back(i, 0);

    // 设置滑动条
    createTrackbar("内参 (0, 0)", "控制面板", nullptr, 10000, cameraMatrixCallBack, &(matrix_pose.at(0)));
    setTrackbarPos("内参 (0, 0)", "控制面板", cameraMatrix(0, 0));
    createTrackbar("内参 (1, 1)", "控制面板", nullptr, 10000, cameraMatrixCallBack, &(matrix_pose.at(0)));
    setTrackbarPos("内参 (1, 1)", "控制面板", cameraMatrix(1, 1));
    createTrackbar("内参 (0, 2)", "控制面板", nullptr, 3000, cameraMatrixCallBack, &(matrix_pose.at(2)));
    setTrackbarPos("内参 (0, 2)", "控制面板", cameraMatrix(0, 2));
    createTrackbar("内参 (1, 2)", "控制面板", nullptr, 3000, cameraMatrixCallBack, &(matrix_pose.at(5)));
    setTrackbarPos("内参 (1, 2)", "控制面板", cameraMatrix(1, 2));
    createTrackbar("畸变 0", "控制面板", nullptr, 10000, distCoeffCallBack, &(dist_pose.at(0)));
    setTrackbarPos("畸变 0", "控制面板", distCoeffs(0, 0) * 5000 + 5000);
    createTrackbar("畸变 1", "控制面板", nullptr, 10000, distCoeffCallBack, &(dist_pose.at(1)));
    setTrackbarPos("畸变 1", "控制面板", distCoeffs(1, 0) * 5000 + 5000);
    createTrackbar("畸变 2", "控制面板", nullptr, 1000, distCoeffCallBack, &(dist_pose.at(2)));
    setTrackbarPos("畸变 2", "控制面板", distCoeffs(2, 0) * 5000 + 500);
    createTrackbar("畸变 3", "控制面板", nullptr, 1000, distCoeffCallBack, &(dist_pose.at(3)));
    setTrackbarPos("畸变 3", "控制面板", distCoeffs(3, 0) * 5000 + 500);
    createTrackbar("畸变 4", "控制面板", nullptr, 1000, distCoeffCallBack, &(dist_pose.at(4)));
    setTrackbarPos("畸变 4", "控制面板", distCoeffs(4, 0) * 5000 + 500);

    sleep(1);

    printf("Press the 's' key to save the parameters to the yaml file: \033[33m%s\033[0m\n", file_name);

    Mat frame;
    if (!capture.read(frame))
        return -1;
    resizeWindow("图像画面", Size(frame.cols * 0.8, frame.rows * 0.8));

    while (true)
    {
        if (!capture.read(frame))
            continue;

        // 图像矫正
        Mat map1, map2;
        initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), cameraMatrix, frame.size(), CV_32FC1, map1, map2);
        remap(frame, frame, map1, map2, INTER_NEAREST);

        // 绘制参照线
        for (int i = 0; i <= frame.cols; i += frame.cols / 10)
            line(frame, Point(i, 0), Point(i, frame.rows), Scalar(0, 0, 255), 1);
        for (int j = 0; j <= frame.rows; j += frame.rows / 10)
            line(frame, Point(0, j), Point(frame.cols, j), Scalar(0, 0, 255), 1);

        imshow("图像画面", frame);
        imshow("控制面板", track_bar_img);

        char c = waitKey(1);
        // 退出程序
        if (c == 27)
        {
            if (waitKey(0) == 27)
                break;
        }
        // 保存参数
        else if (c == 's')
        {
            FileStorage fs_mv_out(file_name, FileStorage::WRITE);
            fs_mv_out << "cameraMatrix" << cameraMatrix;
            fs_mv_out << "distCoeffs" << distCoeffs;

            printf("\033[32mSuccess to write the parameters into \"%s\"\033[0m\n", file_name);
            printf("                  ┌ %-5.4g, %-5.4g, %-5.4g ┐\n"
                   " -- cameraMatrix: │ %-5.4g, %-5.4g, %-5.4g │\n"
                   "                  └ %-5.4g, %-5.4g, %-5.4g ┘\n",
                   cameraMatrix(0, 0), cameraMatrix(0, 1), cameraMatrix(0, 2),
                   cameraMatrix(1, 0), cameraMatrix(1, 1), cameraMatrix(1, 2),
                   cameraMatrix(2, 0), cameraMatrix(2, 1), cameraMatrix(2, 2));
            printf("               ┌ %-8.5g ┐\n"
                   "               │ %-8.5g │\n"
                   " -- distCoeffs: │ %-8.5g │\n"
                   "               │ %-8.5g │\n"
                   "               └ %-8.5g ┘\n",
                   distCoeffs(0), distCoeffs(1), distCoeffs(2), distCoeffs(3), distCoeffs(4));
        }
    }

    return 0;
}