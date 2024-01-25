#include <iostream>

#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include "rmvl/camera/mv_camera.h"

using namespace rm;
using namespace std;
using namespace cv;

const char *usage =
    " 示例命令行，用于从实时提要进行校准。\n"
    " $ \033[33mcalibration -w=4 -h=5 -s=0.025 -o=camera.yml\033[0m\n";

const char *liveCaptureHelp =
    "当使用摄像头的实时视频作为输入时，可以使用以下热键:\n"
    "  <ESC>, 'q' - 退出程序\n"
    "  'g' - 开始捕捉图像\n"
    "  'u' - 打开/关闭不失真开关\n";

static void help()
{
    printf("相机标定例程 (Copy from OpenCV):\n"
           "用法: calibration\n"
           "     -help                    \033[32m# 显示帮助信息\033[0m\n"
           "     -w=<board_width>         \033[32m# 每一个板尺寸内角的数目 (格子数 - 1)\033[0m\n"
           "     -h=<board_height>        \033[32m# 每一个板尺寸内角的数目 (格子数 - 1)\033[0m\n"
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
           "     [-V]                     \033[32m# 使用视频文件，而不是图像列表，使用 [input_data]\033[0m\n"
           "                              \033[32m# 字符串作为视频文件名\033[0m\n"
           "     [-su]                    \033[32m# 显示校正后未失真的图像\033[0m\n"
           "     [-ws=<number_of_pixel>]  \033[32m# cornerSubPix 搜索窗口的一半 (默认为 11)\033[0m\n"
           "     [-dt=<distance>]         \033[32m# 校准网格的左上角和右上角之间的实际距离。如果指定此参数，\033[0m\n"
           "                              \033[32m# 将使用更精确的校准方法，该方法可能更好地用于不准确的，粗\033[0m\n"
           "                              \033[32m# 略的平面目标。\033[0m\n"
           "     [input_data]             \033[32m# 输入数据，为下列之一:\033[0m\n"
           "                              \033[32m#  - 文本文件，包含板子的图像列表，文本文件可以用\033[0m\n"
           "                              \033[32m#    imagelist_creator 生成\033[0m\n"
           "                              \033[32m#  - 包含单板视频的视频文件名称\033[0m\n"
           "                              \033[32m# 如果未指定 input_data, 则使用来自摄像机的实时视图\033[0m\n"
           "\n");
    printf("\n%s", usage);
    printf("\n%s", liveCaptureHelp);
}

enum
{
    DETECTION = 0,
    CAPTURING = 1,
    CALIBRATED = 2
};
enum Pattern
{
    CHESSBOARD,
    CIRCLES_GRID,
    ASYMMETRIC_CIRCLES_GRID
};

static double computeReprojectionErrors(
    const vector<vector<Point3f>> &objectPoints,
    const vector<vector<Point2f>> &imagePoints,
    const vector<Mat> &rvecs, const vector<Mat> &tvecs,
    const Mat &cameraMatrix, const Mat &distCoeffs,
    vector<float> &perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for (i = 0; i < (int)objectPoints.size(); i++)
    {
        projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i],
                      cameraMatrix, distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), NORM_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float)std::sqrt(err * err / n);
        totalErr += err * err;
        totalPoints += n;
    }

    return std::sqrt(totalErr / totalPoints);
}

static void calcChessboardCorners(Size boardSize, float squareSize, vector<Point3f> &corners, Pattern patternType = CHESSBOARD)
{
    corners.resize(0);

    switch (patternType)
    {
    case CHESSBOARD:
    case CIRCLES_GRID:
        for (int i = 0; i < boardSize.height; i++)
            for (int j = 0; j < boardSize.width; j++)
                corners.push_back(Point3f(float(j * squareSize),
                                          float(i * squareSize), 0));
        break;

    case ASYMMETRIC_CIRCLES_GRID:
        for (int i = 0; i < boardSize.height; i++)
            for (int j = 0; j < boardSize.width; j++)
                corners.push_back(Point3f(float((2 * j + i % 2) * squareSize),
                                          float(i * squareSize), 0));
        break;

    default:
        CV_Error(Error::StsBadArg, "未知模式类型\n");
    }
}

