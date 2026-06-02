#pragma once

#include <shared_mutex>

class Account {
private:
    int id;
    double balance;
    mutable std::shared_mutex mtx;

public:
    explicit Account(int id_, double initialBalance = 0.0);
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;

    int getId() const noexcept;

    // Read-only access to balance uses a shared lock so multiple readers can
    // query the balance concurrently without blocking each other.
    double getBalance() const;

    // debit modifies the account balance, so it uses a unique lock to ensure
    // exclusive access during withdrawal.
    bool debit(double amount);

    // credit modifies the account balance, so it uses a unique lock to ensure
    // exclusive access during deposit.
    void credit(double amount);

    friend class TransferService;
};
