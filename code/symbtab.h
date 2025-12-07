// Yordan Yordanov, October 2025

#ifndef SYMBTAB_H
#define SYMBTAB_H

#include <string>
#include "prefixNode.h"

// Prefix tree for symbol table
class SymbTab {
public:
    SymbTab();
    ~SymbTab(); // Destructor
    SymbolInfo* addSymbol(const std::string &symbol, uint8_t code, uint64_t val);
    void printAll();
    bool contains(const std::string &symbol) const;
private:
    PrefixNode* root;
    uint8_t charToIndex(char ch);
    char idxToChar(uint8_t idx);
};

#endif // SYMBTAB_H
