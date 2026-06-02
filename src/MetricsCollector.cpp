#include "MetricsCollector.h"

#include <algorithm>

MetricsCollector::MetricsCollector()
    : startTime(std::chrono::steady_clock::now()) {}

void MetricsCollector::recordSuccess(long long latencyMicroseconds) {
    totalTransactions.fetch_add(1, std::memory_order_relaxed);
    successfulTransactions.fetch_add(1, std::memory_order_relaxed);
    totalLatencyMicroseconds.fetch_add(latencyMicroseconds, std::memory_order_relaxed);
    recordLatency(latencyMicroseconds);
    updateTps();
}

void MetricsCollector::recordFailure(long long latencyMicroseconds) {
    totalTransactions.fetch_add(1, std::memory_order_relaxed);
    failedTransactions.fetch_add(1, std::memory_order_relaxed);
    totalLatencyMicroseconds.fetch_add(latencyMicroseconds, std::memory_order_relaxed);
    recordLatency(latencyMicroseconds);
    updateTps();
}

void MetricsCollector::recordLatency(long long latencyMicroseconds) {
    long long currentMin = minLatencyMicroseconds.load(std::memory_order_relaxed);
    while (latencyMicroseconds < currentMin &&
           !minLatencyMicroseconds.compare_exchange_weak(currentMin, latencyMicroseconds,
                                                        std::memory_order_relaxed)) {
    }

    long long currentMax = maxLatencyMicroseconds.load(std::memory_order_relaxed);
    while (latencyMicroseconds > currentMax &&
           !maxLatencyMicroseconds.compare_exchange_weak(currentMax, latencyMicroseconds,
                                                        std::memory_order_relaxed)) {
    }
}

void MetricsCollector::updateTps() {
    const auto now = std::chrono::steady_clock::now();
    const double elapsedSeconds = std::chrono::duration_cast<std::chrono::duration<double>>(now - startTime).count();
    if (elapsedSeconds <= 0.0) {
        return;
    }

    const double current = static_cast<double>(totalTransactions.load(std::memory_order_relaxed)) / elapsedSeconds;
    currentTps.store(current, std::memory_order_relaxed);

    double peak = peakTps.load(std::memory_order_relaxed);
    while (current > peak &&
           !peakTps.compare_exchange_weak(peak, current, std::memory_order_relaxed)) {
    }
}

MetricsReport MetricsCollector::getReport() const {
    const long long total = totalTransactions.load(std::memory_order_relaxed);
    const long long success = successfulTransactions.load(std::memory_order_relaxed);
    const long long failed = failedTransactions.load(std::memory_order_relaxed);
    const long long totalLatency = totalLatencyMicroseconds.load(std::memory_order_relaxed);
    long long minLatency = minLatencyMicroseconds.load(std::memory_order_relaxed);
    if (minLatency == std::numeric_limits<long long>::max()) {
        minLatency = 0;
    }
    const long long maxLatency = maxLatencyMicroseconds.load(std::memory_order_relaxed);
    const double averageLatency = total > 0 ? static_cast<double>(totalLatency) / total : 0.0;
    const double current = currentTps.load(std::memory_order_relaxed);
    const double peak = peakTps.load(std::memory_order_relaxed);

    return MetricsReport{
        total,
        success,
        failed,
        averageLatency,
        minLatency,
        maxLatency,
        current,
        peak
    };
}
