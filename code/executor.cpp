#include <iostream>
#include <iomanip>
#include <cstdint>
#include <functional>
#include <string>
#include "executor.h"
#include "symbtab.h"
#include "symbInfo.h"
#include "settings.h"

// Global pointer to symbol table (set from main)
// main should set GLOBAL_ST = lex->st;
SymbTab* GLOBAL_ST = nullptr;

Executor::Executor(std::vector<Quad>& quads) : quads(quads) {
    buildLabelMap();
}

Executor::~Executor() {
    // Destructor
}

// Helper to format a SymbolInfo as a readable name
static std::string formatSymbol(SymbolInfo* s, SymbTab* symbtab, const std::function<bool(SymbolInfo*)>& isLabelFn) {
    if (!s) return "_";
    if (s->code == SymbolInfo::NUMBER) return std::to_string(s->val);
    if (s->code == SymbolInfo::CHAR) {
        std::string out = "'";
        out.push_back(static_cast<char>(s->val));
        out += "'";
        return out;
    }
    if (s->code == SymbolInfo::VARIABLE) {
        if (isLabelFn(s)) return "L" + std::to_string(s->val - 10000);
        if (s->val >= 50000) return "t" + std::to_string(s->val - 50000);
        // Try to get name from symbol table
        if (symbtab) {
            std::string name = symbtab->getName(s->code, s->val);
            if (!name.empty()) return name;
        }
        // fallback to numeric id
        return "v" + std::to_string(s->val);
    }
    return "_";
}

void Executor::buildLabelMap() {
    for (uint32_t i = 0; i < quads.size(); ++i) {
        if (quads[i].op == nullptr && quads[i].res != nullptr && isLabel(quads[i].res)) {
            labelMap[quads[i].res->val] = i;
        }
    }
}

bool Executor::isLabel(SymbolInfo* sym) {
    if (sym == nullptr) return false;
    // Labels are in range 10000-49999, temp variables are 50000+
    return sym->code == SymbolInfo::VARIABLE && sym->val >= 10000 && sym->val < 50000;
}

uint32_t Executor::findLabelIndex(SymbolInfo* label) {
    if (label == nullptr || !isLabel(label)) return UINT32_MAX;
    auto it = labelMap.find(label->val);
    if (it != labelMap.end()) {
        return it->second;
    }
    return UINT32_MAX;
}

int64_t Executor::getValue(SymbolInfo* sym) {
    if (sym == nullptr) return 0;

    if (sym->code == SymbolInfo::NUMBER) {
        return static_cast<int64_t>(sym->val);
    }
    else if (sym->code == SymbolInfo::CHAR) {
        return static_cast<int64_t>(sym->val);
    }
    else if (sym->code == SymbolInfo::VARIABLE) {
        // Handle both regular variables and temp variables
        uint64_t key = sym->val;
        auto it = variables.find(key);
        if (it != variables.end()) {
            return it->second;
        }
        return 0;  // Uninitialized variable defaults to 0
    }
    return 0;
}

void Executor::setValue(SymbolInfo* sym, int64_t value) {
    if (sym == nullptr) return;

    if (sym->code == SymbolInfo::VARIABLE) {
        variables[sym->val] = value;
    }
}

void Executor::PrintQuads() {
    std::cout << "\n=== Generated Quads ===" << std::endl;
    for (uint32_t i = 0; i < quads.size(); ++i) {
        printQuad(quads[i], i);
    }
    std::cout << "======================\n" << std::endl;
}

