/**
 * @file util.cpp
 * @author zhaoxi (535394140@qq.com)
 * @brief 数据 IO 工具库实现
 * @version 1.0
 * @date 2025-08-15
 *
 * @copyright Copyright 2025 (c), zhaoxi
 *
 */

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

#include "rmvl/core/util.hpp"
#include "rmvl/io/util.hpp"

namespace rm {

static bool readFloatToken(std::istream &is, float &value) noexcept {
    std::string token;
    if (!(is >> token))
        return false;
    while (!token.empty() && (token.back() == ',' || std::isspace(static_cast<unsigned char>(token.back()))))
        token.pop_back();
    try {
        value = std::stof(token);
        return true;
    } catch (...) {
        return false;
    }
}

void ImuData::write(std::ostream &os, const ImuData &data) noexcept {
    os << data.translation.x << ", " << data.translation.y << ", " << data.translation.z << ", "
       << data.translation.vx << ", " << data.translation.vy << ", " << data.translation.vz << ", "
       << data.rotation.yaw << ", " << data.rotation.pitch << ", " << data.rotation.roll << ", "
       << data.rotation.yaw_speed << ", " << data.rotation.pitch_speed << ", " << data.rotation.roll_speed << "," << std::endl;
}

void ImuData::read(std::istream &is, ImuData &data) noexcept {
    bool ok = true;
    reflect::for_each(data.translation, [&](auto &&val) { ok = ok && readFloatToken(is, val); });
    reflect::for_each(data.rotation, [&](auto &&val) { ok = ok && readFloatToken(is, val); });
    if (!ok)
        is.setstate(std::ios::failbit);
}

void ImuData::write(std::string_view output_file, const std::vector<ImuData> &datas) noexcept {
    std::ofstream ofs(output_file.data(), std::ios::app);
    if (!ofs.is_open())
        return;
    std::for_each(datas.begin(), datas.end(), [&ofs](const ImuData &d) { write(ofs, d); });
    ofs.close();
}

std::vector<ImuData> ImuData::read(std::string_view input_file) noexcept {
    std::ifstream ifs(input_file.data());
    if (!ifs.is_open())
        return {};
    std::vector<ImuData> datas;
    datas.reserve(1000);
    while (ifs) {
        ImuData d;
        read(ifs, d);
        if (ifs)
            datas.push_back(d);
    }
    ifs.close();
    return datas;
}

void writeCorners(std::ostream &out, const std::vector<std::vector<std::array<float, 2>>> &corners) {
    std::for_each(corners.begin(), corners.end(), [&out](const auto &corner) {
        if (corner.empty())
            return;
        std::for_each(corner.begin(), corner.end() - 1, [&out](const auto &p) {
            out << p[0] << ", " << p[1] << ", ";
        });
        out << corner.back()[0] << ", " << corner.back()[1] << "," << std::endl;
    });
    out << "---" << std::endl;
}

void readCorners(std::istream &in, std::vector<std::vector<std::array<float, 2>>> &corners) {
    std::string line;
    while (std::getline(in, line)) {
        if (line == "---")
            break;
        std::vector<std::array<float, 2>> corner;
        std::istringstream iss(line);
        while (iss) {
            float point[2]{};
            if (!readFloatToken(iss, point[0]) || !readFloatToken(iss, point[1]))
                break;
            corner.push_back({point[0], point[1]});
        }
        if (!corner.empty())
            corners.emplace_back(std::move(corner));
    }
}

} // namespace rm
