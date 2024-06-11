#ifndef TEA_AST_FUNCTIONS_FWD_HEADER
#define TEA_AST_FUNCTIONS_FWD_HEADER

#include "Compiler/ASTNodes/AST.hpp"
#include "Compiler/type-check/TypeCheckState.hpp"
#include "Compiler/code-gen/Assembler.hpp"

void
ast_dfs(const AST &ast, uint node, std::function<void(uint, size_t)> callback, size_t depth = 0);

std::string
ast_to_str(const AST &ast, uint node);

void
ast_print(const AST &ast, uint node, const char *prefix);

void
ast_type_check(AST &ast, uint node, TypeCheckState &type_check_state);

void
ast_code_gen(AST &ast, uint node, Assembler &assembler);

void
ast_get_value(AST &ast, uint node, Assembler &assembler, uint8_t result_reg);

void
ast_store(AST &ast, uint node, Assembler &assembler, uint8_t value_reg);

#endif