#include <iostream>

#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/camera/mv_camera.h"
#include "rmvl/core/util.hpp"

constexpr const char *usage =
    " 示例命令行，用于从实时提要进行校准。\n"
    " $ \033[33mrmvl_mv_auto_calib -w=4 -h=5 -s=0.025 -o=camera.yml\033[0m\n";

constexpr const char *liveCaptureHelp =
    "当使用摄像头的实时视频作为输入时，可以使用以下热键:\n"
    "  <ESC>, 'q' - 退出程序\n"
    "  'g' - 开始捕捉图像\n"
    "  'u' - 打开/关闭不失真开关\n";

static void help() {
    printf("相机标定例程 (Copy from OpenCV):\n"
           "用法: rmvl_mv_auto_calib\n"
           "     -help, -?                \033[32m# 显示帮助信息\033[0m\n"
           "     -w=<board_width>         \033[32m# 每一个板尺寸内角的数目 (格子数 - 1)\033[0m\n"
           "     -h=<board_height>        \033[32m# 每一个板尺寸内角的数目 (格子数 - 1)\033[0m\n"
           "     [-c]                     \033[32m# 采用自定的相机光学参数，使用当前目录下的 out_param.yml 文件\033[0m\n"
           "     [-pt=<pattern>]          \033[32m# 图案类型: 棋盘 (chessboard) 或圆形网格 (circles, acircles)\033[0m\n"
           "     [-n=<number_of_frames>]  \033[32m# 用于校准的帧数\033[0m\n"
           "                              \033[32m# (如果没有指定，它将被设置为实际可用的板视图数)\033[0m\n"
           "     [-d=<delay>]             \033[32m# 在捕捉下一个视图的后续尝试之间的最小延迟数 (单位：毫秒)\033[0m\n"
           "                              \033[32m# (仅用于视频捕捉)\033[0m\n"
           "     [-s=<squareSize>]        \033[32m# 用户定义的单位正方形大小 (默认为1)\033[0m\n"
           "     [-o=<out_camera_params>] \033[32m# 内参数的输出文件名\033[0m\n"
           "     [-zt]                    \033[32m# 假设切向失真为零\033[0m\n"
           "     [-a=<aspectRatio>]       \033[32m# 固定长宽比(fx/fy)\033[0m\n"
           "     [-p]                     \033[32m# 把主点固定在中心\033[0m\n"
           "     [-v]                     \033[32m# 围绕水平轴翻转捕获的图像\033[0m\n"
           "     [-ws=<number_of_pixel>]  \033[32m# cornerSubPix 搜索窗口的一半 (默认为 11)\033[0m\n"
           "     [-dt=<distance>]         \033[32m# 校准网格的左上角和右上角之间的实际距离。如果指定此参数，\033[0m\n"
           "                              \033[32m# 将使用更精确的校准方法，该方法可能更好地用于不准确的，粗\033[0m\n"
           "                              \033[32m# 略的平面目标。\033[0m\n"
           "\n");
    printf("\n%s", usage);
    printf("\n%s", liveCaptureHelp);
}

enum {
    DETECTION = 0,
    CAPTURING = 1,
    CALIBRATED = 2
};
enum Pattern {
    CHESSBOARD,
    CIRCLES_GRID,
    ASYMMETRIC_CIRCLES_GRID
};

static double computeReprojectionErrors(
    const std::vector<std::vector<cv::Point3f>> &objectPoints,
    const std::vector<std::vector<cv::Point2f>> &imagePoints,
    const std::vector<cv::Mat> &rvecs, const std::vector<cv::Mat> &tvecs,
    const cv::Mat &cameraMatrix, const cv::Mat &distCoeffs,
    std::vector<float> &perViewErrors) {
    std::vector<cv::Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for (i = 0; i < (int)objectPoints.size(); i++) {
        projectPoints(cv::Mat(objectPoints[i]), rvecs[i], tvecs[i],
                      cameraMatrix, distCoeffs, imagePoints2);
        err = norm(cv::Mat(imagePoints[i]), cv::Mat(imagePoints2), cv::NORM_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float)std::sqrt(err * err / n);
        totalErr += err * err;
        totalPoints += n;
    }

    return std::sqrt(totalErr / totalPoints);
}

