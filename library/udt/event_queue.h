#pragma once

#include <functional>
#include <thread>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

class TEventQueue {
public:
    static void Add(const std::function<void()> func);
private:
    TEventQueue() = default;
    static void WorkerFunc();
private:
    static std::unique_ptr<std::thread> Worker;
    static std::mutex Mutex;
    static std::queue<std::function<void()>> Queue;
    static std::condition_variable CondVar;
};
