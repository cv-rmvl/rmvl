#include <iostream>

#include <rmvl/rmvl.hpp>

using namespace std;
using namespace cv;

int main()
{
    for (uint32_t n = 0; n < 5; ++n)
    {
        vector<vector<Point2f>> corners;
        // read data
        if (!readCorners("ts.yml", n, corners))
            continue;
        // print the data
        for (size_t i = 0; i < corners.size(); ++i)
            for (size_t j = 0; j < corners[i].size(); ++i)
                cout << i << "." << j << " = " << corners[i][j] << endl;
    }

    return 0;
}