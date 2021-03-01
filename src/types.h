#include "ast.h"

Type *make_builtin(builtin_type_specifier* bts);
StructType *make_struct(struct_or_union_specifier* sus);
Type *make_ptr(Type *type, declarator *de);
Type *make_noptr_type(type_specifier* ts, declarator* de);
Type *make_type(type_specifier* ts, declarator* de);
