#pragma once

#include "MetricsCollector.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

class Dashboard {
public:
    Dashboard(size_t totalAccounts,
              std::function<MetricsReport()> metricsProvider,
              std::function<size_t()> queueSizeProvider,
              std::function<size_t()> activeWorkerProvider,
              std::function<std::string()> consistencyProvider);

    ~Dashboard();

    void start();
    void stop();

private:
    void monitorLoop();
    void render(const MetricsReport& report,
                size_t queueSize,
                size_t activeWorkers,
                const std::string& consistencyStatus) const;

    const size_t totalAccounts;
    const std::function<MetricsReport()> metricsProvider;
    const std::function<size_t()> queueSizeProvider;
    const std::function<size_t()> activeWorkerProvider;
    const std::function<std::string()> consistencyProvider;

    std::atomic<bool> running{false};
    std::thread monitorThread;
    mutable std::mutex outputMutex;
};
