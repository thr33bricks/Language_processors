// Yordan Yordanov, October 2025

#ifndef SYNT_H
#define SYNT_H

#include <vector>
#include <string>
#include <stack>
#include "symbInfo.h"

struct Quad {
    SymbolInfo* op;
    SymbolInfo* arg1;
    SymbolInfo* arg2;
    SymbolInfo* res;
};

struct LoopLabels {
    SymbolInfo* startLabel;
    SymbolInfo* endLabel;
};

class Synt {
public:
    Synt(std::vector<SymbolInfo>& symbolList, std::vector<uint32_t>& lines);
    ~Synt();
    bool Parse();
    std::vector<Quad> quads; // Vector to store all generated quads
private:
    bool exitCurlyBlock; // Flag to signal that a '}' has been found
    uint32_t inCurlyCount;
    uint32_t tokenIdx;
    SymbolInfo* token;
    uint32_t tempVarCounter; // Counter for generating temporary variables
    uint32_t labelCounter;   // Counter for generating labels
    std::stack<LoopLabels> loopStack;  // Stack to track nested loops for break/continue

    std::vector<SymbolInfo>& symbolList;
    std::vector<uint32_t>& lines;

    void z();
    void block_list();
    void semicolon();
    void stm();
    SymbolInfo* expr();  // Returns result of expression
    SymbolInfo* add_expr();  // Returns result of addition expression
    SymbolInfo* term();  // Returns result of term
    SymbolInfo* factor();  // Returns result of factor
    void GetToken();
    void SyntaxError(uint8_t errNum, const std::string &error);
    
    // Semantic analysis helper methods
    SymbolInfo* genTempVar();  // Generate a temporary variable
    uint32_t genLabel();  // Generate a label number
    void emitQuad(SymbolInfo* op, SymbolInfo* arg1, SymbolInfo* arg2, SymbolInfo* res);  // Add quad to vector
};

#endif