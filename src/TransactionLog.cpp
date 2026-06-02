#include "TransactionLog.h"
#include <iostream>

int TransactionLog::logTransaction(int fromId, int toId, double amount,
                                   const std::string& status,
                                   const std::string& timestamp) {
    int txnId = nextTxnId.fetch_add(1, std::memory_order_relaxed);

    LedgerEntry entry{
        txnId,
        fromId,
        toId,
        amount,
        status,
        timestamp
    };

    std::lock_guard<std::mutex> lock(mtx);
    entries.push_back(std::move(entry));
    return txnId;
}

void TransactionLog::printAll() const {
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& entry : entries) {
        std::cout << "Txn " << entry.txn_id
                  << ": from=" << entry.from_id
                  << " to=" << entry.to_id
                  << " amount=" << entry.amount
                  << " status=" << entry.status
                  << " timestamp=" << entry.timestamp
                  << '\n';
    }
}
