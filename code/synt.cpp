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
#define REL_OPS_CNT_2 6


const uint8_t relOps[REL_OPS_CNT] = {
    SymbolInfo::MORE, SymbolInfo::LESS
};

const uint8_t relOps2[REL_OPS_CNT_2] = {
    SymbolInfo::LOGICAL_EQUALS, SymbolInfo::NOT_EQUALS, 
    SymbolInfo::LESS_EQUAL, SymbolInfo::MORE_EQUAL, 
    SymbolInfo::AND, SymbolInfo::OR
};

Synt::Synt(std::vector<SymbolInfo>& symbolList, std::vector<uint32_t>& lines)
        : symbolList(symbolList), lines(lines) {
    tokenIdx = 0;
    inCurlyCount = 0;
    exitCurlyBlock = false;
    token = nullptr;
    tempVarCounter = 0;
    labelCounter = 0;
}

Synt::~Synt(){
    // Destructor - quads vector will be automatically cleaned up
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
    while (!exitCurlyBlock && token != nullptr)
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
    if (inCurlyCount > 0 && token != nullptr && token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::CLOSE_CURLY_BRACKET){
        exitCurlyBlock = true;
        // Don't consume the token here - let the block handler consume it
        return;
    }
    else if (token->code == SymbolInfo::VARIABLE){ // identifier
        SymbolInfo* var = token;  // Save the variable
        GetToken();
        
        // Check for increment/decrement as standalone statement (a++ or a--)
        if (token->code == SymbolInfo::OPERATOR2 && 
                (token->val == SymbolInfo::INCREMENT || token->val == SymbolInfo::DECREMENT)) {
            SymbolInfo* op = token;  // Save the operator
            GetToken();
            // Generate quads for increment/decrement
            SymbolInfo* one = new SymbolInfo();
            one->code = SymbolInfo::NUMBER;
            one->val = 1;
            SymbolInfo* arithOp = new SymbolInfo();
            arithOp->code = SymbolInfo::OPERATOR;
            arithOp->val = (op->val == SymbolInfo::INCREMENT) ? SymbolInfo::PLUS : SymbolInfo::MINUS;
            // Update the variable: var = var +/- 1
            emitQuad(arithOp, var, one, var);
            semicolon();
        }
        else if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::EQUALS) {
            SyntaxError(3, "\"=\" symbol expected after identifier!" ); 
        }
        else {
            SymbolInfo* assignOp = token;  // Save the equals token before consuming
            GetToken();
            
            // read()
            if (token->code == SymbolInfo::CONSOLE && 
                    token->val == SymbolInfo::READ){
                SymbolInfo* readOp = token;  // Save the read token
                GetToken();
                if (token->code != SymbolInfo::OPERATOR || 
                        token->val != SymbolInfo::OPEN_BRACKET) 
                    SyntaxError(4, "\"(\" symbol expected after \"read\"!" );
                GetToken();
                if (token->code != SymbolInfo::OPERATOR || 
                        token->val != SymbolInfo::CLOSE_BRACKET) 
                    SyntaxError(5, "\")\" symbol expected after \"read(\"!" );
                GetToken();
                // Generate quad for read: read -> var
                emitQuad(readOp, nullptr, nullptr, var);
                semicolon();
            }
            else{
                SymbolInfo* exprResult = expr();
                // Generate quad for assignment: var = exprResult
                emitQuad(assignOp, exprResult, nullptr, var);
                semicolon();
            }
        }
    } 
    else if (token->code == SymbolInfo::CONSOLE && 
              token->val == SymbolInfo::READ){ // read()
        SymbolInfo* readOp = token;  // Save the read token
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET) 
            SyntaxError(4, "\"(\" symbol expected after \"read\"!" );
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET) 
            SyntaxError(5, "\")\" symbol expected after \"read(\"!" );
        GetToken();
        // Generate quad for read (discard result)
        emitQuad(readOp, nullptr, nullptr, nullptr);
        semicolon();
    }
    else if (token->code == SymbolInfo::CONSOLE && 
              token->val == SymbolInfo::PRINT){ // print
        SymbolInfo* printOp = token;  // Save the print token
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET) 
            SyntaxError(6, "\"(\" symbol expected after \"print\"!" );
        GetToken();
        SymbolInfo* exprResult = expr();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET) 
            SyntaxError(7, "\")\" symbol expected after \"print(expression\"!" );
        GetToken();
        // Generate quad for print: print exprResult
        emitQuad(printOp, exprResult, nullptr, nullptr);
        semicolon();
    } 
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::IF){ // if
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET) 
            SyntaxError(8, "\"(\" symbol expected after \"if\"!" );
        GetToken();
        SymbolInfo* condition = expr();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET) 
            SyntaxError(9, "\")\" symbol expected after \"if(expression\"!" );
        GetToken();
        
        // Generate label for else/end
        uint32_t elseLabel = genLabel();
        SymbolInfo* elseLabelSym = new SymbolInfo();
        elseLabelSym->code = SymbolInfo::VARIABLE;  // Use VARIABLE code for labels
        elseLabelSym->val = 10000 + elseLabel;  // Offset to distinguish from regular vars
        
        // Generate quad: if condition is false, goto elseLabel
        SymbolInfo* gotoOp = new SymbolInfo();
        gotoOp->code = SymbolInfo::LOOP;  // Use LOOP code for goto
        gotoOp->val = 999;  // Special value for goto
        SymbolInfo* zero = new SymbolInfo();
        zero->code = SymbolInfo::NUMBER;
        zero->val = 0;
        SymbolInfo* equalsOp = new SymbolInfo();
        equalsOp->code = SymbolInfo::OPERATOR2;
        equalsOp->val = SymbolInfo::LOGICAL_EQUALS;
        SymbolInfo* notCond = genTempVar();
        emitQuad(equalsOp, condition, zero, notCond);
        emitQuad(gotoOp, notCond, nullptr, elseLabelSym);
        
        stm();  // Process if statement
        
        bool hasElse = false;
        if (token != nullptr && token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::ELSE){ // else
            hasElse = true;
            GetToken();
            
            // Generate label for end of if-else
            uint32_t endLabel = genLabel();
            SymbolInfo* endLabelSym = new SymbolInfo();
            endLabelSym->code = SymbolInfo::VARIABLE;
            endLabelSym->val = 10000 + endLabel;
            
            // Generate quad: goto endLabel (skip else)
            emitQuad(gotoOp, nullptr, nullptr, endLabelSym);
            
            // Emit else label
            emitQuad(nullptr, nullptr, nullptr, elseLabelSym);  // Label quad
            
            stm();  // Process else statement
            
            // Emit end label
            emitQuad(nullptr, nullptr, nullptr, endLabelSym);  // Label quad
        }
        else {
            // Emit else label (end of if)
            emitQuad(nullptr, nullptr, nullptr, elseLabelSym);  // Label quad
        }
    } 
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::WHILE){ // while
        GetToken();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::OPEN_BRACKET)
            SyntaxError(10, "\"(\" symbol expected after \"while\"!" );
        GetToken();
        
        // Generate labels for while loop
        uint32_t startLabel = genLabel();
        uint32_t endLabel = genLabel();
        SymbolInfo* startLabelSym = new SymbolInfo();
        startLabelSym->code = SymbolInfo::VARIABLE;
        startLabelSym->val = 10000 + startLabel;
        SymbolInfo* endLabelSym = new SymbolInfo();
        endLabelSym->code = SymbolInfo::VARIABLE;
        endLabelSym->val = 10000 + endLabel;
        
        // Push loop labels onto stack for break/continue
        LoopLabels loopLabels;
        loopLabels.startLabel = startLabelSym;
        loopLabels.endLabel = endLabelSym;
        loopStack.push(loopLabels);
        
        // Emit start label
        emitQuad(nullptr, nullptr, nullptr, startLabelSym);  // Label quad
        
        SymbolInfo* condition = expr();
        if (token->code != SymbolInfo::OPERATOR || 
                token->val != SymbolInfo::CLOSE_BRACKET)
            SyntaxError(11, "\")\" symbol expected after \"while(expression\"!" );
        GetToken();
        
        // Generate quad: if condition is false (0), goto endLabel
        // We want to jump to endLabel if condition == 0
        SymbolInfo* gotoOp = new SymbolInfo();
        gotoOp->code = SymbolInfo::LOOP;
        gotoOp->val = 999;  // Special value for goto
        // Conditional goto: if condition == 0, goto endLabel
        SymbolInfo* zero = new SymbolInfo();
        zero->code = SymbolInfo::NUMBER;
        zero->val = 0;
        SymbolInfo* equalsOp = new SymbolInfo();
        equalsOp->code = SymbolInfo::OPERATOR2;
        equalsOp->val = SymbolInfo::LOGICAL_EQUALS;
        SymbolInfo* isFalse = genTempVar();
        emitQuad(equalsOp, condition, zero, isFalse);
        // If isFalse != 0 (i.e., condition == 0), goto endLabel
        emitQuad(gotoOp, isFalse, nullptr, endLabelSym);
        
        stm();  // Process while body
        
        // Generate quad: goto startLabel (loop back)
        emitQuad(gotoOp, nullptr, nullptr, startLabelSym);
                
        // Emit end label
        emitQuad(nullptr, nullptr, nullptr, endLabelSym);  // Label quad
        
        // Pop loop labels from stack
        loopStack.pop();
        
        // Note: After processing while body, we continue here
        // The while loop is complete, so we return to the caller
    } 
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::BREAK){ // break
        GetToken();
        if (loopStack.empty()) {
            SyntaxError(17, "\"break\" statement not inside a loop!" );
        }
        // Generate quad: goto endLabel (exit the loop)
        SymbolInfo* gotoOp = new SymbolInfo();
        gotoOp->code = SymbolInfo::LOOP;
        gotoOp->val = 999;  // Special value for goto
        emitQuad(gotoOp, nullptr, nullptr, loopStack.top().endLabel);
        semicolon();
    }
    else if (token->code == SymbolInfo::LOOP && 
              token->val == SymbolInfo::CONTINUE){ // continue
        GetToken();
        if (loopStack.empty()) {
            SyntaxError(18, "\"continue\" statement not inside a loop!" );
        }
        // Generate quad: goto startLabel (loop back to condition)
        SymbolInfo* gotoOp = new SymbolInfo();
        gotoOp->code = SymbolInfo::LOOP;
        gotoOp->val = 999;  // Special value for goto
        emitQuad(gotoOp, nullptr, nullptr, loopStack.top().startLabel);
        semicolon();
    } 
    else if (token->code == SymbolInfo::OPERATOR && 
              token->val == SymbolInfo::OPEN_CURLY_BRACKET){ // block
        inCurlyCount++;
        GetToken();
        block_list();
        // After block_list() returns, the closing brace should still be in token
        // (it was seen by inner stm() but not consumed - stm() just returned)
        if (token != nullptr && token->code == SymbolInfo::OPERATOR &&
                token->val == SymbolInfo::CLOSE_CURLY_BRACKET) {
            if (tokenIdx < symbolList.size()) {
                GetToken();  // Consume the closing brace
            } else {
                token = nullptr; // EOF after closing brace - avoid throwing
            }
        }
        // If token is null, we've hit EOF (which is OK if we're at the end)
        inCurlyCount--;
    }
    else SyntaxError(13, "Statement cannot be recognized!" );
}

