#include "ast.h"

FunctionType *make_function(Type* type, declarator* de);
Type *make_ptr(Type *type, declarator *de);
Type *make_type(Type* type, declarator* de);
Type *valid_type_specifier(vector<type_specifier*> tsps);