static void calcChessboardCorners(cv::Size boardSize, float squareSize, std::vector<cv::Point3f> &corners, Pattern patternType = CHESSBOARD) {
    corners.resize(0);

    switch (patternType) {
    case CHESSBOARD:
    case CIRCLES_GRID:
        for (int i = 0; i < boardSize.height; i++)
            for (int j = 0; j < boardSize.width; j++)
                corners.push_back(cv::Point3f(float(j * squareSize),
                                              float(i * squareSize), 0));
        break;

    case ASYMMETRIC_CIRCLES_GRID:
        for (int i = 0; i < boardSize.height; i++)
            for (int j = 0; j < boardSize.width; j++)
                corners.push_back(cv::Point3f(float((2 * j + i % 2) * squareSize),
                                              float(i * squareSize), 0));
        break;

    default:
        RMVL_Error(RMVL_StsBadArg, "未知模式类型\n");
    }
}

static bool runCalibration(std::vector<std::vector<cv::Point2f>> imagePoints,
                           cv::Size imageSize, cv::Size boardSize, Pattern patternType,
                           float squareSize, float aspectRatio,
                           float grid_width, bool release_object,
                           int flags, cv::Mat &cameraMatrix, cv::Mat &distCoeffs,
                           std::vector<cv::Mat> &rvecs, std::vector<cv::Mat> &tvecs,
                           std::vector<float> &reprojErrs,
                           std::vector<cv::Point3f> &newObjPoints,
                           double &totalAvgErr) {
    cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
    if (flags & cv::CALIB_FIX_ASPECT_RATIO)
        cameraMatrix.at<double>(0, 0) = aspectRatio;

    distCoeffs = cv::Mat::zeros(8, 1, CV_64F);

    std::vector<std::vector<cv::Point3f>> objectPoints(1);
    calcChessboardCorners(boardSize, squareSize, objectPoints[0], patternType);
    objectPoints[0][boardSize.width - 1].x = objectPoints[0][0].x + grid_width;
    newObjPoints = objectPoints[0];

    objectPoints.resize(imagePoints.size(), objectPoints[0]);

    double rms;
    int iFixedPoint = -1;
    if (release_object)
        iFixedPoint = boardSize.width - 1;
    rms = calibrateCameraRO(objectPoints, imagePoints, imageSize, iFixedPoint,
                            cameraMatrix, distCoeffs, rvecs, tvecs, newObjPoints,
                            flags | cv::CALIB_FIX_K3 | cv::CALIB_USE_LU);
    printf("RMS error reported by calibrateCamera: %g\n", rms);

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    if (release_object) {
        std::cout << "New board corners: " << std::endl;
        std::cout << newObjPoints[0] << std::endl;
        std::cout << newObjPoints[boardSize.width - 1] << std::endl;
        std::cout << newObjPoints[boardSize.width * (boardSize.height - 1)] << std::endl;
        std::cout << newObjPoints.back() << std::endl;
    }

    objectPoints.clear();
    objectPoints.resize(imagePoints.size(), newObjPoints);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

static void saveCameraParams(const std::string &filename, const cv::Mat &cameraMatrix,
                             const cv::Mat &distCoeffs, double totalAvgErr) {
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);

    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs << "avg_reprojection_error" << totalAvgErr;
}

