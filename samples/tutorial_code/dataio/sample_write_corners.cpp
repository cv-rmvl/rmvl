#include <iostream>

#include <rmvl/rmvl.hpp>

int main()
{
    for (uint32_t n = 0; n < 5; ++n)
    {
        // data prepare
        std::vector<std::vector<cv::Point2f>> corners;
        corners.resize(3);
        for (std::size_t i = 0; i < 3; ++i)
        {
            corners[i].resize(n + 1);
            for (std::size_t j = 0; j < n + 1; ++j)
                corners[i][j] = {i + j, n + j};
        }
        // write data
        RMVL_Assert(rm::writeCorners("ts.yml", n, corners));
    }

    return 0;
}