// Yordan Yordanov, October 2025

#include <iostream>
#include "symbtab.h"
#include "lex.h"
#include "synt.h"


int main() {
    Lex* lex = new Lex();
    Synt* synt = new Synt(lex->symbolList, lex->lines);
    
    SymbolInfo* si = lex->LexAnalyze();
    while(si != nullptr){
        lex->symbolList.push_back(*si);
        lex->lines.push_back(lex->currLine);
        si = lex->LexAnalyze();
    }

    // Test lexycal analyzer
    // std::cout << "File content after removing comments: " << lex->fileContent << std::endl;
    // lex->st->printAll();

    // Test Syntax analyzer
    bool syntSuccess = synt->Parse();
    if(syntSuccess)
        std::cout << "Syntax analysis successful!" << std::endl;
    else
        std::cout << "Syntax analysis failed!" << std::endl;

    return 0;
}
