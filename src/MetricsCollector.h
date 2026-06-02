#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <limits>

struct MetricsReport {
    long long totalTransactions;
    long long successfulTransactions;
    long long failedTransactions;
    double averageLatencyMicroseconds;
    long long minLatencyMicroseconds;
    long long maxLatencyMicroseconds;
    double currentTps;
    double peakTps;
};

class MetricsCollector {
public:
    MetricsCollector();

    void recordSuccess(long long latencyMicroseconds);
    void recordFailure(long long latencyMicroseconds);

    MetricsReport getReport() const;

private:
    void recordLatency(long long latencyMicroseconds);
    void updateTps();

    std::atomic<long long> totalTransactions{0};
    std::atomic<long long> successfulTransactions{0};
    std::atomic<long long> failedTransactions{0};
    std::atomic<long long> totalLatencyMicroseconds{0};
    std::atomic<long long> minLatencyMicroseconds{std::numeric_limits<long long>::max()};
    std::atomic<long long> maxLatencyMicroseconds{0};
    std::atomic<double> currentTps{0.0};
    std::atomic<double> peakTps{0.0};
    std::chrono::steady_clock::time_point startTime;
};
