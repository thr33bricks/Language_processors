// Yordan Yordanov, October 2025

#include "symbtab.h"
#include <queue>
#include <iostream>
using namespace std;

SymbTab::SymbTab() {
    root = new PrefixNode();
    for (int i = 0; i < ALPHABET_SIZE; ++i) {
        root->children[i] = nullptr;
    }
    root->isEndOfWord = false;
    root->info = nullptr;
}

SymbTab::~SymbTab() {
    // Destructor implementation to free allocated memory
    // (not implemented here for brevity)
}

SymbolInfo* SymbTab::addSymbol(const std::string &symbol, uint8_t code, uint64_t val) {
    PrefixNode* current = root;
    for (char ch : symbol) {
        uint8_t index = charToIndex(ch);
        if (current->children[index] == nullptr) {
            current->children[index] = new PrefixNode();
            for (int i = 0; i < ALPHABET_SIZE; ++i)
                current->children[index]->children[i] = nullptr;
            current->children[index]->isEndOfWord = false;
            current->children[index]->info = nullptr;
        }
        current = current->children[index];
    }

    current->isEndOfWord = true;
    if (current->info == nullptr) {
        current->info = new SymbolInfo();
        current->info->code = code;
        current->info->val = val;

        return current->info;
    }

    return current->info; // Symbol already exists, currently retrurning existing info, maybe wong?
}

void SymbTab::printAll() {
    if (!root) return;

    // Each queue entry stores a node pointer and the word built so far
    std::queue<std::pair<const PrefixNode*, std::string>> q;
    q.push({root, ""});

    while (!q.empty()) {
        auto [node, prefix] = q.front();
        q.pop();

        // If this node marks the end of a word, print the accumulated prefix
        if (node->isEndOfWord)
            std::cout << prefix << '\n';

        // Visit all children and extend the prefix with the corresponding character
        for (int i = 0; i < ALPHABET_SIZE; ++i) {
            if (node->children[i] != nullptr) {
                char ch = idxToChar(i);
                q.push({node->children[i], prefix + ch});
            }
        }
    }
}

// 94 characters in total
uint8_t SymbTab::charToIndex(char ch){
    if(ch >= '!' && ch <= '~') // printable ASCII
        return ch - '!';
    else
        return 255; // invalid character
}

char SymbTab::idxToChar(uint8_t idx){
    if(idx <= 93) // printable ASCII
        return idx + 33;
    else
        return 255; // invalid character
}
