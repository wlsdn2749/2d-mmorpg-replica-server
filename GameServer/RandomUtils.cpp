#include "pch.h"
#include "RandomUtils.h"
#include <chrono>

thread_local std::mt19937 RandomUtils::s_generator;
thread_local bool RandomUtils::s_initialized = false;

void RandomUtils::InitializeIfNeeded() {
    if (!s_initialized) {
        auto now = std::chrono::high_resolution_clock::now();
        auto seed = static_cast<uint32_t>(now.time_since_epoch().count()) ^
            std::hash<std::thread::id>{}(std::this_thread::get_id());

        s_generator.seed(seed);
        s_initialized = true;
    }
}

// 범위 기반 랜덤 구현
int RandomUtils::GetRandom(int min, int max) {
    InitializeIfNeeded();
    std::uniform_int_distribution<int> dist(min, max);
    return dist(s_generator);
}

uint32_t RandomUtils::GetRandom(uint32_t min, uint32_t max) {
    InitializeIfNeeded();
    std::uniform_int_distribution<uint32_t> dist(min, max);
    return dist(s_generator);
}

float RandomUtils::GetRandom(float min, float max) {
    InitializeIfNeeded();
    std::uniform_real_distribution<float> dist(min, max);
    return dist(s_generator);
}

double RandomUtils::GetRandom(double min, double max) {
    InitializeIfNeeded();
    std::uniform_real_distribution<double> dist(min, max);
    return dist(s_generator);
}

// 0 ~ max 범위 랜덤 구현
int RandomUtils::GetRandom(int max) {
    return GetRandom(0, max - 1);
}

uint32_t RandomUtils::GetRandom(uint32_t max) {
    return GetRandom(0u, max - 1);
}

float RandomUtils::GetRandom(float max) {
    return GetRandom(0.0f, max);
}

double RandomUtils::GetRandom(double max) {
    return GetRandom(0.0, max);
}