SymbolInfo* Synt::expr(){
    SymbolInfo* result = add_expr();  // Get first add_expr
    
    bool relOpFound = false;
    SymbolInfo* relOp = nullptr;
    
    // check for relational ops
    for (uint8_t i = 0; i < REL_OPS_CNT; ++i){
        if (token->code == SymbolInfo::OPERATOR && 
                token->val == relOps[i]){
            relOpFound = true;
            relOp = token;
            break;
        }  
    }
    for (uint8_t i = 0; i < REL_OPS_CNT_2; ++i){
        if (token->code == SymbolInfo::OPERATOR2 && 
                token->val == relOps2[i]){
            relOpFound = true;
            relOp = token;
            break;
        }  
    }
    
    if (relOpFound){
        GetToken();
        SymbolInfo* right = add_expr();  // Get second add_expr
        
        // Generate quad for relational operation: temp = result relOp right
        SymbolInfo* temp = genTempVar();
        emitQuad(relOp, result, right, temp);
        result = temp;  // Result becomes the temporary variable
    }
    
    return result;
}

// term [+/- term]
SymbolInfo* Synt::add_expr(){ 
    SymbolInfo* result = term();  // Get first term
    
    while (token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::PLUS ||
           token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::MINUS){
        SymbolInfo* op = token;  // Save the operator
        GetToken();
        SymbolInfo* right = term();  // Get second term
        
        // Generate quad for operation: temp = result op right
        SymbolInfo* temp = genTempVar();
        emitQuad(op, result, right, temp);
        result = temp;  // Result becomes the temporary variable
    }
    
    return result;
}

