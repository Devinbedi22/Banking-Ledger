#include "src/Account.h"
#include <iostream>
#include <memory>
#include <vector>

int main() {
    const int accountCount = 5;
    const double initialBalance = 1000.0;

    std::vector<std::unique_ptr<Account>> accounts;
    accounts.reserve(accountCount);
    for (int i = 1; i <= accountCount; ++i) {
        accounts.push_back(std::make_unique<Account>(i, initialBalance));
    }

    double totalBalance = 0.0;
    for (const auto& accountPtr : accounts) {
        totalBalance += accountPtr->getBalance();
    }

    std::cout << "Banking Ledger Started" << std::endl;
    std::cout << "Total balance: " << totalBalance << std::endl;
    std::cout << "All systems operational" << std::endl;

    return 0;
}
