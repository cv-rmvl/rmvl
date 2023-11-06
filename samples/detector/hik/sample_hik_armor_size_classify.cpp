#include <iostream>
#include <thread>

#include "rmvl/core/timer.hpp"
#include "rmvl/camera/hik_camera.h"
#include "rmvl/detector/armor_detector.h"

#include <opencv2/highgui.hpp>
#include <opencv2/ml.hpp>

using namespace rm;
using namespace para;
using namespace std;
using namespace cv;

int wait_time = 1;
int collect_num = 2000;

HikCamera::ptr capture;                     // 相机
auto p_detector = ArmorDetector::make_detector(); // 识别模块
Mat frame;                                        // 帧图像
vector<group::ptr> groups;                        // 序列组列表
Mat armor_samples;                                // 装甲板信息样本
Mat armor_responses;                              // 装甲板响应/标签

void collect(PixChannel color, ArmorSizeType type, int begin_idx)
{
    for (int idx = 0; idx < collect_num; ++idx)
    {
        capture->read(frame);
        if (frame.empty())
            RMVL_Error(RMVL_StsBadSize, "frame is empty, something wrong with the camera.");
        auto info = p_detector->detect(groups, frame, color, GyroData(), Timer::now());
        const auto &combos = info.combos;
        if (combos.size() != 1)
        {
            if (combos.size() > 1)
                WARNING_("Size of the combos is greater than 1, size = %zu", combos.size());
            idx--;
        }
        else
        {
            auto p_armor = Armor::cast(combos.front());
            // 收集数据
            armor_samples.at<float>(begin_idx + idx, 0) = p_armor->getComboRatio();  // 装甲板长宽比
            armor_samples.at<float>(begin_idx + idx, 1) = p_armor->getWidthRatio();  // 灯条宽度比
            armor_samples.at<float>(begin_idx + idx, 2) = p_armor->getLengthRatio(); // 灯条长度比
            armor_responses.at<int>(begin_idx + idx) = type == ArmorSizeType::SMALL ? 1 : 2;

            const auto &corners = p_armor->getCorners();
            for (int i = 0; i < 4; ++i)
                line(frame, corners[i], corners[(i + 1) % 4], Scalar(0, 255, 0), 2);
        }
        putText(frame, to_string(idx), Point(20, 50), FONT_HERSHEY_COMPLEX, 1.0, Scalar(255, 255, 255), 1);
        imshow("信息收集", frame);
        int ch = waitKey(wait_time);
        if (ch == 27)
            waitKey(0);
        else if (ch == 113) // 'q'
            exit(0);
    }
}

const char *keys = "{ ? h help  |     | 帮助信息 }"
                   "{ m model   |     | 模型路径，这将跳过训练直接进行测试 }"
                   "{ n num     |2000 | 每种装甲板收集的数量 }"
                   "{ w waitkey |1    | cv::waitKey(?) }"
                   "{ c color   |0    | 识别装甲板颜色 '\033[33m0\033[0m': 识别蓝色，"
                   "'\033[33m1\033[0m': 识别红色 }";

const char *help = "                      \033[34;1m使用说明\033[0m\n"
                   "程序会收集大小装甲板数据各 'num' 张（默认为 2000 张），通过采集\n1) "
                   "装甲板长宽比 combo_ratio\n2) 灯条宽度比 width_ratio\n3) 灯条长度比 "
                   "length_ratio\n5) 共 3 个指标特征，进行 SVM 二分类，分类后的结果将自"
                   "动\n导出至 \033[33mout_armor_size.xml\033[0m 中";

