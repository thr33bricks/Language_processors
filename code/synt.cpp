// Grammar

// z          -> block_list
// block_list -> { stm }
// semicolon  -> ; { ; }
// stm        -> ident = expr semicolon | ident = read() semicolon | 
//               read() semicolon | print ( expr ) semicolon | 
//               if ( expr ) stm [ else stm ] | while ( expr ) stm | 
//               break semicolon | continue semicolon | "{" block_list "}"
// expr       -> add_expr [ relop add_expr ]
// add_expr   -> term { + term } | term { - term }
// relop      -> > | < | == | != | <= | >=
// term       -> factor { * factor } | factor { / factor }
// factor     -> ident | number | char | ( expr ) | ident ++ | ident --

// Yordan Yordanov, October 2025

#include <iostream>
#include <stdexcept>
#include "synt.h"
#include "settings.h"

#define REL_OPS_CNT 2
#define REL_OPS_CNT_2 4


const uint8_t relOps[REL_OPS_CNT] = {
    SymbolInfo::MORE, SymbolInfo::LESS
};

const uint8_t relOps2[REL_OPS_CNT_2] = {
    SymbolInfo::LOGICAL_EQUALS, SymbolInfo::NOT_EQUALS, 
    SymbolInfo::LESS_EQUAL, SymbolInfo::MORE_EQUAL
};

struct Quad {
    SymbolInfo* op;
    SymbolInfo* arg1;
    SymbolInfo* arg2;
    // res;
};


Synt::Synt(std::vector<SymbolInfo>& symbolList, std::vector<uint32_t>& lines)
        : symbolList(symbolList), lines(lines) {
    tokenIdx = 0;
    exitCurlyBlock = false;
    inCurlyBlock = false;
    token = nullptr;
}

void Synt::SyntaxError(uint8_t errNum, const std::string &error){
    uint32_t line = lines[tokenIdx-1];
    if(error != "" && ERROR){
        std::cout << "SYNTAX ERROR " << (uint16_t)errNum << ": " << error << " - line " << line << std::endl;
        throw std::runtime_error("Syntax error 1!");
    }
    else if(ERROR){
        std::cout << "SYNTAX ERROR 1: Generic error! - line " << line <<  std::endl;
        throw std::runtime_error("Syntax error 2!");
    }
}

void Synt::GetToken(){
    if(tokenIdx < symbolList.size())
        token = &symbolList[tokenIdx++];
    else{
        token = nullptr;
        throw std::runtime_error("Syntax - End of file!");
    }
}

void Synt::z(){
    block_list();
}

void Synt::block_list(){
    while (!exitCurlyBlock)
        stm();
    exitCurlyBlock = false;
}

void Synt::semicolon(){
    if (token->code != SymbolInfo::OPERATOR || 
         token->val != SymbolInfo::SEMICOLON) 
        SyntaxError(16, "\";\" symbol expected at the end of statement!" );

    while (token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::SEMICOLON)
        GetToken();
}

