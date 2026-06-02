#include "Dashboard.h"

#include <iostream>
#include <iomanip>
#include <thread>

Dashboard::Dashboard(size_t totalAccounts,
                     std::function<MetricsReport()> metricsProvider,
                     std::function<size_t()> queueSizeProvider,
                     std::function<size_t()> activeWorkerProvider,
                     std::function<std::string()> consistencyProvider)
    : totalAccounts(totalAccounts),
      metricsProvider(std::move(metricsProvider)),
      queueSizeProvider(std::move(queueSizeProvider)),
      activeWorkerProvider(std::move(activeWorkerProvider)),
      consistencyProvider(std::move(consistencyProvider)) {}

Dashboard::~Dashboard() {
    stop();
}

void Dashboard::start() {
    bool expected = false;
    if (!running.compare_exchange_strong(expected, true)) {
        return;
    }

    monitorThread = std::thread(&Dashboard::monitorLoop, this);
}

void Dashboard::stop() {
    bool expected = true;
    if (!running.compare_exchange_strong(expected, false)) {
        return;
    }

    if (monitorThread.joinable()) {
        monitorThread.join();
    }
}

void Dashboard::monitorLoop() {
    while (running.load(std::memory_order_relaxed)) {
        const MetricsReport report = metricsProvider();
        const size_t queueSize = queueSizeProvider();
        const size_t activeWorkers = activeWorkerProvider();
        const std::string consistencyStatus = consistencyProvider();

        render(report, queueSize, activeWorkers, consistencyStatus);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    const MetricsReport report = metricsProvider();
    const size_t queueSize = queueSizeProvider();
    const size_t activeWorkers = activeWorkerProvider();
    const std::string consistencyStatus = consistencyProvider();
    render(report, queueSize, activeWorkers, consistencyStatus);
}

void Dashboard::render(const MetricsReport& report,
                       size_t queueSize,
                       size_t activeWorkers,
                       const std::string& consistencyStatus) const {
    std::lock_guard<std::mutex> lock(outputMutex);
    std::cout << "\n[Live Dashboard]" << std::endl;
    std::cout << "  Total accounts: " << totalAccounts << std::endl;
    std::cout << "  Total transactions: " << report.totalTransactions << std::endl;
    std::cout << "  Successful transactions: " << report.successfulTransactions << std::endl;
    std::cout << "  Failed transactions: " << report.failedTransactions << std::endl;
    std::cout << "  Current TPS: " << report.currentTps << std::endl;
    std::cout << "  Peak TPS: " << report.peakTps << std::endl;
    std::cout << "  Average latency (us): " << report.averageLatencyMicroseconds << std::endl;
    std::cout << "  Max latency (us): " << report.maxLatencyMicroseconds << std::endl;
    std::cout << "  Queue size: " << queueSize << std::endl;
    std::cout << "  Active worker threads: " << activeWorkers << std::endl;
    std::cout << "  Consistency check: " << consistencyStatus << std::endl;
    std::cout << std::endl;
}
