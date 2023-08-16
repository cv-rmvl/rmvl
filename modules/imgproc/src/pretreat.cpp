/**
 * @file pretreat.cpp
 * @author RoboMaster Vision Community
 * @brief Image pretreating module
 * @version 1.0
 * @date 2022-11-23
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <opencv2/imgproc.hpp>

#include "rmvl/core/util.hpp"
#include "rmvl/imgproc/pretreat.h"

using namespace std;
using namespace cv;

Mat rm::binary(Mat src, rm::PixChannel ch1, rm::PixChannel ch2, uint8_t thresh)
{
    if (src.type() != CV_8UC3)
        RMVL_Error(RMVL_StsBadArg, "The image type of \"src\" is incorrect");
    Mat bin = Mat::zeros(Size(src.cols, src.rows), CV_8UC1);
    // Image process
    parallel_for_(Range(0, src.rows),
                  [&](const Range &range)
                  {
                      uchar *data_src = nullptr;
                      uchar *data_bin = nullptr;
                      for (int row = range.start; row < range.end; ++row)
                      {
                          data_src = src.ptr<uchar>(row);
                          data_bin = bin.ptr<uchar>(row);
                          for (int col = 0; col < src.cols; ++col)
                              if (data_src[3 * col + ch1] - data_src[3 * col + ch2] > thresh)
                                  data_bin[col] = 255;
                      }
                  });
    return bin;
}

Mat rm::binary(Mat src, uint8_t thresh)
{
    if (src.type() != CV_8UC3 && src.type() != CV_8UC1)
        RMVL_Error(RMVL_StsBadArg, "The image type of \"src\" is incorrect");
    Mat bin;
    if (src.type() == CV_8UC3)
        cvtColor(src, bin, COLOR_BGR2GRAY);
    else
        bin = src;
    threshold(bin, bin, thresh, 255, THRESH_BINARY);
    return bin;
}