// term 
SymbolInfo* Synt::term(){
    SymbolInfo* result = factor();  // Get first factor
    
    while (token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::MULTI ||
           token->code == SymbolInfo::OPERATOR && 
            token->val == SymbolInfo::SLASH){
        SymbolInfo* op = token;  // Save the operator
        GetToken();
        SymbolInfo* right = factor();  // Get second factor
        
        // Generate quad for operation: temp = result op right
        SymbolInfo* temp = genTempVar();
        emitQuad(op, result, right, temp);
        result = temp;  // Result becomes the temporary variable
    }
    
    return result;
}

// factor

SymbolInfo* Synt::factor(){
    SymbolInfo* result = nullptr;

    // Handle unary plus/minus
    if (token != nullptr && token->code == SymbolInfo::OPERATOR && token->val == SymbolInfo::PLUS) {
        // unary plus: skip and parse next factor
        GetToken();
        return factor();
    }
    if (token != nullptr && token->code == SymbolInfo::OPERATOR && token->val == SymbolInfo::MINUS) {
        // unary minus: compute 0 - <factor>
        GetToken();
        SymbolInfo* rhs = factor();
        SymbolInfo* zero = new SymbolInfo();
        zero->code = SymbolInfo::NUMBER;
        zero->val = 0;
        SymbolInfo* minusOp = new SymbolInfo();
        minusOp->code = SymbolInfo::OPERATOR;
        minusOp->val = SymbolInfo::MINUS;
        SymbolInfo* temp = genTempVar();
        emitQuad(minusOp, zero, rhs, temp);
        return temp;
    }

    if (token == nullptr) SyntaxError(15, "Factor cannot be recognized!" );

    if (token->code == SymbolInfo::NUMBER){
        result = token;  // Return the number itself
        GetToken();
    }
    else if (token->code == SymbolInfo::VARIABLE){
        SymbolInfo* var = token;  // Save the variable
        GetToken();

        if (token != nullptr && token->code == SymbolInfo::OPERATOR2 &&
                 token->val == SymbolInfo::INCREMENT){
            // Generate quad for increment: var = var + 1
            SymbolInfo* one = new SymbolInfo();
            one->code = SymbolInfo::NUMBER;
            one->val = 1;
            SymbolInfo* plusOp = new SymbolInfo();
            plusOp->code = SymbolInfo::OPERATOR;
            plusOp->val = SymbolInfo::PLUS;
            result = genTempVar();
            emitQuad(plusOp, var, one, result);
            // Also update the original variable
            emitQuad(plusOp, var, one, var);
            GetToken();
        }
        else if (token != nullptr && token->code == SymbolInfo::OPERATOR2 &&
                 token->val == SymbolInfo::DECREMENT){
            // Generate quad for decrement: var = var - 1
            SymbolInfo* one = new SymbolInfo();
            one->code = SymbolInfo::NUMBER;
            one->val = 1;
            SymbolInfo* minusOp = new SymbolInfo();
            minusOp->code = SymbolInfo::OPERATOR;
            minusOp->val = SymbolInfo::MINUS;
            result = genTempVar();
            emitQuad(minusOp, var, one, result);
            // Also update the original variable
            emitQuad(minusOp, var, one, var);
            GetToken();
        }
        else {
            result = var;  // Just return the variable
        }
    }
    else if (token->code == SymbolInfo::CHAR){
        result = token;  // Return the char itself
        GetToken();
    }
    else if (token->code == SymbolInfo::OPERATOR &&
              token->val == SymbolInfo::OPEN_BRACKET){
        GetToken();
        result = expr();  // Get result from expression
        if (token->code != SymbolInfo::OPERATOR ||
                token->val != SymbolInfo::CLOSE_BRACKET)
            SyntaxError(14, "\")\" symbol expected after expression to form a factor!" );
        GetToken();
    }
    else SyntaxError(15, "Factor cannot be recognized!" );
    
    return result;
}