static bool runCalibration(vector<vector<Point2f>> imagePoints,
                           Size imageSize, Size boardSize, Pattern patternType,
                           float squareSize, float aspectRatio,
                           float grid_width, bool release_object,
                           int flags, Mat &cameraMatrix, Mat &distCoeffs,
                           vector<Mat> &rvecs, vector<Mat> &tvecs,
                           vector<float> &reprojErrs,
                           vector<Point3f> &newObjPoints,
                           double &totalAvgErr)
{
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if (flags & CALIB_FIX_ASPECT_RATIO)
        cameraMatrix.at<double>(0, 0) = aspectRatio;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f>> objectPoints(1);
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
                            flags | CALIB_FIX_K3 | CALIB_USE_LU);
    printf("RMS error reported by calibrateCamera: %g\n", rms);

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    if (release_object)
    {
        cout << "New board corners: " << endl;
        cout << newObjPoints[0] << endl;
        cout << newObjPoints[boardSize.width - 1] << endl;
        cout << newObjPoints[boardSize.width * (boardSize.height - 1)] << endl;
        cout << newObjPoints.back() << endl;
    }

    objectPoints.clear();
    objectPoints.resize(imagePoints.size(), newObjPoints);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

static void saveCameraParams(const string &filename, const Mat &cameraMatrix,
                             const Mat &distCoeffs, double totalAvgErr)
{
    FileStorage fs(filename, FileStorage::WRITE);

    fs << "cameraMatrix" << cameraMatrix;
    fs << "distCoeffs" << distCoeffs;
    fs << "avg_reprojection_error" << totalAvgErr;
}

static bool readStringList(const string &filename, vector<string> &l)
{
    l.resize(0);
    FileStorage fs(filename, FileStorage::READ);
    if (!fs.isOpened())
        return false;
    size_t dir_pos = filename.rfind('/');
    if (dir_pos == string::npos)
        dir_pos = filename.rfind('\\');
    FileNode n = fs.getFirstTopLevelNode();
    if (n.type() != FileNode::SEQ)
        return false;
    FileNodeIterator it = n.begin(), it_end = n.end();
    for (; it != it_end; ++it)
    {
        string fname = (string)*it;
        if (dir_pos != string::npos)
        {
            string fpath = samples::findFile(filename.substr(0, dir_pos + 1) + fname, false);
            if (fpath.empty())
            {
                fpath = samples::findFile(fname);
            }
            fname = fpath;
        }
        else
        {
            fname = samples::findFile(fname);
        }
        l.push_back(fname);
    }
    return true;
}

static bool runAndSave(const string &outputFilename, const vector<vector<Point2f>> &imagePoints,
                       Size imageSize, Size boardSize, Pattern patternType, float squareSize,
                       float grid_width, bool release_object, float aspectRatio, int flags,
                       Mat &cameraMatrix, Mat &distCoeffs)
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;
    vector<Point3f> newObjPoints;

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

const char *keys = "{ help          |                       | }"
                   "{ w             |                       | }"
                   "{ h             |                       | }"
                   "{ pt            | chessboard            | }"
                   "{ n             | 40                    | }"
                   "{ d             | 5000                  | }"
                   "{ s             | 1                     | }"
                   "{ o             | out_calibration.yml   | }"
                   "{ zt            |                       | }"
                   "{ a             |                       | }"
                   "{ p             |                       | }"
                   "{ v             |                       | }"
                   "{ V             |                       | }"
                   "{ su            |                       | }"
                   "{ ws            |11                     | }"
                   "{ dt            |                       | }"
                   "{ @input_data   |0                      | }";

