// MIT License
//
// Copyright (c) 2022 Ignacio Vizzo, Tiziano Guadagnino, Benedikt Mersch, Cyrill
// Stachniss.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#pragma once

#include <tbb/blocked_range.h>
#include <tbb/parallel_reduce.h>

#include <Eigen/Core>
#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>

#include "Registration.hpp"

// This parameters are not intended to be changed, therefore we do not expose it
namespace {
constexpr int MAX_NUM_ITERATIONS_ = 500;
constexpr double ESTIMATION_THRESHOLD_ = 0.0001;

inline void TransformPoints(const Eigen::Matrix4d &T, std::vector<Eigen::Vector3d> &points) {
    std::transform(points.cbegin(), points.cend(), points.begin(), [&](const auto &point) {
        const Eigen::Matrix3d R = T.block<3, 3>(0, 0);
        const Eigen::Vector3d translation = T.block<3, 1>(0, 3);
        return Eigen::Vector3d{R * point + translation};
    });
}
}  // namespace

namespace kiss_icp {

template <typename InternalMapT>
class Map {
public:
    // Type aliases
    using Vector3dVector = std::vector<Eigen::Vector3d>;
    using Vector3dVectorTuple = std::tuple<Vector3dVector, Vector3dVector>;

public:
    template <typename... Args>
    Map(Args &&...args) : map_(std::forward<Args>(args)...) {}

public:
    inline void Clear() { return map_.Clear(); }
    inline bool Empty() const { return map_.Empty(); }
    inline Vector3dVector Pointcloud() const { return map_.Pointcloud(); }
    inline void AddPoints(const Vector3dVector &points, const Eigen::Vector3d &origin) {
        map_.AddPoints(points, origin);
    };

    inline void AddPoints(const Vector3dVector &points, const Eigen::Matrix4d &pose) {
        auto points_t = points;
        TransformPoints(pose, points_t);
        const Eigen::Vector3d &origin = pose.block<3, 1>(0, 3);
        map_.AddPoints(points_t, origin);
    };
    inline Vector3dVectorTuple GetCorrespondences(const Vector3dVector &points,
                                                  double max_correspondance_distance) const {
        struct ResultTuple {
            ResultTuple(std::size_t n) {
                source.reserve(n);
                target.reserve(n);
            }
            std::vector<Eigen::Vector3d> source;
            std::vector<Eigen::Vector3d> target;
        };
        using points_iterator = std::vector<Eigen::Vector3d>::const_iterator;
        const auto [source, target] = tbb::parallel_reduce(
            // Range
            tbb::blocked_range<points_iterator>{points.cbegin(), points.cend()},
            // Identity
            ResultTuple(points.size()),
            // 1st lambda: Paralell computation
            [&](const tbb::blocked_range<points_iterator> &r, ResultTuple res) -> ResultTuple {
                auto &[src, tgt] = res;
                auto &source_private = src;  // compile on clang
                auto &target_private = tgt;  // compile on clang
                source_private.reserve(r.size());
                target_private.reserve(r.size());
                std::for_each(r.begin(), r.end(), [&](const auto &point) {
                    Eigen::Vector3d closest_neighboors = map_.GetClosestNeighboor(point);
                    if ((closest_neighboors - point).norm() < max_correspondance_distance) {
                        source_private.emplace_back(point);
                        target_private.emplace_back(closest_neighboors);
                    }
                });
                return res;
            },
            // 2st lambda: Parallell reduction
            [&](ResultTuple a, const ResultTuple &b) -> ResultTuple {
                auto &[src, tgt] = a;
                auto &source = src;  // compile on clang
                auto &target = tgt;  // compile on clang
                const auto &[srcp, tgtp] = b;
                auto &source_private = srcp;  // compile on clang
                auto &target_private = tgtp;  // compile on clang
                source.insert(source.end(),   //
                              std::make_move_iterator(source_private.begin()),
                              std::make_move_iterator(source_private.end()));
                target.insert(target.end(),  //
                              std::make_move_iterator(target_private.begin()),
                              std::make_move_iterator(target_private.end()));
                return a;
            });

        return std::make_tuple(source, target);
    }

    inline Eigen::Matrix4d RegisterPoinCloud(const Vector3dVector &points,
                                             const Eigen::Matrix4d &initial_guess,
                                             double max_correspondence_distance,
                                             double kernel) {
        if (map_.Empty()) return initial_guess;

        // Equation (9)
        Vector3dVector source = points;
        TransformPoints(initial_guess, source);

        // ICP-loop
        Eigen::Matrix4d T_icp = Eigen::Matrix4d::Identity();
        for (int j = 0; j < MAX_NUM_ITERATIONS_; ++j) {
            // Equation (10)
            const auto &[src, tgt] = GetCorrespondences(source, max_correspondence_distance);
            // Equation (11)
            auto [x, estimation] = AlignClouds(src, tgt, kernel);
            // Equation (12)
            TransformPoints(estimation, source);
            // Update iterations
            T_icp = estimation * T_icp;
            // Termination criteria
            if (x.norm() < ESTIMATION_THRESHOLD_) break;
        }
        // Spit the final transformation
        return T_icp * initial_guess;
    }

private:
    InternalMapT map_;
};

}  // namespace kiss_icp
