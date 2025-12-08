// Yordan Yordanov, October 2025

#ifndef SYMBINFO_H
#define SYMBINFO_H

#include <cstdint>

class SymbolInfo {
public:
    SymbolInfo();
    ~SymbolInfo();
    uint8_t code;
    uint64_t val;

    enum IdentifierTypes {
        VARIABLE, NUMBER, CHAR, LOOP, BOOL,
        CONSOLE, OPERATOR, OPERATOR2
    };

    enum Operators {
        PLUS, MINUS, EQUALS, SLASH, MULTI, 
        NOT, LESS, MORE, OPEN_BRACKET, 
        CLOSE_BRACKET, OPEN_CURLY_BRACKET, 
        CLOSE_CURLY_BRACKET, SEMICOLON, DOT
    };

    enum Operators2 {
        LINE_COMMENT, LOGICAL_EQUALS, 
        NOT_EQUALS, LESS_EQUAL, MORE_EQUAL, 
        INCREMENT, DECREMENT, AND, OR
    };

    enum Loops {
        IF, ELSE, FOR, WHILE, BREAK, 
        CONTINUE, RETURN
    };
    
    enum Bools {
        TRUE, FALSE, BOOL_NULL
    };

    enum Console {
        READ, PRINT
    };
private:

};

#endif // SYMBINFO_H
