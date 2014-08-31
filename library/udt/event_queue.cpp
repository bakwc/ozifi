#include <iostream>

#include "event_queue.h"

void TEventQueue::Add(const std::function<void ()> func) {
    std::unique_lock<std::mutex> guard(Mutex);
    if (!Worker) {
        Worker.reset(new std::thread(&TEventQueue::WorkerFunc));
    }
    Queue.push(func);
    if (Queue.size() > 100) {
        std::cerr << "WARNING: TEventQueue overflow: " << Queue.size() << "\n";
    }
    CondVar.notify_one();
}

void TEventQueue::WorkerFunc() {
    while(true) {
        std::queue<std::function<void()>> queue;
        std::unique_lock<std::mutex> guard(Mutex);
        if (Queue.empty()) {
            CondVar.wait(guard);
        }
        queue.swap(Queue);
        while (!queue.empty()) {
            queue.front()();
            queue.pop();
        }
    }
}

std::unique_ptr<std::thread> TEventQueue::Worker;
std::mutex TEventQueue::Mutex;
std::queue<std::function<void()>> TEventQueue::Queue;
std::condition_variable TEventQueue::CondVar;
