#include "types.h"

extern LLVMContext context;

Type *make_builtin(builtin_type_specifier* bts)
{
    if (bts->tok.str == "int")
        return IntegerType::get(context, 32);
    else if (bts->tok.str == "char")
        return IntegerType::get(context, 8);
    else if (bts->tok.str == "void")
        return IntegerType::get(context, 8);
    assert(false);
}

Type *make_ptr(Type *type, declarator *de)
{
    for (int i = 0; i < de->num_pointers(); ++i)
        type = PointerType::getUnqual(type);
    return type;
}

Type *make_noptr_type(type_specifier* ts)
{
    if (builtin_type_specifier* bts = dynamic_cast<builtin_type_specifier*>(ts))
        return make_builtin(bts);
    if (struct_or_union_specifier* sus = dynamic_cast<struct_or_union_specifier*>(ts))
        return find_tag(sus->id.str)->type;
    assert(false);
}

Type *make_type(type_specifier* ts, declarator* de)
{
    Type *type = make_noptr_type(ts);
    if (de) type = make_ptr(type, de);
    return type;
}

FunctionType *make_function(type_specifier* ts, declarator* de)
{
    vector<Type*> arguments;
    declarator* decl = de->unparenthesize();
    function_declarator* fd = dynamic_cast<function_declarator*>(decl->dd);
    bool vararg = false;
    if (!fd->is_noparam())
    {
        for (parameter_declaration *pd : fd->pl)
        {
            if (pd)
                arguments.push_back(make_type(pd->ds->ts, pd->decl));
            else
                vararg = true;
        }
    }
    return FunctionType::get(make_type(ts, de), arguments, vararg);
}
