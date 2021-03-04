#include "types.h"

extern LLVMContext context;

Type *make_ptr(Type* type, declarator* de)
{
    if (de)
        for (int i = 0; i < de->num_pointers(); ++i)
            type = PointerType::getUnqual(type);
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
                arguments.push_back(make_ptr(pd->ds->type, pd->decl));
            else
                vararg = true;
        }
    }
    return FunctionType::get(make_ptr(type, de), arguments, vararg);
}
