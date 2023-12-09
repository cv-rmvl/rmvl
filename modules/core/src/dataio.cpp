/**
 * @file dataio.cpp
 * @author RoboMaster Vision Community
 * @brief
 * @version 1.0
 * @date 2023-04-22
 *
 * @copyright Copyright 2023 (c), RoboMaster Vision Community
 *
 */

#include <sstream>
#include <algorithm>

#include "rmvl/core/dataio.hpp"
#include "rmvl/core/util.hpp"

void rm::GyroData::write(std::ostream &os, const GyroData &data) noexcept
{
    os << data.translation.x << ", " << data.translation.y << ", " << data.translation.z << ", "
       << data.translation.vx << ", " << data.translation.vy << ", " << data.translation.vz << ", "
       << data.rotation.yaw << ", " << data.rotation.pitch << ", " << data.rotation.roll << ", "
       << data.rotation.yaw_speed << ", " << data.rotation.pitch_speed << ", " << data.rotation.roll_speed << "," << std::endl;
}

void rm::GyroData::read(std::istream &is, GyroData &data) noexcept
{
    std::string tstr[6];
    std::for_each(tstr, tstr + 6, [&is](std::string &s) { is >> s; });
    size_t t_idx = 0;
    reflect::for_each(data.translation, [&](auto &&val) { val = std::stof(tstr[t_idx++]); });

    std::string rstr[6];
    std::for_each(rstr, rstr + 6, [&is](std::string &s) { is >> s; });
    size_t r_idx = 0;
    reflect::for_each(data.rotation, [&](auto &&val) { val = std::stof(rstr[r_idx++]); });
}

void rm::writeCorners(std::ostream &out, const std::vector<std::vector<std::array<float, 2>>> &corners)
{
    std::for_each(corners.begin(), corners.end(), [&out](const auto &corner) {
        std::for_each(corner.begin(), corner.end() - 1, [&out](const auto &p) {
            out << p[0] << ", " << p[1] << ", ";
        });
        out << corner.back()[0] << ", " << corner.back()[1] << "," << std::endl;
    });
    out << "---" << std::endl;
}

void rm::readCorners(std::istream &in, std::vector<std::vector<std::array<float, 2>>> &corners)
{
    std::string line;
    while (std::getline(in, line))
    {
        if (line == "---")
            break;
        std::vector<std::array<float, 2>> corner;
        std::istringstream iss(line);
        while (!iss.eof())
        {
            std::string point[2];
            iss >> point[0] >> point[1];
            corner.push_back({std::stof(point[0]), std::stof(point[1])});
        }
        corners.emplace_back(std::move(corner));
    }
}
