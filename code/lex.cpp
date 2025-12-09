// Yordan Yordanov, October 2025

#include "lex.h"
#include "settings.h"

#include <iostream>
#include <fstream>
#include <sstream>

using std::string;


#define SUPP_CHARS_CNT 17
#define OP_CNT 14
#define OP2_CNT 9
#define LOOPS_CNT 7
#define BOOLS_CNT 3
#define CONSOLE_CNT 2

const char suppChars[SUPP_CHARS_CNT] = {'+', '-', '=', '/', '*', '!', '<', '>', '(', ')', '{', '}', ';', '.', '\'', '&', '|'};
const char operators[OP_CNT] = {'+', '-', '=', '/', '*', '!', '<', '>', '(', ')', '{', '}', ';', '.'};
const char* operators_2[OP2_CNT] = {"//", "==", "!=", "<=", ">=", "++", "--", "&&", "||"};

const char* loops[LOOPS_CNT] = {"if", "else", "for", "while", "break", "continue", "return"};
const char* bools[BOOLS_CNT] = {"true", "false", "null"};
const char* console[CONSOLE_CNT] = {"read", "print"};


Lex::Lex(){
    varIdx = 0;
    currLine = 1;
    fileContIdx = 0;
    error = false;

    st = new SymbTab();
    ReadFile();
}

Lex::~Lex(){
    // TO DO
    st->~SymbTab();
}

SymbolInfo* Lex::LexAnalyze(){
    char ch = GetNextChar();
    if(ch == '\n') currLine++;

    while(IsWhitespace(ch)){
        ch = GetNextChar();
        if(ch == '\n') currLine++;
    }
    
    if(IsLetter(ch))
        return RecognizeIdentifier(ch);
    else if(IsDigit(ch))
        return RecognizeNumber(ch);
    else if(ch == '\'')
        return RecognizeChar(ch);
    else if(ch == '/'){
        // Peek next char to decide if this is a comment or the division operator
        char next = GetNextChar();
        if (next == '/') {
            // It's a single-line comment: consume rest of the line
            RemoveComment();
            return LexAnalyze();
        } else {
            // Not a comment: push the peeked character back and treat '/' as an operator
            fileContIdx--; 
            return RecognizeOperator(ch);
        }
    }
    else if(IsOperatorChar(ch))
        return RecognizeOperator(ch);
    else
        LexicalError(ch);
    return nullptr; // unrecognized char or \0
}

void Lex::RemoveComment() {
    // Called when we've already consumed the second '/'
    // Consume characters until end of current line or EOF.
    while (true) {
        char ch = GetNextChar();
        if (ch == '\n' || ch == '\0') {
            // leave the newline (or EOF) for the caller to handle,
            // so line counting in LexAnalyze remains correct
            fileContIdx--;
            break;
        }
        // otherwise keep consuming
    }
}

SymbolInfo* Lex::RecognizeIdentifier(char ch){
    string buffer = "";
    while (IsLetter(ch) || IsDigit(ch) || ch == '_'){
        buffer += ch;
        ch = GetNextChar();
    }
    fileContIdx--;

    for(uint8_t i = 0; i < LOOPS_CNT; ++i){
        if(string(buffer) == string(loops[i]))
            return st->addSymbol(string(buffer), SymbolInfo::LOOP, i);
    }

    for(uint8_t i = 0; i < BOOLS_CNT; ++i){
        if(string(buffer) == string(bools[i]))
            return st->addSymbol(string(buffer), SymbolInfo::BOOL, i);
    }

    for(uint8_t i = 0; i < CONSOLE_CNT; ++i){
        if(string(buffer) == string(console[i]))
            return st->addSymbol(string(buffer), SymbolInfo::CONSOLE, i);
    }

    return st->addSymbol(buffer, SymbolInfo::VARIABLE, varIdx++);
}