// Semantic analysis helper methods
SymbolInfo* Synt::genTempVar(){
    // Create a temporary variable SymbolInfo
    // Use high offset (50000) to avoid conflicts with regular variables
    SymbolInfo* temp = new SymbolInfo();
    temp->code = SymbolInfo::VARIABLE;
    temp->val = 50000 + tempVarCounter++;  // Use offset to distinguish from regular vars
    return temp;
}

uint32_t Synt::genLabel(){
    return labelCounter++;
}

void Synt::emitQuad(SymbolInfo* op, SymbolInfo* arg1, SymbolInfo* arg2, SymbolInfo* res){
    // Broad EMITQUAD debug: show any label result quads
    if (res != nullptr && res->code == SymbolInfo::VARIABLE && res->val >= 10000) {
    }
    // Broad EMITQUAD debug: show any label result quads
    if (res != nullptr && res->code == SymbolInfo::VARIABLE && res->val >= 10000) {
    }
    if (op != nullptr && op->code == SymbolInfo::LOOP && op->val == 999 && arg1 == nullptr && res != nullptr && res->code == SymbolInfo::VARIABLE && res->val >= 10000) {
    }
    Quad quad;
    quad.op = op;
    quad.arg1 = arg1;
    quad.arg2 = arg2;
    quad.res = res;
    quads.push_back(quad);
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
