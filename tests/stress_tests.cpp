#include "../src/Account.h"
#include "../src/TransactionLog.h"
#include "../src/TransferService.h"
#include "../src/ThreadPool.h"

#include <atomic>
#include <cassert>
#include <cmath>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

int main() {
    const int numAccounts = 1000;
    const double initialBalance = 10000.0;
    const int numThreads = 100;
    const int transfersPerThread = 50;

    std::vector<std::unique_ptr<Account>> accounts;
    accounts.reserve(numAccounts);
    for (int i = 1; i <= numAccounts; ++i) {
        accounts.push_back(std::make_unique<Account>(i, initialBalance));
    }

    auto totalFunds = [&](void) {
        double total = 0.0;
        for (const auto& accountPtr : accounts) {
            total += accountPtr->getBalance();
        }
        return total;
    };

    const double totalBefore = totalFunds();
    std::cout << "Total funds before: " << totalBefore << '\n';

    TransactionLog log;

    std::atomic<int> successCount{0};
    std::atomic<int> failCount{0};

    const auto startTime = std::chrono::steady_clock::now();
    {
        ThreadPool pool(numThreads);

        for (int threadIndex = 0; threadIndex < numThreads; ++threadIndex) {
            pool.enqueue([&]() {
                std::mt19937_64 rng(std::random_device{}());
                std::uniform_int_distribution<int> accountDist(1, numAccounts);
                std::uniform_real_distribution<double> amountDist(1.0, 500.0);

                for (int transferIndex = 0; transferIndex < transfersPerThread; ++transferIndex) {
                    int fromId = accountDist(rng);
                    int toId = accountDist(rng);
                    while (toId == fromId) {
                        toId = accountDist(rng);
                    }

                    Account& fromAccount = *accounts[fromId - 1];
                    Account& toAccount = *accounts[toId - 1];
                    TransferService service(fromAccount, toAccount, log);
                    bool result = service.transfer(fromId, toId, amountDist(rng));
                    if (result) {
                        ++successCount;
                    } else {
                        ++failCount;
                    }
                }
            });
        }
    } // pool is destroyed here, ensuring all tasks complete before measurement

    const auto endTime = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();
    const int totalTransactions = numThreads * transfersPerThread;
    const double throughput = totalTransactions / elapsed;

    const double totalAfter = totalFunds();
    std::cout << "Total funds after: " << totalAfter << '\n';
    std::cout << "Transactions: " << totalTransactions << " in " << elapsed << " seconds\n";
    std::cout << "Throughput: " << throughput << " tx/s\n";
    std::cout << "Success count: " << successCount.load() << '\n';
    std::cout << "Fail count: " << failCount.load() << '\n';

    const double difference = std::abs(totalBefore - totalAfter);
    std::cout << "Total difference: " << difference << '\n';

    const int totalCount = successCount.load() + failCount.load();
    const bool countsMatch = (totalCount == totalTransactions);
    bool passed = (difference < 1e-6) && countsMatch;

    if (passed) {
        std::cout << "PASSED" << '\n';
    } else {
        std::cout << "FAILED" << '\n';
    }

    assert(passed);
    return passed ? 0 : 1;
}