static bool runAndSave(const std::string &outputFilename, const std::vector<std::vector<cv::Point2f>> &imagePoints,
                       cv::Size imageSize, cv::Size boardSize, Pattern patternType, float squareSize,
                       float grid_width, bool release_object, float aspectRatio, int flags,
                       cv::Mat &cameraMatrix, cv::Mat &distCoeffs) {
    std::vector<cv::Mat> rvecs, tvecs;
    std::vector<float> reprojErrs;
    double totalAvgErr = 0;
    std::vector<cv::Point3f> newObjPoints;

    bool ok = runCalibration(imagePoints, imageSize, boardSize, patternType, squareSize,
                             aspectRatio, grid_width, release_object, flags, cameraMatrix, distCoeffs,
                             rvecs, tvecs, reprojErrs, newObjPoints, totalAvgErr);
    printf("%s. avg reprojection error = %.7f\n",
           ok ? "Calibration succeeded" : "Calibration failed",
           totalAvgErr);

    if (ok)
        saveCameraParams(outputFilename, cameraMatrix, distCoeffs, totalAvgErr);
    return ok;
}

const char *keys = "{ ? help    |                       | }"
                   "{ w         |                       | }"
                   "{ h         |                       | }"
                   "{ c         |                       | }"
                   "{ pt        | chessboard            | }"
                   "{ n         | 40                    | }"
                   "{ d         | 5000                  | }"
                   "{ s         | 1                     | }"
                   "{ o         | out_calibration.yml   | }"
                   "{ zt        |                       | }"
                   "{ a         |                       | }"
                   "{ p         |                       | }"
                   "{ v         |                       | }"
                   "{ ws        |11                     | }"
                   "{ dt        |                       | }";

