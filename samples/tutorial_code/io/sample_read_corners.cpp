#include <iostream>

#include <rmvl/rmvl.hpp>

int main()
{
    for (uint32_t n = 0; n < 5; ++n)
    {
        std::vector<std::vector<cv::Point2f>> corners;
        // read data
        if (!rm::readCorners("ts.yml", n, corners))
            continue;
        // print the data
        for (std::size_t i = 0; i < corners.size(); ++i)
            for (std::size_t j = 0; j < corners[i].size(); ++i)
                std::cout << i << "." << j << " = " << corners[i][j] << std::endl;
    }

    return 0;
}