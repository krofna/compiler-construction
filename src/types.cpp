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

StructType *make_struct(struct_or_union_specifier* sus)
{
    if (tag* t = find_tag(sus->id.str))
        return t->type;

    StructType *type = StructType::create(context);
    vector<Type*> members;
    for (struct_declaration* sd : sus->sds)
        for (declarator* dec : sd->ds)
            members.push_back(make_type(sd->ts, dec));

    type->setBody(members);
    return type;
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
        return make_struct(sus);
    assert(false);
}

Type *make_type(type_specifier* ts, declarator* de)
{
    Type *type = make_noptr_type(ts);
    type = make_ptr(type, de);
    return type;
}

FunctionType *make_function(type_specifier* ts, declarator* de)
{
    vector<Type*> arguments;
    declarator* decl = de->unparenthesize();
    function_declarator* fd = dynamic_cast<function_declarator*>(decl->dd);
    if (!fd->is_noparam())
    {
        for (parameter_declaration *pd : fd->pl)
            arguments.push_back(make_type(pd->ds->ts, pd->decl));
    }
    return FunctionType::get(make_type(ts, de), arguments, false);
}
