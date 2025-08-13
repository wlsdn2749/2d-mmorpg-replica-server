#pragma once

struct Account
{
private:
    std::atomic<int> userId{ 0 }; // 초기화

public:
    void SetUserId(int id) { userId.store(id, std::memory_order_relaxed); }
    int GetUserId() const { return userId.load(std::memory_order_relaxed); }
};