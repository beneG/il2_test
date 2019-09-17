#pragma once
#include <random>
#include <chrono>

static long long seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
static std::mt19937 rng((unsigned int)seed);

constexpr const double M_PI   = 3.14159265358979323846;   // pi
constexpr const double M_PI_2 = 1.57079632679489661923;   // pi/2
constexpr const double M_PI_4 = 0.785398163397448309616;  // pi/4

