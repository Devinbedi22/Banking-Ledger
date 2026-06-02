#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount)
        : stop(false) {
        // Ensure at least one worker thread is created.
        size_t count = std::max<size_t>(1, threadCount);
        for (size_t i = 0; i < count; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] {
                            return stop.load(std::memory_order_relaxed) || !tasks.empty();
                        });
                        if (stop.load(std::memory_order_relaxed) && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    activeWorkers.fetch_add(1, std::memory_order_relaxed);
                    task();
                    activeWorkers.fetch_sub(1, std::memory_order_relaxed);
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop.load(std::memory_order_relaxed)) {
                return;
            }
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

    size_t queueSize() const {
        std::lock_guard<std::mutex> lock(queueMutex);
        return tasks.size();
    }

    size_t activeWorkerCount() const noexcept {
        return activeWorkers.load(std::memory_order_relaxed);
    }

private:
    std::vector<std::thread> workers;
    mutable std::queue<std::function<void()>> tasks;
    mutable std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<size_t> activeWorkers{0};
};
