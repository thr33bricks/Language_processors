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
    std::string getName(uint8_t code, uint64_t val) const;
private:
    PrefixNode* root;
    uint8_t charToIndex(char ch) const;
    char idxToChar(uint8_t idx) const;
};

#endif // SYMBTAB_H
