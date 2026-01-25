#include <iostream>
#include <thread>

#include "rmvl/camera/mv_camera.h"
#include "rmvl/core/timer.hpp"
#include "rmvl/core/util.hpp"
#include "rmvl/detector/armor_detector.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>

using namespace std::chrono_literals;

static int wait_time = 1;
static int collect_num = 2000;

static rm::MvCamera::ptr capture;                            // 相机
static auto p_detector = rm::ArmorDetector::make_detector(); // 识别模块
static cv::Mat frame;                                        // 帧图像
static std::vector<rm::group::ptr> groups;                   // 序列组列表
static cv::Mat armor_samples;                                // 装甲板信息样本
static cv::Mat armor_responses;                              // 装甲板响应/标签

void collect(rm::PixChannel color, rm::ArmorSizeType type, int begin_idx) {
    for (int idx = 0; idx < collect_num; ++idx) {
        capture->read(frame);
        if (frame.empty())
            RMVL_Error(RMVL_StsBadSize, "frame is empty, something wrong with the camera.");
        auto info = p_detector->detect(groups, frame, color, rm::ImuData(), rm::Timer::now());
        const auto &combos = info.combos;
        if (combos.size() != 1) {
            if (combos.size() > 1)
                WARNING_("Size of the combos is greater than 1, size = %zu", combos.size());
            idx--;
        } else {
            auto p_armor = rm::Armor::cast(combos.front());
            // 收集数据
            armor_samples.at<float>(begin_idx + idx, 0) = p_armor->getComboRatio();  // 装甲板长宽比
            armor_samples.at<float>(begin_idx + idx, 1) = p_armor->getWidthRatio();  // 灯条宽度比
            armor_samples.at<float>(begin_idx + idx, 2) = p_armor->getLengthRatio(); // 灯条长度比
            armor_responses.at<int>(begin_idx + idx) = type == rm::ArmorSizeType::SMALL ? 1 : 2;

            const auto &corners = p_armor->corners();
            for (int i = 0; i < 4; ++i)
                line(frame, corners[i], corners[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }
        putText(frame, std::to_string(idx), cv::Point(20, 50), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
        imshow("信息收集", frame);
        int ch = cv::waitKey(wait_time);
        if (ch == 27)
            cv::waitKey(0);
        else if (ch == 113) // 'q'
            exit(0);
    }
}

constexpr const char *keys = "{ ? h help  |     | 帮助信息 }"
                             "{ m model   |     | 模型路径，这将跳过训练直接进行测试 }"
                             "{ n num     |2000 | 每种装甲板收集的数量 }"
                             "{ w waitkey |1    | cv::cv::waitKey(?) }"
                             "{ c color   |0    | 识别装甲板颜色 '\033[33m0\033[0m': 识别蓝色，"
                             "'\033[33m1\033[0m': 识别红色 }";

constexpr const char *help = "                      \033[34;1m使用说明\033[0m\n"
                             "程序会收集大小装甲板数据各 'num' 张（默认为 2000 张），通过采集\n1) "
                             "装甲板长宽比 combo_ratio\n2) 灯条宽度比 width_ratio\n3) 灯条长度比 "
                             "length_ratio\n5) 共 3 个指标特征，进行 SVM 二分类，分类后的结果将自"
                             "动\n导出至 \033[33mout_armor_size.xml\033[0m 中";

int main(int argc, const char *argv[]) {
    // 命令行参数初始化
    cv::CommandLineParser parser(argc, argv, keys);
    if (parser.has("help")) {
        parser.printMessage();
        printf(help);
        exit(0);
    }

    // 获取命令行参数
    rm::PixChannel color = parser.get<int>("color") == 0 ? rm::PixChannel::BLUE : rm::PixChannel::RED;
    std::string model;
    if (parser.has("model"))
        model = parser.get<std::string>("model");
    collect_num = parser.get<int>("num");
    wait_time = parser.get<int>("waitkey");

    armor_samples = cv::Mat::zeros(collect_num * 2, 3, CV_32FC1);
    armor_responses = cv::Mat::zeros(collect_num * 2, 1, CV_32SC1);

    // 设置相机参数
    capture = rm::MvCamera::make_capture(rm::CameraConfig::create(rm::GrabMode::Continuous, rm::RetrieveMode::OpenCV));
    if (!capture->isOpened()) {
        printf("相机打开失败\n");
        return -1;
    }
    int exposure = 400;
    int gain = 64;
    int r_gain = 100;
    int g_gain = 100;
    int b_gain = 100;

    cv::FileStorage fs_mv_set("out_para.yml", cv::FileStorage::READ);
    if (fs_mv_set.isOpened()) {
        fs_mv_set["exposure"].isNone() ? void(0) : (fs_mv_set["exposure"] >> exposure);
        fs_mv_set["gain"].isNone() ? void(0) : (fs_mv_set["gain"] >> gain);
        fs_mv_set["r_gain"].isNone() ? void(0) : (fs_mv_set["r_gain"] >> r_gain);
        fs_mv_set["g_gain"].isNone() ? void(0) : (fs_mv_set["g_gain"] >> g_gain);
        fs_mv_set["b_gain"].isNone() ? void(0) : (fs_mv_set["b_gain"] >> b_gain);
    }

    capture->set(rm::CameraProperties::auto_exposure, false);
    capture->set(rm::CameraProperties::exposure, exposure);
    capture->set(rm::CameraProperties::gain, gain);
    capture->set(rm::CameraProperties::auto_wb, false);
    capture->set(rm::CameraProperties::wb_rgain, r_gain);
    capture->set(rm::CameraProperties::wb_ggain, g_gain);
    capture->set(rm::CameraProperties::wb_bgain, b_gain);

    cv::Ptr<cv::ml::SVM> p_svm = nullptr;

    if (model.empty()) {
        printf("=======================================\n");
        // --------------------- 小装甲板收集 ---------------------
        std::string key_str;
        printf("\033[32m小装甲板\033[0m信息收集即将开始...\n");
        printf("输入 \033[33;1mok\033[0m 来开始收集 >>> ");
        std::cin >> key_str;
        if (key_str != "ok")
            return 0;
        collect(color, rm::ArmorSizeType::SMALL, 0);
        cv::destroyAllWindows();
        rm::Timer::sleep_for(10);

        // --------------------- 大装甲板收集 ---------------------
        printf("\033[32m大装甲板\033[0m信息收集即将开始...\n");
        printf("输入 \033[33;1mok\033[0m 来开始收集 >>> ");
        std::cin >> key_str;
        if (key_str != "ok")
            return 0;
        collect(color, rm::ArmorSizeType::BIG, collect_num);
        cv::destroyAllWindows();
        rm::Timer::sleep_for(10);

        // 训练与分类
        p_svm = cv::ml::SVM::create();
        p_svm->setType(cv::ml::SVM::C_SVC);
        p_svm->setKernel(cv::ml::SVM::RBF);
        if (p_svm->train(armor_samples, cv::ml::ROW_SAMPLE, armor_responses))
            p_svm->save("out_armor_size.xml"); // 导出模型
        else
            RMVL_Error(RMVL_StsError, "Failed to train the SVM.");

        // 测试
        printf("想要进行测试吗？\n");
        printf("输入 \033[33;1mok\033[0m 来进行测试 >>> ");
        std::cin >> key_str;
        if (key_str != "ok")
            return 0;
    } else {
        p_svm = cv::ml::SVM::load(model);
    }
    printf("在键盘上按下一次 \033[33mEsc\033[0m 来暂停，按下两次 \033[33mEsc\033[0m 来退出测试\n");
    while (capture->read(frame)) {
        auto info = p_detector->detect(groups, frame, color, rm::ImuData(), rm::Timer::now());
        const auto &combos = info.combos;
        if (!combos.empty()) {
            if (combos.size() > 1)
                WARNING_("Size of the combos is greater than 1, size = %zu", combos.size());
            auto p_armor = rm::Armor::cast(combos.front());
            // 收集数据
            cv::Matx13f test_sample;
            test_sample(0) = p_armor->getComboRatio();  // 装甲板长宽比
            test_sample(1) = p_armor->getWidthRatio();  // 灯条宽度比
            test_sample(2) = p_armor->getLengthRatio(); // 灯条长度比
            // 测试
            float result = p_svm->predict(test_sample);
            putText(frame, result == 1 ? "SMALL" : "BIG", cv::Point(20, 50),
                    cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            const auto &corners = p_armor->corners();
            for (int i = 0; i < 4; ++i)
                line(frame, corners[i], corners[(i + 1) % 4], cv::Scalar(0, 255, 0), 2);
        }
        imshow("模型测试", frame);
        if (cv::waitKey(wait_time) == 27)
            if (cv::waitKey(0) == 27)
                break;
    }

    return 0;
}
