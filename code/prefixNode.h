// Yordan Yordanov, October 2025

#ifndef PREFIXNODE_H
#define PREFIXNODE_H

#include "settings.h"
#include "symbInfo.h"

class PrefixNode {
public:
    PrefixNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
    SymbolInfo* info;
};

#endif // PREFIXNODE_H