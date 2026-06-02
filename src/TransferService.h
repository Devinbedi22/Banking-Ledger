#pragma once

#include "Account.h"
#include "TransactionLog.h"

class TransferService {
private:
    Account& accountA;
    Account& accountB;
    TransactionLog& log;

    static std::string currentTimestamp();

public:
    TransferService(Account& accountA_, Account& accountB_, TransactionLog& log_);

    // transfer performs a funds transfer between the two managed accounts.
    // Locks for both accounts are acquired together using std::lock to avoid
    // deadlock when multiple threads transfer between the same accounts.
    bool transfer(int fromId, int toId, double amount);
};
