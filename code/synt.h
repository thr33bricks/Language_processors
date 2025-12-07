// Yordan Yordanov, October 2025

#ifndef SYNT_H
#define SYNT_H

#include <vector>
#include <string>
#include "symbInfo.h"

class Synt {
public:
    Synt(std::vector<SymbolInfo>& symbolList, std::vector<uint32_t>& lines);
    ~Synt();
    bool Parse();
private:
    bool inCurlyBlock;
    bool exitCurlyBlock;
    uint32_t tokenIdx;
    SymbolInfo* token;

    std::vector<SymbolInfo>& symbolList;
    std::vector<uint32_t>& lines;

    void z();
    void block_list();
    void semicolon();
    void stm();
    void expr();
    void add_expr();
    void term();
    void factor();
    void GetToken();
    void SyntaxError(uint8_t errNum, const std::string &error);
};

#endif