#include "types.h"

extern LLVMContext context;

Type *direct_declarator::gen_type(Type *type)
{
    return type;
}

Type *parenthesized_declarator::gen_type(Type *type)
{
    return decl->gen_type(type);
}

Type *function_declarator::gen_type(Type *type)
{
    vector<Type*> arguments;
    bool vararg = false;
    if (!is_noparam())
    {
        for (parameter_declaration *pd : pl)
        {
            if (pd)
            {
                if (pd->decl)
                    arguments.push_back(pd->decl->gen_type(pd->ds->type));
                else
                    arguments.push_back(pd->ds->type);
            }
            else
                vararg = true;
        }
    }
    type = FunctionType::get(type, arguments, vararg);
    if (dd) type = dd->gen_type(type);
    return type;
}

Type *declarator::gen_type(Type *type)
{
    for (int i = 0; i < p.size(); ++i)
    {
        if (type->isVoidTy())
            type = Type::getInt8Ty(context);
        type = PointerType::getUnqual(type);
    }

    if (dd) type = dd->gen_type(type);
    return type;
}
