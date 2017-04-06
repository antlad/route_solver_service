#pragma once

#include <Eigen/Core>

#include <vector>

int ei_test_init_cuda();

double dot_cuda(const std::vector<Eigen::Vector3d>& v1, const std::vector<Eigen::Vector3d>& v2);
