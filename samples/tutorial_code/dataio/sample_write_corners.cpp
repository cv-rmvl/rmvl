#include <iostream>

#include <rmvl/rmvl.hpp>

using namespace std;
using namespace cv;

int main()
{
    for (uint32_t n = 0; n < 5; ++n)
    {
        // data prepare
        vector<vector<Point2f>> corners;
        corners.resize(3);
        for (size_t i = 0; i < 3; ++i)
        {
            corners[i].resize(n + 1);
            for (size_t j = 0; j < n + 1; ++j)
                corners[i][j] = Point2f(i + j, n + j);
        }
        // write data
        RMVL_Assert(writeCorners("ts.yml", n, corners));
    }

    return 0;
}