#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <map>
#include <vector>
#include <string>
#include "synt.h"

class Executor {
public:
    Executor(std::vector<Quad>& quads);
    ~Executor();
    void Execute();
    void PrintQuads();  // Debug function to print all quads
private:
    std::vector<Quad>& quads;
    std::map<uint64_t, int64_t> variables;  // Map variable IDs to their values
    std::map<uint64_t, uint8_t> varTypes;   // Map variable IDs to their stored type (SymbolInfo::NUMBER or SymbolInfo::CHAR)
    std::map<uint64_t, uint32_t> labelMap;  // Map label IDs to quad indices
    
    int64_t getValue(SymbolInfo* sym);
    void setValue(SymbolInfo* sym, int64_t value);
    bool isLabel(SymbolInfo* sym);
    uint32_t findLabelIndex(SymbolInfo* label);
    void buildLabelMap();  // Build map of labels to quad indices
    void printQuad(const Quad& quad, uint32_t index);
};

#endif