int main(int argc, char **argv)
{
    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        help();
        return 0;
    }

    Size imageSize;
    float aspectRatio = 1;
    Mat cameraMatrix, distCoeffs;
    string outputFilename;
    string inputFilename = "";

    bool undistortImage = false;
    int flags = 0;
    MvCamera capture(rm::CameraConfig::create(rm::GrabMode::Continuous, RetrieveMode::OpenCV));

    FileStorage fs("out_para.yml", FileStorage::READ);

    int exposure = 10000;
    int gain = 128;
    int r_gain = 100;
    int g_gain = 100;
    int b_gain = 100;

    fs["exposure"].isNone() ? void(0) : (fs["exposure"] >> exposure);
    fs["gain"].isNone() ? void(0) : (fs["gain"] >> gain);
    fs["r_gain"].isNone() ? void(0) : (fs["r_gain"] >> r_gain);
    fs["g_gain"].isNone() ? void(0) : (fs["g_gain"] >> g_gain);
    fs["b_gain"].isNone() ? void(0) : (fs["b_gain"] >> b_gain);

    capture.set(CAMERA_MANUAL_EXPOSURE);
    capture.set(CAMERA_EXPOSURE, exposure);
    capture.set(CAMERA_GAIN, gain);
    capture.set(CAMERA_MANUAL_WB);
    capture.set(CAMERA_WB_RGAIN, r_gain);
    capture.set(CAMERA_WB_GGAIN, g_gain);
    capture.set(CAMERA_WB_BGAIN, b_gain);

    bool flipVertical;
    bool showUndistorted;
    bool videofile;
    clock_t prevTimestamp = 0;
    int mode = DETECTION;
    int cameraId = 0;
    vector<vector<Point2f>> imagePoints;
    vector<string> imageList;
    Pattern pattern = CHESSBOARD;

    Size boardSize;
    boardSize.width = parser.get<int>("w");
    boardSize.height = parser.get<int>("h");
    if (parser.has("pt"))
    {
        string val = parser.get<string>("pt");
        if (val == "circles")
            pattern = CIRCLES_GRID;
        else if (val == "acircles")
            pattern = ASYMMETRIC_CIRCLES_GRID;
        else if (val == "chessboard")
            pattern = CHESSBOARD;
        else
            return fprintf(stderr, "无效的图案类型: 必须是棋盘或圆形\n"), -1;
    }
    float squareSize = parser.get<float>("s");
    int nframes = parser.get<int>("n");
    int delay = parser.get<int>("d");
    if (parser.has("a"))
    {
        flags |= CALIB_FIX_ASPECT_RATIO;
        aspectRatio = parser.get<float>("a");
    }
    if (parser.has("zt"))
        flags |= CALIB_ZERO_TANGENT_DIST;
    if (parser.has("p"))
        flags |= CALIB_FIX_PRINCIPAL_POINT;
    flipVertical = parser.has("v");
    videofile = parser.has("V");
    if (parser.has("o"))
        outputFilename = parser.get<string>("o");
    showUndistorted = parser.has("su");
    if (isdigit(parser.get<string>("@input_data")[0]))
        cameraId = parser.get<int>("@input_data");
    else
        inputFilename = parser.get<string>("@input_data");
    int winSize = parser.get<int>("ws");
    float grid_width = squareSize * (boardSize.width - 1);
    bool release_object = false;
    if (parser.has("dt"))
    {
        grid_width = parser.get<float>("dt");
        release_object = true;
    }
    if (!parser.check())
    {
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

    if (!inputFilename.empty())
        if (!videofile && readStringList(samples::findFile(inputFilename), imageList))
            mode = CAPTURING;

    if (!capture.isOpened() && imageList.empty())
        return fprintf(stderr, "Could not initialize video (%d) capture\n", cameraId), -2;
    if (!imageList.empty())
        nframes = (int)imageList.size();
    if (capture.isOpened())
        printf("%s", liveCaptureHelp);

    namedWindow("图像画面", WINDOW_NORMAL);

    for (int i = 0;; i++)
    {
        Mat view, viewGray;
        bool blink = false;

        if (capture.isOpened())
        {
            Mat view0;
            capture.read(view0);
            view0.copyTo(view);
        }
        else if (i < (int)imageList.size())
            view = imread(imageList[i], 1);

        if (view.empty())
        {
            cout << "dshfaldf" << endl;
            if (imagePoints.size() > 0)
                runAndSave(outputFilename, imagePoints, imageSize,
                           boardSize, pattern, squareSize, grid_width, release_object, aspectRatio,
                           flags, cameraMatrix, distCoeffs);
            continue;
        }

        imageSize = view.size();

        if (flipVertical)
            flip(view, view, 0);

        vector<Point2f> pointbuf;
        cvtColor(view, viewGray, COLOR_BGR2GRAY);

        bool found;
        switch (pattern)
        {
        case CHESSBOARD:
            found = findChessboardCorners(view, boardSize, pointbuf,
                                          CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
            break;
        case CIRCLES_GRID:
            found = findCirclesGrid(view, boardSize, pointbuf);
            break;
        case ASYMMETRIC_CIRCLES_GRID:
            found = findCirclesGrid(view, boardSize, pointbuf, CALIB_CB_ASYMMETRIC_GRID);
            break;
        default:
            return fprintf(stderr, "Unknown pattern type\n"), -1;
        }

        // improve the found corners' coordinate accuracy
        if (pattern == CHESSBOARD && found)
            cornerSubPix(viewGray, pointbuf, Size(winSize, winSize),
                         Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));

        if (mode == CAPTURING && found &&
            (!capture.isOpened() || clock() - prevTimestamp > delay * 1e-3 * CLOCKS_PER_SEC))
        {
            imagePoints.push_back(pointbuf);
            prevTimestamp = clock();
            blink = capture.isOpened();
        }

        if (found)
            drawChessboardCorners(view, boardSize, Mat(pointbuf), found);

        string msg = mode == CAPTURING ? "100/100" : mode == CALIBRATED ? "Calibrated"
                                                                        : "Press 'g' to start";
        int baseLine = 0;
        Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
        Point textOrigin(view.cols - 2 * textSize.width - 10, view.rows - 2 * baseLine - 10);

        if (mode == CAPTURING)
        {
            if (undistortImage)
                msg = cv::format("%d/%d Undist", (int)imagePoints.size(), nframes);
            else
                msg = cv::format("%d/%d", (int)imagePoints.size(), nframes);
        }

        putText(view, msg, textOrigin, 1, 1,
                mode != CALIBRATED ? Scalar(0, 0, 255) : Scalar(0, 255, 0));

        if (blink)
            bitwise_not(view, view);

        if (mode == CALIBRATED && undistortImage)
        {
            Mat temp = view.clone();
            undistort(temp, view, cameraMatrix, distCoeffs);
        }

        imshow("图像画面", view);
        char key = static_cast<char>(waitKey(capture.isOpened() ? 1 : 30));

        if (key == 27)
            break;

        if (key == 'u' && mode == CALIBRATED)
            undistortImage = !undistortImage;

        if (capture.isOpened() && key == 'g')
        {
            mode = CAPTURING;
            imagePoints.clear();
        }

        if (mode == CAPTURING && imagePoints.size() >= (unsigned)nframes)
        {
            if (runAndSave(outputFilename, imagePoints, imageSize, boardSize, pattern, squareSize,
                           grid_width, release_object, aspectRatio, flags, cameraMatrix, distCoeffs))
                mode = CALIBRATED;
            else
                mode = DETECTION;
            if (!capture.isOpened())
                break;
        }
    }

    if (!capture.isOpened() && showUndistorted)
    {
        Mat view, rview, map1, map2;
        initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
                                getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
                                imageSize, CV_16SC2, map1, map2);

        for (size_t i = 0; i < imageList.size(); i++)
        {
            view = imread(imageList[i], 1);
            if (view.empty())
                continue;
            // undistort( view, rview, cameraMatrix, distCoeffs, cameraMatrix );
            remap(view, rview, map1, map2, INTER_LINEAR);
            imshow("图像画面", rview);
            char c = static_cast<char>(waitKey());
            if (c == 27 || c == 'q' || c == 'Q')
                break;
        }
    }

    return 0;
}
