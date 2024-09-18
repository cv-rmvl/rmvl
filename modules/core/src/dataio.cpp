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

#include <algorithm>
#include <fstream>
#include <sstream>

#include "rmvl/core/io.hpp"
#include "rmvl/core/util.hpp"

namespace rm
{

void GyroData::write(std::ostream &os, const GyroData &data) noexcept
{
    os << data.translation.x << ", " << data.translation.y << ", " << data.translation.z << ", "
       << data.translation.vx << ", " << data.translation.vy << ", " << data.translation.vz << ", "
       << data.rotation.yaw << ", " << data.rotation.pitch << ", " << data.rotation.roll << ", "
       << data.rotation.yaw_speed << ", " << data.rotation.pitch_speed << ", " << data.rotation.roll_speed << "," << std::endl;
}

void GyroData::read(std::istream &is, GyroData &data) noexcept
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

void GyroData::write(std::string_view output_file, const std::vector<GyroData> &datas) noexcept
{
    std::ofstream ofs(output_file.data(), std::ios::app);
    if (!ofs.is_open())
        return;
    std::for_each(datas.begin(), datas.end(), [&ofs](const GyroData &d) { write(ofs, d); });
    ofs.close();
}

std::vector<GyroData> GyroData::read(std::string_view input_file) noexcept
{
    std::ifstream ifs(input_file.data());
    if (!ifs.is_open())
        return {};
    std::vector<GyroData> datas;
    datas.reserve(1000);
    while (!ifs.eof())
    {
        GyroData d;
        read(ifs, d);
        datas.push_back(d);
    }
    ifs.close();
    return datas;
}

void writeCorners(std::ostream &out, const std::vector<std::vector<std::array<float, 2>>> &corners)
{
    std::for_each(corners.begin(), corners.end(), [&out](const auto &corner) {
        std::for_each(corner.begin(), corner.end() - 1, [&out](const auto &p) {
            out << p[0] << ", " << p[1] << ", ";
        });
        out << corner.back()[0] << ", " << corner.back()[1] << "," << std::endl;
    });
    out << "---" << std::endl;
}

void readCorners(std::istream &in, std::vector<std::vector<std::array<float, 2>>> &corners)
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

} // namespace rm
