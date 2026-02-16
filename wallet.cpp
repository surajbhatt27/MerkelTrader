#include "Wallet.h"
#include <iostream>
#include "CSVReader.h"

Wallet::Wallet() 
{
    // Constructor definition
}

std::ostream& operator<<(std::ostream& os, Wallet& wallet)
{
    os << wallet.toString();
    return os;
}

std::string Wallet::toString()
{
    std::string s;
    for (std::pair<std::string,double> pair : currencies)
    {
        std::string currency = pair.first;
        double amount = pair.second;
        s += currency + " : " + std::to_string(amount) + "\n";
    }
    return s;
}

void Wallet::insertCurrency(std::string type, double amount)
{
    if (amount < 0)
    {
        throw std::exception{};
    }
    
    if (currencies.count(type) == 0)
    {
        currencies[type] = amount;
    }
    else
    {
        currencies[type] += amount; 
    }
}

bool Wallet::containsCurrency(std::string type, double amount)
{
    if (currencies.count(type) == 0) 
        return false;
    else
        return currencies[type] >= amount;
}

bool Wallet::removeCurrency(std::string type, double amount)
{
    if (amount < 0)
    {
        throw std::exception{};
    }
    
    if (currencies.count(type) == 0) 
    {
        std::cout << "Wallet::removeCurrency - No currency for " << type << std::endl;
        return false; 
    }
    
    if (containsCurrency(type, amount))
    {
        currencies[type] -= amount;
        return true;
    }
    else
    {
        std::cout << "Wallet::removeCurrency - Not enough " << type << " (has " << currencies[type] << ", needs " << amount << ")" << std::endl;
        return false; 
    }
}

bool Wallet::canFulfillOrder(OrderBookEntry order)
{
    std::vector<std::string> currs = CSVReader::tokenise(order.product, '/');
    if (currs.size() != 2)
    {
        std::cerr << "Invalid product format: " << order.product << "\n";
        return false;
    }

    if (order.orderType == OrderBookType::ask)
    {
        if (!containsCurrency(currs[0], order.amount))
        {
            std::cerr << "Insufficient " << currs[0] << " in wallet for ask of " << order.amount << "\n";
            return false;
        }
    }
    else if (order.orderType == OrderBookType::bid)
    {
        double requiredFunds = order.amount * order.price;
        if (!containsCurrency(currs[1], requiredFunds))
        {
            std::cerr << "Insufficient " << currs[1] << " in wallet for bid of " << requiredFunds << "\n";
            return false;
        }
    }
    return true;
}


void Wallet::processSale(OrderBookEntry& sale)
{
    std::vector<std::string> currs = CSVReader::tokenise(sale.product,'/');
    // ask
    if (sale.orderType== OrderBookType::asksale)
    {
        double outgoingAmount = sale.amount;
        std::string outgoingCurrency = currs[0];
        double incomingAmount = sale.amount * sale.price;
        std::string incomingCurrency = currs[1];

        currencies[incomingCurrency] += incomingAmount;
        currencies[outgoingCurrency] -= outgoingAmount;
    }
    // bid
    if (sale.orderType == OrderBookType::bidsale)
    {        
        double incomingAmount = sale.amount;
        std::string incomingCurrency = currs[0];
        double outgoingAmount = sale.amount * sale.price;
        std::string outgoingCurrency = currs[1];

        currencies[incomingCurrency] += incomingAmount;
        currencies[outgoingCurrency] -= outgoingAmount;
    }
}