// It supports only non-negative integers for now
// And only whole numbers (no floats)
// But my lexical analyzer supports the dot operator '.'
// So maybe I'll add floats later
SymbolInfo* Lex::RecognizeNumber(char ch){
    string buffer = "";
    while (IsDigit(ch)){
        buffer += ch;
        ch = GetNextChar();
    }
    fileContIdx--;

    return st->addSymbol(buffer, SymbolInfo::NUMBER, (uint64_t)std::stoul(buffer));
}

SymbolInfo* Lex::RecognizeChar(char ch){
    char ch2 = GetNextChar();
    char ch3 = GetNextChar();
    char ch4 = GetNextChar();
    char* constCh = new char[4];
    
    if(ch3 == '\''){
        char* constCh = new char[4];
        constCh[0] = ch;
        constCh[1] = ch2;
        constCh[2] = ch3;
        constCh[3] = '\0';

        fileContIdx--;
        return st->addSymbol(string(constCh), SymbolInfo::CHAR, (uint64_t)ch2);
    }
    else if(ch4 == '\'' && ch2 == '\\'){
        char res = '\0';
        char* constCh = new char[5];
        constCh[0] = ch;
        constCh[1] = ch2;
        constCh[2] = ch3;
        constCh[3] = ch4;
        constCh[4] = '\0';

        switch(ch3){
            case 'n': res = '\n'; break;
            case 't': res = '\t'; break;
            case '\\': res = '\\'; break;
            case '\'': res = '\''; break;
            default: res = ch3; break; // unrecognized escape sequence, just take the char as is
        }

        return st->addSymbol(string(constCh), SymbolInfo::CHAR, (uint64_t)res);
    }

    LexicalError(ch2, "Single character expected!");
    return nullptr;
}

SymbolInfo* Lex::RecognizeOperator(char ch){
    char ch2 = GetNextChar();
    char* op = new char[3];
    
    if(IsOperatorChar(ch2)){
        op[0] = ch;
        op[1] = ch2;
        op[2] = '\0';

        for(uint8_t i = 0; i < OP2_CNT; ++i){
            if(string(op) == string(operators_2[i]))
                return st->addSymbol(string(op), SymbolInfo::OPERATOR2, i);
        }
    }

    fileContIdx--;
    for(uint8_t i = 0; i < OP_CNT; ++i){
        if(ch == operators[i])
            return st->addSymbol(string("")+ch, SymbolInfo::OPERATOR, i);
    }

    LexicalError(ch2, "");
    return nullptr;
}

uint8_t Lex::LexicalError(char ch, const std::string &error){
    if(ch == '\0'){
        if(DEBUG)
            std::cout << "LEX: End of file!" << std::endl;
        return 1;
    }
    else if(error != "" && ERROR){
        this->error = true;
        std::cout << "LEX ERROR 2: " << error << " '" << ch << "'" << std::endl;
        return 2;
    }
    else if(ERROR){
        this->error = true;
        std::cout << "LEX ERROR 3: Unrecognized character! '" << ch << "'" << std::endl;
        return 3;
    }
}

char Lex::GetNextChar(){
    char c = fileContIdx < fileContent.size() ? fileContent[fileContIdx] : '\0';
    fileContIdx++;
    return c;
}


uint8_t Lex::ReadFile(){
    std::ifstream file(INPUT_FILE); // Open file
    if (!file) {
        if(ERROR) std::cerr << "LEX: Unable to open file\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the whole file into the stringstream
    fileContent = buffer.str();

    return 0;
}

bool Lex::IsWhitespace(char ch) { return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r'; }
bool Lex::IsLetter(char ch) { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'); }
bool Lex::IsDigit(char ch) { return ch >= '0' && ch <= '9'; }
bool Lex::IsOperatorChar(char ch) {
    for (uint8_t i = 0; i < SUPP_CHARS_CNT; ++i)
        if (ch == suppChars[i]) return true;
    return false;
}