int main(int argc, const char *argv[])
{
    // 命令行参数初始化
    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        parser.printMessage();
        cout << help << endl;
        exit(0);
    }

    // 获取命令行参数
    PixChannel color = parser.get<int>("color") == 0 ? PixChannel::BLUE : PixChannel::RED;
    string model;
    if (parser.has("model"))
        model = parser.get<string>("model");
    collect_num = parser.get<int>("num");
    wait_time = parser.get<int>("waitkey");

    armor_samples = Mat::zeros(collect_num * 2, 3, CV_32FC1);
    armor_responses = Mat::zeros(collect_num * 2, 1, CV_32SC1);

    // 设置相机参数
    capture = HikCamera::make_capture(GRAB_CONTINUOUS, RETRIEVE_CV);
    if (!capture->isOpened())
    {
        printf("相机打开失败\n");
        return -1;
    }
    int exposure = 3000;
    int gain = 8;
    int r_gain = 1500;
    int g_gain = 1500;
    int b_gain = 1500;

    FileStorage fs_mv_set("out_para.yml", FileStorage::READ);
    if (fs_mv_set.isOpened())
    {
        readExcludeNone(fs_mv_set["exposure"], exposure);
        readExcludeNone(fs_mv_set["gain"], gain);
        readExcludeNone(fs_mv_set["r_gain"], r_gain);
        readExcludeNone(fs_mv_set["g_gain"], g_gain);
        readExcludeNone(fs_mv_set["b_gain"], b_gain);
    }

    capture->set(CAMERA_MANUAL_EXPOSURE, 0);
    capture->set(CAMERA_EXPOSURE, exposure);
    capture->set(CAMERA_GAIN, gain);
    capture->set(CAMERA_MANUAL_WB, 0);
    capture->set(CAMERA_WB_RGAIN, r_gain);
    capture->set(CAMERA_WB_GGAIN, g_gain);
    capture->set(CAMERA_WB_BGAIN, b_gain);

    cv::Ptr<cv::ml::SVM> p_svm = nullptr;

    if (model.empty())
    {
        cout << "=======================================" << endl;
        // --------------------- 小装甲板收集 ---------------------
        string key_str;
        cout << "\033[32m小装甲板\033[0m信息收集即将开始..." << endl;
        cout << "输入 \033[33;1mok\033[0m 来开始收集 >>> ";
        cin >> key_str;
        if (key_str != "ok")
            return 0;
        collect(color, ArmorSizeType::SMALL, 0);
        destroyAllWindows();
        this_thread::sleep_for(chrono::milliseconds(10));

        // --------------------- 大装甲板收集 ---------------------
        cout << "\033[32m大装甲板\033[0m信息收集即将开始..." << endl;
        cout << "输入 \033[33;1mok\033[0m 来开始收集 >>> ";
        cin >> key_str;
        if (key_str != "ok")
            return 0;
        collect(color, ArmorSizeType::BIG, collect_num);
        destroyAllWindows();
        this_thread::sleep_for(chrono::milliseconds(10));

        // 训练与分类
        p_svm = ml::SVM::create();
        p_svm->setType(ml::SVM::C_SVC);
        p_svm->setKernel(ml::SVM::RBF);
        if (p_svm->train(armor_samples, ml::ROW_SAMPLE, armor_responses))
            p_svm->save("out_armor_size.xml"); // 导出模型
        else
            RMVL_Error(RMVL_StsError, "Failed to train the SVM.");

        // 测试
        cout << "想要进行测试吗？" << endl;
        cout << "输入 \033[33;1mok\033[0m 来进行测试 >>> ";
        cin >> key_str;
        if (key_str != "ok")
            return 0;
    }
    else
    {
        p_svm = ml::SVM::load(model);
    }
    cout << "在键盘上按下一次 \033[33mEsc\033[0m 来暂停，按下两次 \033[33mEsc\033[0m 来退出测试" << endl;
    while (capture->read(frame))
    {
        auto info = p_detector->detect(groups, frame, color, GyroData(), Timer::now());
        const auto &combos = info.combos;
        if (!combos.empty())
        {
            if (combos.size() > 1)
                WARNING_("Size of the combos is greater than 1, size = %zu", combos.size());
            auto p_armor = Armor::cast(combos.front());
            // 收集数据
            Matx13f test_sample;
            test_sample(0) = p_armor->getComboRatio();  // 装甲板长宽比
            test_sample(1) = p_armor->getWidthRatio();  // 灯条宽度比
            test_sample(2) = p_armor->getLengthRatio(); // 灯条长度比
            // 测试
            float result = p_svm->predict(test_sample);
            putText(frame, result == 1 ? "SMALL" : "BIG", Point(20, 50),
                    FONT_HERSHEY_COMPLEX, 1.0, Scalar(255, 255, 255), 2);
            const auto &corners = p_armor->getCorners();
            for (int i = 0; i < 4; ++i)
                line(frame, corners[i], corners[(i + 1) % 4], Scalar(0, 255, 0), 2);
        }
        imshow("模型测试", frame);
        if (waitKey(wait_time) == 27)
            if (waitKey(0) == 27)
                break;
    }

    return 0;
}
