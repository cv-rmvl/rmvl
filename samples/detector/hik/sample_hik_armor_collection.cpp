#include <filesystem>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "rmvl/camera/hik_camera.h"
#include "rmvl/core/timer.hpp"
#include "rmvl/core/util.hpp"
#include "rmvl/detector/armor_detector.h"

namespace fs = std::filesystem;

/**
 * @brief 展示识别结果
 *
 * @param[in] src 原图像
 * @param[in] p_combo 指定装甲板
 */
static inline void draw(cv::Mat src, rm::combo::ptr p_combo) {
    // 角点
    const auto &corners = p_combo->corners();
    for (int i = 0; i < 4; ++i)
        cv::line(src, corners[i], corners[(i + 1) % 4], cv::Scalar(0, 255, 0));
}

const char *keys = "{ ? h help  |          | 帮助信息 }"
                   "{ i index   |0         | 第一帧图像下标 }"
                   "{ p prefix  |rmvl_data | 不带 \"/\" 后缀的导出路径，可使用递归路径，不存在会自动创建"
                   "\n\t\t如 \033[33m-p=datasets/0\033[0m 或 \033[33m-p=datasets/1\033[0m }"
                   "{ n num     |1000      | 图片数量 }"
                   "{ c color   |0         | 识别装甲板颜色 '\033[33m0\033[0m': 识别蓝色，"
                   "'\033[33m1\033[0m': 识别红色 }"
                   "{ w waitkey |20        | cv::cv::waitKey(?) }";

const char *help = "                      \033[34;1m使用说明\033[0m\n"
                   "本程序为装甲板 ROI 收集例程，用于收集装甲板数字部分的 ROI，\n"
                   "程序终端会显示当前收集并导出的 ROI 图片序号，并且包含 2 个图\n"
                   "形窗口一个是标记出装甲板的相机捕获的画面，另一个是 ROI 原图\n"
                   "与便于观察的 ROI 二值图";

int main(int argc, char *argv[]) {
    // 命令行参数初始化
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        printf("%s\n", help);
        exit(0);
    }

    // 获取命令行参数
    auto path = parser.get<std::string>("prefix");
    auto first_idx = parser.get<int>("index");
    auto num = parser.get<int>("num");
    auto color = parser.get<int>("color") == 0 ? rm::PixChannel::BLUE : rm::PixChannel::RED;
    auto waitkey = parser.get<int>("waitkey");

    if (!fs::exists(path)) {
        if (fs::create_directories(path))
            printf("创建新文件夹 \033[33m\"%s\"\033[0m...\n", path.c_str());
        else {
            printf("\033[33m\"%s\"\033[0m 文件夹不存在，重新创建失败...\n", path.c_str());
            return -1;
        }
    } else
        printf("已发现 \033[33m\"%s\"\033[0m 文件夹\n", path.c_str());

    // 相机初始化
    rm::HikCamera capture(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV));
    if (!capture.isOpened()) {
        printf("相机打开失败\n");
        return -1;
    }
    cv::FileStorage camera_param("out_para.yml", cv::FileStorage::READ);

    int exposure = 3000;
    int gain = 8;
    int r_gain = 1500;
    int g_gain = 1500;
    int b_gain = 1500;

    // 设置相机参数
    cv::FileStorage fs_mv_set("out_para.yml", cv::FileStorage::READ);
    if (fs_mv_set.isOpened()) {
        fs_mv_set["exposure"].isNone() ? void(0) : (fs_mv_set["exposure"] >> exposure);
        fs_mv_set["gain"].isNone() ? void(0) : (fs_mv_set["gain"] >> gain);
        fs_mv_set["r_gain"].isNone() ? void(0) : (fs_mv_set["r_gain"] >> r_gain);
        fs_mv_set["g_gain"].isNone() ? void(0) : (fs_mv_set["g_gain"] >> g_gain);
        fs_mv_set["b_gain"].isNone() ? void(0) : (fs_mv_set["b_gain"] >> b_gain);
    }

    capture.set(rm::CAMERA_MANUAL_EXPOSURE);
    capture.set(rm::CAMERA_EXPOSURE, exposure);
    capture.set(rm::CAMERA_GAIN, gain);
    capture.set(rm::CAMERA_MANUAL_WB);
    capture.set(rm::CAMERA_WB_RGAIN, r_gain);
    capture.set(rm::CAMERA_WB_GGAIN, g_gain);
    capture.set(rm::CAMERA_WB_BGAIN, b_gain);

    auto p_detector = rm::ArmorDetector::make_detector();
    std::vector<rm::group::ptr> groups;

    cv::namedWindow("装甲板收集", cv::WINDOW_NORMAL);
    cv::resizeWindow("装甲板收集", cv::Size(1000, 800));
    cv::namedWindow("ROI 图像", cv::WINDOW_NORMAL);
    cv::resizeWindow("ROI 图像", cv::Size(800, 400));

    cv::Mat src;
    int index = 0;
    while (index < num) {
        if (!capture.read(src))
            RMVL_Error(RMVL_StsError, "Fail to read the image.");
        // 识别
        auto info = p_detector->detect(groups, src, color, rm::ImuData(), rm::Timer::now());
        const auto &combos = info.combos;
        if (combos.size() > 1)
            WARNING_("当前识别到多于 1 个装甲板：识别到 %zu 个", combos.size());
        // ROI截取图像
        cv::Mat roi_img;
        if (!combos.empty()) {
            roi_img = rm::Armor::getNumberROI(src, rm::Armor::cast(combos.front()));
            // 显示效果
            draw(src, combos.front());
        }
        // 保存图像
        if (!roi_img.empty()) {
            std::string file_name = path + "/" + std::to_string(index + first_idx) + ".png";
            INFO_("保存图像至 %s", file_name.c_str());
            if (!imwrite(file_name, roi_img))
                ERROR_("保存图像失败");
            cv::imshow("ROI 图像", roi_img);
            index++;
        }
        cv::imshow("装甲板收集", src);
        if (cv::waitKey(waitkey) == 27)
            if (cv::waitKey(0) == 27)
                exit(0);
    }
    putText(src, "Collection Over!", cv::Point(100, 200), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
    cv::imshow("装甲板收集", src);
    PASS_("收集完毕，按任意键以退出程序");
    cv::waitKey(0);
    return 0;
}
