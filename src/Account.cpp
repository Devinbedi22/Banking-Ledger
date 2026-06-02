#include "Account.h"
#include <mutex>
#include <shared_mutex>

Account::Account(int id_, double initialBalance)
    : id(id_), balance(initialBalance) {}

int Account::getId() const noexcept {
    return id;
}

double Account::getBalance() const {
    std::shared_lock lock(mtx);
    return balance;
}

bool Account::debit(double amount) {
    std::unique_lock lock(mtx);
    if (amount < 0.0) {
        return false;
    }
    if (balance < amount) {
        return false;
    }
    balance -= amount;
    return true;
}

void Account::credit(double amount) {
    std::unique_lock lock(mtx);
    balance += amount;
}
