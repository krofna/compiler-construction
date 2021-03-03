#include "types.h"

extern LLVMContext context;

Type *make_ptr(Type *type, declarator *de)
{
    for (int i = 0; i < de->num_pointers(); ++i)
        type = PointerType::getUnqual(type);
    return type;
}

Type *make_type(Type* type, declarator* de)
{
    if (de) type = make_ptr(type, de);
    return type;
}

FunctionType *make_function(Type* type, declarator* de)
{
    vector<Type*> arguments;
    de = de->unparenthesize();
    function_declarator* fd = dynamic_cast<function_declarator*>(de->dd);
    bool vararg = false;
    if (!fd->is_noparam())
    {
        for (parameter_declaration *pd : fd->pl)
        {
            if (pd)
                arguments.push_back(make_type(pd->ds->type, pd->decl));
            else
                vararg = true;
        }
    }
    return FunctionType::get(make_type(type, de), arguments, vararg);
}
