#include <iostream>
#include "symbtab.h"
#include "lex.h"
#include "synt.h"
#include "executor.h"

// Declare the global used by the executor implementation
extern SymbTab* GLOBAL_ST;

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
    if(syntSuccess) {
        std::cout << "Syntax analysis successful!" << std::endl;
        //std::cout << "Total quads generated: " << synt->quads.size() << std::endl;
        
        // Set the global symbol table pointer for name lookups in executor
        GLOBAL_ST = lex->st;

        // Execute the quads
        Executor* executor = new Executor(synt->quads);
        executor->PrintQuads();  // Print all generated quads
        executor->Execute();     // Execute the quads
        
        delete executor;
    }
    else {
        std::cout << "Syntax analysis failed!" << std::endl;
    }

    delete synt;
    delete lex;
    return 0;
}