void Synt::stm(){
    // handle curly block ending
    if (inCurlyBlock && token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::CLOSE_CURLY_BRACKET){
        exitCurlyBlock = true;
        return;
    }
    else if (token->code == SymbolInfo::VARIABLE){ // identifier
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::EQUALS) 
            SyntaxError(3, "\"=\" symbol expected after identifier!" ); 
        GetToken();
        
        // read()
        if (token->code == SymbolInfo::CONSOLE && 
                token->val == SymbolInfo::READ){
            GetToken();
            if (token->code != SymbolInfo::OPERATOR || 
                    token->val != SymbolInfo::OPEN_BRACKET) 
                SyntaxError(4, "\"(\" symbol expected after \"read\"!" );
            GetToken();
            if (token->code != SymbolInfo::OPERATOR || 
                    token->val != SymbolInfo::CLOSE_BRACKET) 
                SyntaxError(5, "\")\" symbol expected after \"read(\"!" );
            GetToken();
            semicolon();
        }
        else{
            expr();
            semicolon();
        }
    } 
    else if (token->code == SymbolInfo::CONSOLE && 
              token->val == SymbolInfo::READ){ // read()
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET) 
            SyntaxError(4, "\"(\" symbol expected after \"read\"!" );
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET) 
            SyntaxError(5, "\")\" symbol expected after \"read(\"!" );
        GetToken();
        semicolon();
    }
    else if (token->code == SymbolInfo::CONSOLE && 
              token->val == SymbolInfo::PRINT){ // print
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET) 
            SyntaxError(6, "\"(\" symbol expected after \"print\"!" );
        GetToken();
        expr();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET) 
            SyntaxError(7, "\")\" symbol expected after \"print(expression\"!" );
        GetToken();
        semicolon();
    } 
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::IF){ // if
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET) 
            SyntaxError(8, "\"(\" symbol expected after \"if\"!" );
        GetToken();
        expr();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET) 
            SyntaxError(9, "\")\" symbol expected after \"if(expression\"!" );
        GetToken();
        stm();
        if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::ELSE){ // else
            GetToken();
            stm();
        }
    } 
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::WHILE){ // while
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET)
            SyntaxError(10, "\"(\" symbol expected after \"while\"!" );
        GetToken();
        expr();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET)
            SyntaxError(11, "\")\" symbol expected after \"while(expression\"!" );
        GetToken();
        stm();
    } 
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::BREAK){ // break
        GetToken();
        semicolon();
    }
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::CONTINUE){ // continue
        GetToken();
        semicolon();
    } 
    else if (token->code == SymbolInfo::OPERATOR && 
              token->val == SymbolInfo::OPEN_CURLY_BRACKET){ // block
        inCurlyBlock = true;
        GetToken();
        block_list();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_CURLY_BRACKET)
            SyntaxError(12, "\"}\" symbol expected at the end of block!" );
        GetToken();
        inCurlyBlock = false;
    }
    else SyntaxError(13, "Statement cannot be recognized!" );
}

void Synt::expr(){
    add_expr();
    bool relOpFound = false; // check for relational ops
    for (uint8_t i = 0; i < REL_OPS_CNT; ++i){
        if (token->code == SymbolInfo::OPERATOR && 
                token->val == relOps[i]){
            relOpFound = true;
            break;
        }  
    }
    for (uint8_t i = 0; i < REL_OPS_CNT_2; ++i){
        if (token->code == SymbolInfo::OPERATOR2 && 
                token->val == relOps2[i]){
            relOpFound = true;
            break;
        }  
    }
    
    if (relOpFound){
        GetToken();
        add_expr();
    }
}

// term [+/- term]
void Synt::add_expr(){ 
    term();
    while (token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::PLUS ||
           token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::MINUS){
        GetToken();
        term();
    }
}

// term 
void Synt::term(){
    factor();
    while (token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::MULTI ||
           token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::SLASH){
        GetToken();
        factor();
    }
}

// factor
void Synt::factor(){
    if (token->code == SymbolInfo::NUMBER)
        GetToken();
    else if (token->code == SymbolInfo::VARIABLE){
        GetToken();

        if (token->code == SymbolInfo::OPERATOR2 && 
                 token->val == SymbolInfo::INCREMENT ||
                token->code == SymbolInfo::OPERATOR2 && 
                 token->val == SymbolInfo::DECREMENT)
            GetToken();
    }
    else if (token->code == SymbolInfo::CHAR)
        GetToken();
    else if (token->code == SymbolInfo::OPERATOR && 
              token->val == SymbolInfo::OPEN_BRACKET){
        GetToken();
        expr();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET)
            SyntaxError(14, "\")\" symbol expected after expression to form a factor!" );
        GetToken();
    }
    else SyntaxError(15, "Factor cannot be recognized!" );
}

bool Synt::Parse(){
    GetToken();
    try{  z(); }
    catch (const std::runtime_error& e) {
        if (std::string(e.what()) != "Syntax - End of file!")
            return false;
    }
    return true;
}

