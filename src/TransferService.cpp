#include "TransferService.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

TransferService::TransferService(Account& accountA_, Account& accountB_, TransactionLog& log_)
    : accountA(accountA_), accountB(accountB_), log(log_) {}

bool TransferService::transfer(int fromId, int toId, double amount) {
    Account* fromAccount = nullptr;
    Account* toAccount = nullptr;

    if (accountA.getId() == fromId) {
        fromAccount = &accountA;
    } else if (accountB.getId() == fromId) {
        fromAccount = &accountB;
    }

    if (accountA.getId() == toId) {
        toAccount = &accountA;
    } else if (accountB.getId() == toId) {
        toAccount = &accountB;
    }

    std::string status = "FAILED";
    bool success = false;

    if (!fromAccount || !toAccount || amount < 0.0) {
        log.logTransaction(fromId, toId, amount, status, currentTimestamp());
        return false;
    }

    if (fromAccount == toAccount) {
        // Same account transfer is a no-op but still recorded.
        status = "SUCCESS";
        log.logTransaction(fromId, toId, amount, status, currentTimestamp());
        return true;
    }

    std::unique_lock<std::shared_mutex> lockFrom(fromAccount->mtx, std::defer_lock);
    std::unique_lock<std::shared_mutex> lockTo(toAccount->mtx, std::defer_lock);
    std::lock(lockFrom, lockTo);

    if (fromAccount->balance >= amount) {
        fromAccount->balance -= amount;
        toAccount->balance += amount;
        success = true;
        status = "SUCCESS";
    }

    log.logTransaction(fromId, toId, amount, status, currentTimestamp());
    return success;
}

std::string TransferService::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_ptr = std::localtime(&timeT);
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