void Executor::printQuad(const Quad& quad, uint32_t index) {
    std::cout << "[" << std::setw(3) << index << "] ";

    // Label quad
    if (quad.op == nullptr && quad.res != nullptr && isLabel(quad.res)) {
        std::cout << "LABEL L" << (quad.res->val - 10000) << ":" << std::endl;
        return;
    }

    // Operation
    if (quad.op == nullptr) {
        std::cout << "NOP";
    }
    else if (quad.op->code == SymbolInfo::OPERATOR) {
        switch (quad.op->val) {
            case SymbolInfo::PLUS: std::cout << "+"; break;
            case SymbolInfo::MINUS: std::cout << "-"; break;
            case SymbolInfo::MULTI: std::cout << "*"; break;
            case SymbolInfo::SLASH: std::cout << "/"; break;
            case SymbolInfo::EQUALS: std::cout << "="; break;
            case SymbolInfo::LESS: std::cout << "<"; break;
            case SymbolInfo::MORE: std::cout << ">"; break;
            default: std::cout << "OP" << (int)quad.op->val; break;
        }
    }
    else if (quad.op->code == SymbolInfo::OPERATOR2) {
        switch (quad.op->val) {
            case SymbolInfo::LOGICAL_EQUALS: std::cout << "=="; break;
            case SymbolInfo::NOT_EQUALS: std::cout << "!="; break;
            case SymbolInfo::LESS_EQUAL: std::cout << "<="; break;
            case SymbolInfo::MORE_EQUAL: std::cout << ">="; break;
            case SymbolInfo::AND: std::cout << "&&"; break;
            case SymbolInfo::OR: std::cout << "||"; break;
            default: std::cout << "OP2_" << (int)quad.op->val; break;
        }
    }
    else if (quad.op->code == SymbolInfo::CONSOLE) {
        if (quad.op->val == SymbolInfo::READ) {
            std::cout << "READ";
        }
        else if (quad.op->val == SymbolInfo::PRINT) {
            std::cout << "PRINT";
        }
    }
    else if (quad.op->code == SymbolInfo::LOOP) {
        if (quad.op->val == 999) {
            std::cout << "GOTO";
        }
        else if (quad.op->val == SymbolInfo::BREAK) {
            std::cout << "BREAK";
        }
        else if (quad.op->val == SymbolInfo::CONTINUE) {
            std::cout << "CONTINUE";
        }
    }
    else {
        std::cout << "OP_" << (int)quad.op->code << "_" << (int)quad.op->val;
    }

    // Arguments & result using symbol formatting (prefer actual names when available)
    auto isLabelFn = [this](SymbolInfo* s){ return isLabel(s); };
    SymbTab* lookup = GLOBAL_ST;
    std::cout << " " << formatSymbol(quad.arg1, lookup, isLabelFn);
    std::cout << " " << formatSymbol(quad.arg2, lookup, isLabelFn);
    std::cout << " -> " << formatSymbol(quad.res, lookup, isLabelFn) << std::endl;
}

