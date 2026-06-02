#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

struct LedgerEntry {
    int txn_id;
    int from_id;
    int to_id;
    double amount;
    std::string status;
    std::string timestamp;
};

class TransactionLog {
private:
    std::vector<LedgerEntry> entries;
    mutable std::mutex mtx;
    std::atomic<int> nextTxnId{1};

public:
    TransactionLog() = default;

    // logTransaction appends a new ledger entry in a thread-safe manner.
    // A mutex is used to protect the vector from concurrent modifications,
    // while the transaction id is generated atomically without locking.
    int logTransaction(int fromId, int toId, double amount,
                       const std::string& status,
                       const std::string& timestamp);

    // printAll acquires the mutex to safely read and print all entries.
    void printAll() const;
};
