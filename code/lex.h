// Yordan Yordanov, October 2025

#ifndef LEX_H
#define LEX_H

#include "symbtab.h"
#include <string>
#include <vector>

class Lex {
public:
    Lex();
    ~Lex();
    SymbolInfo* LexAnalyze();
    void RemoveComment();
    SymbolInfo* RecognizeIdentifier(char ch);
    SymbolInfo* RecognizeNumber(char ch);
    SymbolInfo* RecognizeChar(char ch);
    SymbolInfo* RecognizeOperator(char ch);
    uint8_t LexicalError(char ch, const std::string &error = "");
    char GetNextChar();
    uint8_t ReadFile();

    uint32_t currLine;
    bool error;

    SymbTab* st;
    std::string fileContent;
    std::vector<SymbolInfo> symbolList;
    std::vector<uint32_t> lines; // keeps all symbol lines
private:
    uint16_t varIdx;
    uint64_t fileContIdx;

    bool IsWhitespace(char ch);
    bool IsLetter(char ch);
    bool IsDigit(char ch);
    bool IsOperatorChar(char ch);
};

#endif