void Executor::Execute() {
    std::cout << "\n=== Executing Quads ===" << std::endl;

    uint32_t pc = 0;  // Program counter

    while (pc < quads.size()) {
        const Quad& quad = quads[pc];

        // Skip label quads (they're just markers)
        if (quad.op == nullptr && quad.res != nullptr && isLabel(quad.res)) {
            pc++;
            continue;
        }

        // Handle different operations
        if (quad.op == nullptr) {
            pc++;
            continue;
        }

        // GOTO operation
        if (quad.op->code == SymbolInfo::LOOP && quad.op->val == 999) {
            if (quad.arg1 == nullptr) {
                // Unconditional goto
                uint32_t target = findLabelIndex(quad.res);
                if (target != UINT32_MAX) {
                    pc = target;
                    continue;
                }
            }
            else {
                // Conditional goto: if arg1 != 0, goto res
                int64_t cond = getValue(quad.arg1);
                if (cond != 0) {
                    uint32_t target = findLabelIndex(quad.res);
                    if (target != UINT32_MAX) {
                        pc = target;
                        continue;
                    }
                }
                // If condition is false, fall through to next instruction
            }
        }
        else if (quad.op->code == SymbolInfo::CONSOLE && quad.op->val == SymbolInfo::READ) {
            if (quad.res != nullptr) {
                std::cout << "Input: ";
                // If destination is known to be CHAR, read single char
                auto itType = varTypes.find(quad.res->val);
                if (itType != varTypes.end() && itType->second == SymbolInfo::CHAR) {
                    char c;
                    std::cin >> std::ws >> c; // read next non-whitespace char
                    setValue(quad.res, static_cast<int64_t>(c));
                    varTypes[quad.res->val] = SymbolInfo::CHAR;
                } else {
                    // Read a token and try to interpret it
                    std::string token;
                    if (!(std::cin >> token)) {
                        // input failure, default to 0
                        setValue(quad.res, 0);
                        varTypes[quad.res->val] = SymbolInfo::NUMBER;
                    } else {
                        // token like 'a'
                        if (token.size() >= 3 && token.front() == 39 && token.back() == 39) {
                            char c = token[1];
                            setValue(quad.res, static_cast<int64_t>(c));
                            varTypes[quad.res->val] = SymbolInfo::CHAR;
                        } else {
                            // try parse integer
                            try {
                                long long v = std::stoll(token);
                                setValue(quad.res, static_cast<int64_t>(v));
                                varTypes[quad.res->val] = SymbolInfo::NUMBER;
                            } catch (...) {
                                // fallback: if single char token, store as char; otherwise store first char
                                if (!token.empty()) {
                                    setValue(quad.res, static_cast<int64_t>(token[0]));
                                    varTypes[quad.res->val] = SymbolInfo::CHAR;
                                } else {
                                    setValue(quad.res, 0);
                                    varTypes[quad.res->val] = SymbolInfo::NUMBER;
                                }
                            }
                        }
                    }
                }
            } else {
                // read is used standalone, consume a token and discard
                std::string tmp;
                std::cin >> tmp;
            }
        }
        else if (quad.op->code == SymbolInfo::CONSOLE && quad.op->val == SymbolInfo::PRINT) {
            if (quad.arg1 != nullptr) {
                // If it's a char literal, print as char
                if (quad.arg1->code == SymbolInfo::CHAR) {
                    char c = static_cast<char>(quad.arg1->val);

                    if(PRINT_NEWLINE)
                        std::cout << "Output: " << c << std::endl;
                    else  std::cout << c;
                }
                else if (quad.arg1->code == SymbolInfo::VARIABLE) {
                    // If we know this variable is a CHAR, print as char
                    auto it = varTypes.find(quad.arg1->val);
                    if (it != varTypes.end() && it->second == SymbolInfo::CHAR) {
                        char c = static_cast<char>(getValue(quad.arg1));

                        if(PRINT_NEWLINE)
                            std::cout << "Output: " << c << std::endl;
                        else  std::cout << c;
                    } else {
                        int64_t value = getValue(quad.arg1);

                        if(PRINT_NEWLINE)
                            std::cout << "Output: " << value << std::endl;
                        else  std::cout << value;
                    }
                }
                else {
                    // number or fallback
                    int64_t value = getValue(quad.arg1);
                    std::cout << "Output: " << value << std::endl;
                }
            }
            pc++;
            continue;
        }

        // Assignment operation
        if (quad.op->code == SymbolInfo::OPERATOR && quad.op->val == SymbolInfo::EQUALS) {
            if (quad.res != nullptr && quad.arg1 != nullptr) {
                // Propagate type information
                if (quad.arg1->code == SymbolInfo::NUMBER) {
                    varTypes[quad.res->val] = SymbolInfo::NUMBER;
                }
                else if (quad.arg1->code == SymbolInfo::CHAR) {
                    varTypes[quad.res->val] = SymbolInfo::CHAR;
                }
                else if (quad.arg1->code == SymbolInfo::VARIABLE) {
                    auto it = varTypes.find(quad.arg1->val);
                    if (it != varTypes.end()) varTypes[quad.res->val] = it->second;
                    else varTypes[quad.res->val] = SymbolInfo::NUMBER; // default to number
                } else {
                    varTypes[quad.res->val] = SymbolInfo::NUMBER;
                }

                int64_t value = getValue(quad.arg1);
                setValue(quad.res, value);
            }
            pc++;
            continue;
        }

        // Arithmetic operations
        if (quad.op->code == SymbolInfo::OPERATOR) {
            int64_t arg1Val = getValue(quad.arg1);
            int64_t arg2Val = getValue(quad.arg2);
            int64_t result = 0;

            switch (quad.op->val) {
                case SymbolInfo::PLUS:
                    result = arg1Val + arg2Val;
                    break;
                case SymbolInfo::MINUS:
                    result = arg1Val - arg2Val;
                    break;
                case SymbolInfo::MULTI:
                    result = arg1Val * arg2Val;
                    break;
                case SymbolInfo::SLASH:
                    if (arg2Val == 0) {
                        std::cout << "ERROR: Division by zero!" << std::endl;
                        return;
                    }
                    result = arg1Val / arg2Val;
                    break;
                case SymbolInfo::LESS:
                    result = (arg1Val < arg2Val) ? 1 : 0;
                    break;
                case SymbolInfo::MORE:
                    result = (arg1Val > arg2Val) ? 1 : 0;
                    break;
                default:
                    pc++;
                    continue;
            }

            if (quad.res != nullptr) {
                setValue(quad.res, result);
                // Arithmetic results are numeric
                varTypes[quad.res->val] = SymbolInfo::NUMBER;
            }
            pc++;
            continue;
        }

        // Relational operations
        if (quad.op->code == SymbolInfo::OPERATOR2) {
            int64_t arg1Val = getValue(quad.arg1);
            int64_t arg2Val = getValue(quad.arg2);
            int64_t result = 0;

            switch (quad.op->val) {
                case SymbolInfo::LOGICAL_EQUALS:
                    result = (arg1Val == arg2Val) ? 1 : 0;
                    break;
                case SymbolInfo::NOT_EQUALS:
                    result = (arg1Val != arg2Val) ? 1 : 0;
                    break;
                case SymbolInfo::LESS_EQUAL:
                    result = (arg1Val <= arg2Val) ? 1 : 0;
                    break;
                case SymbolInfo::MORE_EQUAL:
                    result = (arg1Val >= arg2Val) ? 1 : 0;
                    break;
                case SymbolInfo::AND:
                    result = (arg1Val && arg2Val) ? 1 : 0;
                    break;
                case SymbolInfo::OR:
                    result = (arg1Val || arg2Val) ? 1 : 0;
                    break;
                default:
                    pc++;
                    continue;
            }

            if (quad.res != nullptr) {
                setValue(quad.res, result);
                // Relational results are numeric (0/1)
                varTypes[quad.res->val] = SymbolInfo::NUMBER;
            }
            pc++;
            continue;
        }

        // BREAK and CONTINUE (should be handled by loop stack, but for now just skip)
        if (quad.op->code == SymbolInfo::LOOP) {
            if (quad.op->val == SymbolInfo::BREAK || quad.op->val == SymbolInfo::CONTINUE) {
                // These should have been converted to GOTO by the parser
                // If we see them, it's an error
                std::cout << "ERROR: Unhandled break/continue!" << std::endl;
            }
        }

        pc++;
    }

    std::cout << "\n=== Execution Complete ===" << std::endl;
    std::cout << "Final variable values:" << std::endl;
    for (const auto& pair : variables) {
        uint64_t key = pair.first;
        std::string name;
        if (key >= 50000) {
            name = "t" + std::to_string(key - 50000);
        } else {
            if (GLOBAL_ST) {
                name = GLOBAL_ST->getName(SymbolInfo::VARIABLE, key);
            }
            if (name.empty()) name = "v" + std::to_string(key);
        }

        auto it = varTypes.find(key);
        if (it != varTypes.end() && it->second == SymbolInfo::CHAR) {
            char c = static_cast<char>(pair.second);
            std::cout << "  " << name << " = '" << c << "'" << std::endl;
        } else {
            std::cout << "  " << name << " = " << pair.second << std::endl;
        }
    }
}