int main(int argc, char *argv[]) {
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        help();
        return 0;
    }

    cv::Size imageSize;
    float aspectRatio = 1;
    cv::Mat cameraMatrix, distCoeffs;
    std::string outputFilename;

    bool undistortImage = false;
    int flags = 0;
    rm::MvCamera capture(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV));

    int exposure{10000};
    int gain{128};
    int r_gain{100};
    int g_gain{100};
    int b_gain{100};

    // 相机光学参数配置
    if (parser.has("c")) {
        cv::FileStorage fs("out_para.yml", cv::FileStorage::READ);
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
    capture.set(rm::CameraProperties::wb_rgain, r_gain);
    capture.set(rm::CameraProperties::wb_ggain, g_gain);
    capture.set(rm::CameraProperties::wb_bgain, b_gain);

    bool flipVertical;
    clock_t prevTimestamp = 0;
    int mode = DETECTION;
    std::vector<std::vector<cv::Point2f>> imagePoints;
    Pattern pattern = CHESSBOARD;

    cv::Size boardSize;
    boardSize.width = parser.get<int>("w");
    boardSize.height = parser.get<int>("h");
    if (parser.has("pt")) {
        std::string val = parser.get<std::string>("pt");
        if (val == "circles")
            pattern = CIRCLES_GRID;
        else if (val == "acircles")
            pattern = ASYMMETRIC_CIRCLES_GRID;
        else if (val == "chessboard")
            pattern = CHESSBOARD;
        else
            return fprintf(stderr, "无效的图案类型: 必须是棋盘 chessboard 或圆形 circles, acircles\n"), -1;
    }
    float squareSize = parser.get<float>("s");
    int nframes = parser.get<int>("n");
    int delay = parser.get<int>("d");
    if (parser.has("a")) {
        flags |= cv::CALIB_FIX_ASPECT_RATIO;
        aspectRatio = parser.get<float>("a");
    }
    if (parser.has("zt"))
        flags |= cv::CALIB_ZERO_TANGENT_DIST;
    if (parser.has("p"))
        flags |= cv::CALIB_FIX_PRINCIPAL_POINT;
    flipVertical = parser.has("v");
    if (parser.has("o"))
        outputFilename = parser.get<std::string>("o");
    int winSize = parser.get<int>("ws");
    float grid_width = squareSize * (boardSize.width - 1);
    bool release_object = false;
    if (parser.has("dt")) {
        grid_width = parser.get<float>("dt");
        release_object = true;
    }
    if (!parser.check()) {
        help();
        parser.printErrors();
        return -1;
    }
    if (squareSize <= 0)
        return fprintf(stderr, "无效的标定板方宽度\n"), -1;
    if (nframes <= 3)
        return printf("无效的图像数量\n"), -1;
    if (aspectRatio <= 0)
        return printf("无效的纵横比\n"), -1;
    if (delay <= 0)
        return printf("无效的延迟\n"), -1;
    if (boardSize.width <= 0)
        return fprintf(stderr, "无效的标定板宽\n"), -1;
    if (boardSize.height <= 0)
        return fprintf(stderr, "无效的标定板高\n"), -1;

    if (capture.isOpened())
        printf("%s", liveCaptureHelp);

    namedWindow("图像画面", cv::WINDOW_NORMAL);

    while (true) {
        cv::Mat view;
        bool blink = false;

        if (capture.isOpened()) {
            cv::Mat view0;
            capture.read(view0);
            if (view0.type() == CV_8UC1)
                cv::cvtColor(view0, view, cv::COLOR_GRAY2BGR);
            else
                view0.copyTo(view);
        }

        if (view.empty()) {
            if (imagePoints.size() > 0)
                runAndSave(outputFilename, imagePoints, imageSize,
                           boardSize, pattern, squareSize, grid_width, release_object, aspectRatio,
                           flags, cameraMatrix, distCoeffs);
            continue;
        }

        imageSize = view.size();

        if (flipVertical)
            flip(view, view, 0);

        std::vector<cv::Point2f> pointbuf;
        cv::Mat viewGray;
        cvtColor(view, viewGray, cv::COLOR_BGR2GRAY);

        bool found;
        switch (pattern) {
        case CHESSBOARD:
            found = findChessboardCorners(viewGray, boardSize, pointbuf);
            break;
        case CIRCLES_GRID:
            found = findCirclesGrid(viewGray, boardSize, pointbuf);
            break;
        case ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid(viewGray, boardSize, pointbuf, cv::CALIB_CB_ASYMMETRIC_GRID);
            break;
        default:
            return fprintf(stderr, "Unknown pattern type\n"), -1;
        }

        // improve the found corners' coordinate accuracy
        if (pattern == CHESSBOARD && found)
            cornerSubPix(viewGray, pointbuf, cv::Size(winSize, winSize),
                         cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));

        if (mode == CAPTURING && found &&
            (!capture.isOpened() || clock() - prevTimestamp > delay * 1e-3 * CLOCKS_PER_SEC)) {
            imagePoints.push_back(pointbuf);
            prevTimestamp = clock();
            blink = capture.isOpened();
        }

        if (found)
            drawChessboardCorners(view, boardSize, cv::Mat(pointbuf), found);

        std::string msg = mode == CAPTURING ? "100/100" : mode == CALIBRATED ? "Calibrated"
                                                                             : "Press 'g' to start";
        int baseLine = 0;
        cv::Size textSize = cv::getTextSize(msg, 1, 1, 1, &baseLine);
        cv::Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

        if (mode == CAPTURING) {
            if (undistortImage)
                msg = cv::format("%d/%d Undist", (int)imagePoints.size(), nframes);
            else
                msg = cv::format("%d/%d", (int)imagePoints.size(), nframes);
        }

        putText(view, msg, textOrigin, 1, 1,
                mode != CALIBRATED ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0));

        if (blink)
            bitwise_not(view, view);

        if (mode == CALIBRATED && undistortImage) {
            cv::Mat temp = view.clone();
            undistort(temp, view, cameraMatrix, distCoeffs);
        }

        imshow("图像画面", view);
        char key = static_cast<char>(cv::waitKey(1));

        if (key == 27)
            break;

        if (key == 'u' && mode == CALIBRATED)
            undistortImage = !undistortImage;

        if (capture.isOpened() && key == 'g') {
            mode = CAPTURING;
            imagePoints.clear();
        }

        if (mode == CAPTURING && imagePoints.size() >= (unsigned)nframes) {
            if (runAndSave(outputFilename, imagePoints, imageSize, boardSize, pattern, squareSize,
                           grid_width, release_object, aspectRatio, flags, cameraMatrix, distCoeffs))
                mode = CALIBRATED;
            else
                mode = DETECTION;
            if (!capture.isOpened())
                break;
        }
    }

    return 0;
}
