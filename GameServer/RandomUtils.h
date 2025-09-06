// RandomUtils.h
#pragma once
#include <random>


class RandomUtils {
public:
    // 범위 기반 랜덤 명시적 특화 선언
    static int GetRandom(int min, int max);
    static uint32_t GetRandom(uint32_t min, uint32_t max);
    static float GetRandom(float min, float max);
    static double GetRandom(double min, double max);

    // 0 ~ max 범위 랜덤 명시적 특화 선언
    static int GetRandom(int max);
    static uint32_t GetRandom(uint32_t max);
    static float GetRandom(float max);
    static double GetRandom(double max);

    // 특화된 편의 함수들
    static float GetRandomFloat100() { return GetRandom(0.0f, 100.0f); }
    static float GetRandomFloat01() { return GetRandom(0.0f, 1.0f); }

private:
    static thread_local std::mt19937 s_generator;
    static thread_local bool s_initialized;

    static void InitializeIfNeeded();
};
