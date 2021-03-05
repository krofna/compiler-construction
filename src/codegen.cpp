#include "ast.h"
#include "llvm/IR/Verifier.h"

LLVMContext context;
unique_ptr<Module> module;
static unique_ptr<IRBuilder<>> builder, alloca_builder;
static BasicBlock *continue_block = nullptr;
static BasicBlock *break_block = nullptr;

extern string unescape(const string& s);

static AllocaInst *create_alloca(Type *type, const string &var_name)
{
    alloca_builder->SetInsertPoint(alloca_builder->GetInsertBlock(),
                                   alloca_builder->GetInsertBlock()->begin());
    return alloca_builder->CreateAlloca(type, 0, var_name.c_str());
}

static GlobalVariable *create_global(Type *type, const string &var_name)
{
    return new GlobalVariable(*module, type, false,
                              GlobalValue::CommonLinkage,
                              Constant::getNullValue(type),
                              var_name.c_str());
}

static Value *create_variable(Type *type, const string &var_name)
{
    scope *s = scopes.back();
    if (s->global)
        return create_global(type, var_name);
    return create_alloca(type, var_name);
}

static Value *cast(Value *val, Type *type, BasicBlock *block = nullptr)
{
    Type *vtype = val->getType();
    if (vtype == type)
        return val;

    if (type->isPointerTy() && vtype->isPointerTy())
    {
        if (block) builder->SetInsertPoint(block);
        return builder->CreateBitCast(val, type);
    }

    if (type->isPointerTy())
    {
        if (block) builder->SetInsertPoint(block);
        return builder->CreateIntToPtr(val, type);
    }

    if (vtype->isPointerTy())
    {
        if (block) builder->SetInsertPoint(block);
        return builder->CreatePtrToInt(val, type);
    }

    if (vtype->isIntegerTy() && type->isIntegerTy())
    {
        if (block) builder->SetInsertPoint(block);
        return builder->CreateSExtOrTrunc(val, type);
    }
    return nullptr;
}

static Value *store(Value *val, Value *ptr)
{
    val = cast(val, ptr->getType()->getContainedType(0));
    if (!val)
        return nullptr;
    return builder->CreateStore(val, ptr);
}

static bool adjust_int(Value *&lhs, Value *&rhs, BasicBlock *lblock = nullptr, BasicBlock *rblock = nullptr)
{
    Type *ltype = lhs->getType();
    Type *rtype = rhs->getType();

    if (!rtype->isIntegerTy() || !ltype->isIntegerTy())
        return false;

    if (rtype->getPrimitiveSizeInBits() > ltype->getPrimitiveSizeInBits())
    {
        if (lblock) builder->SetInsertPoint(lblock);
        lhs = builder->CreateSExt(lhs, rhs->getType());
    }
    if (rtype->getPrimitiveSizeInBits() < ltype->getPrimitiveSizeInBits())
    {
        if (rblock) builder->SetInsertPoint(rblock);
        rhs = builder->CreateSExt(rhs, lhs->getType());
    }
    return true;
}

static bool adjust_ptr(Value *&lhs, Value *&rhs)
{
    Type *ltype = lhs->getType();
    Type *rtype = rhs->getType();

    if (!rtype->isPointerTy() || !ltype->isPointerTy())
        return false;

    if (ltype != rtype)
        return false;

    Type *type = Type::getInt64Ty(context);
    lhs = builder->CreatePtrToInt(lhs, type);
    rhs = builder->CreatePtrToInt(rhs, type);
    return true;
}

static bool adjust_int_ptr(Value *&lhs, Value *&rhs)
{
    if (lhs->getType() == rhs->getType())
        return true;
    if (adjust_int(lhs, rhs))
        return true;
    if (adjust_ptr(lhs, rhs))
        return true;
    return false;
}

static bool conditional_adjust(Value *&tval, Value *&fval, BasicBlock *true_block, BasicBlock *false_block)
{
    Type *ttype = tval->getType();
    Type *ftype = fval->getType();
    if (ttype == ftype)
        return true;

    if (ttype->isStructTy() || ftype->isStructTy())
    {
        if (ttype != ftype)
            return false;
        return true;
    }
    if (ttype->isPointerTy() && ftype->isPointerTy())
    {
        if (ttype != ftype)
            return false;
        return true;
    }
    if (ttype->isPointerTy())
        return fval = cast(fval, ttype, false_block);
    if (ftype->isPointerTy())
        return tval = cast(tval, ftype, true_block);
    if (adjust_int(tval, fval, true_block, false_block))
        return true;
    return false;
}

static Value *truncate(Value* cond)
{
    Type *type = cond->getType();
    if (type->isIntegerTy(1))
        return cond;

    if (type->isPointerTy())
        cond = builder->CreatePtrToInt(cond, Type::getInt64Ty(context));

    Value *zero = builder->getInt64(0);
    if (!adjust_int_ptr(cond, zero))
        return nullptr;

    return builder->CreateICmpNE(cond, zero);
}

static Value *create_gep(Value *ptr, Value *idx)
{
    idx = cast(idx, Type::getInt32Ty(context));
    return builder->CreateGEP(ptr, idx);
}

static Value *negative(Value *val)
{
    if (!val->getType()->isIntegerTy())
        return nullptr;
    return builder->CreateSub(ConstantInt::get(val->getType(), 0), val);
}

static Value *create_add(Value *lhs, Value *rhs)
{
    Type *rtype = rhs->getType();
    Type *ltype = lhs->getType();

    if (rtype->isStructTy() || ltype->isStructTy())
        return nullptr;

    if (rtype->isPointerTy() && ltype->isPointerTy())
        return nullptr;

    if (rtype->isPointerTy())
        return create_gep(rhs, lhs);

    if (ltype->isPointerTy())
        return create_gep(lhs, rhs);

    if (!adjust_int(rhs, lhs))
        return nullptr;

    return builder->CreateAdd(lhs, rhs);
}

static Value *create_div(Value *lhs, Value *rhs)
{
    if (!adjust_int(lhs, rhs))
        return nullptr;

    return builder->CreateSDiv(lhs, rhs);
}

static Value *create_rem(Value *lhs, Value *rhs)
{
    if (!adjust_int(lhs, rhs))
        return nullptr;

    return builder->CreateSRem(lhs, rhs);
}

static Value *get_size(Type *type)
{
    return ConstantExpr::getSizeOf(type);
}

static Value *create_sub(Value *lhs, Value *rhs)
{
    Type *rtype = rhs->getType();
    Type *ltype = lhs->getType();

    if (rtype->isStructTy() || ltype->isStructTy())
        return nullptr;

    if (rtype->isPointerTy() && ltype->isPointerTy())
    {
        if (rtype->getContainedType(0) != ltype->getContainedType(0))
            return nullptr;

        Type *type = Type::getInt32Ty(context);
        lhs = builder->CreatePtrToInt(lhs, type);
        rhs = builder->CreatePtrToInt(rhs, type);
        Value *diff = builder->CreateSub(lhs, rhs);
        return create_div(diff, get_size(rtype->getContainedType(0)));
    }

    if (rtype->isPointerTy())
        return nullptr;

    if (ltype->isPointerTy())
        return create_gep(lhs, negative(rhs));

    if (!adjust_int(lhs, rhs))
        return nullptr;

    return builder->CreateSub(lhs, rhs);
}

static Value *create_mul(Value *lhs, Value *rhs)
{
    if (!adjust_int(lhs, rhs))
        return nullptr;

    return builder->CreateMul(lhs, rhs);
}

Value* declarator::codegen()
{
    string identifier = get_identifier().str;
    if (dd->is_identifier() || dd->is_definition())
    {
        variable_object* vo = find_variable(identifier);
        if (vo->type->isVoidTy())
            error::reject(get_identifier());
        if (vo->type->isStructTy() && ((StructType*)vo->type)->isOpaque())
            error::reject(get_identifier());
        vo->store = create_variable(vo->type, identifier);
        return vo->store;
    }
    else
    {
        function_object* fo = find_function(identifier);
        if (!fo->function)
        {
            fo->function = Function::Create(
                fo->type,
                GlobalValue::ExternalLinkage,
                identifier.c_str(),
                *module);
        }
        return fo->function;
    }
}

void declaration::codegen()
{
    for (declarator* de : d)
        de->codegen();
}

Value* primary_expression::make_lvalue()
{
    if (tok.type == IDENTIFIER)
    {
        variable_object* vo = find_variable(tok.str);
        if (!vo)
            return nullptr;
        return vo->store;
    }
    return nullptr;
}

Value* primary_expression::make_rvalue()
{
    if (tok.type == IDENTIFIER)
    {
        if (variable_object* vo = find_variable(tok.str))
            return builder->CreateLoad(vo->store);
        if (function_object* fo = find_function(tok.str))
            return fo->function;
        error::reject(tok);
    }
    else if (tok.type == CONSTANT)
    {
        if (tok.str[0] == '\'')
        {
            int val = tok.str[1];
            return ConstantInt::get(context, APInt(8, val, true));
        }
        else
        {
            int val = stoi(tok.str);
            return ConstantInt::get(context, APInt(32, val, true));
        }
    }
    else if (tok.type == STRING_LITERAL)
    {
        string str = unescape(tok.str);
        return builder->CreateGlobalStringPtr(StringRef(str.c_str(), str.size()));
    }
    error::reject(tok);
}

Value* parenthesized_expression::make_rvalue()
{
    return expr->make_rvalue();
}

Value* parenthesized_expression::make_lvalue()
{
    return expr->make_lvalue();
}

Value* postfix_expression::make_rvalue()
{
    return pe->make_rvalue();
}

Value* postfix_expression::make_lvalue()
{
    if (pe)
        return pe->make_lvalue();
    return nullptr;
}

Value* subscript_expression::make_rvalue()
{
    Value *ptr = make_lvalue();
    if (!ptr)
        error::reject(op);
    return builder->CreateLoad(ptr);
}

Value* subscript_expression::make_lvalue()
{
    Value *l = pfe->make_rvalue();
    Value *v = create_add(l, expr->make_rvalue());
    if (!v)
        error::reject(op);
    return v;
}

Value* call_expression::make_rvalue()
{
    Value* lhs = pfe->make_rvalue();
    if (!(lhs->getType()->isPointerTy() && lhs->getType()->getContainedType(0)->isFunctionTy()))
        error::reject(opop);

    FunctionType *ftype = (FunctionType*)lhs->getType()->getContainedType(0);

    // TODO: mjesto za error?
    if (ftype->params().size() != args.size())
    {
        if (ftype->params().size() > args.size())
            error::reject(op);
        if (!ftype->isVarArg())
            error::reject(op);
    }

    vector<Value*> cargs;
    for (int i = 0; i < args.size(); ++i)
        cargs.push_back(args[i]->make_rvalue());

    for (int i = 0; i < ftype->params().size(); ++i)
    {
        cargs[i] = cast(cargs[i], ftype->params()[i]);
        if (!cargs[i])
            error::reject(opop);
    }

    return builder->CreateCall(ftype, lhs, cargs);
}

// TODO: forbid this nonsense
Value* call_expression::make_lvalue()
{
    Value *val = make_rvalue();
    Value *alloca = create_alloca(val->getType(), "tmp");
    store(val, alloca);
    return alloca;
}

Value* dot_expression::make_rvalue()
{
    Value *ptr = make_lvalue();
    if (!ptr)
        error::reject(op);
    return builder->CreateLoad(ptr);
}

Value* dot_expression::make_lvalue()
{
    Value *l = pfe->make_lvalue();
    if (!l)
        error::reject(op);

    Type *type = l->getType();
    if (type->getNumContainedTypes() != 1)
        error::reject(op);

    type = type->getContainedType(0);
    if (!type->isStructTy())
        error::reject(op);

    StructType *stype = (StructType*)type;
    extern map<string, tag*> htags;
    tag *t = htags[stype->getName().str()];
    auto it = t->indices.find(id.str);
    if (it == t->indices.end())
        error::reject(id);

    vector<Value*> indices;
    indices.push_back(builder->getInt32(0));
    indices.push_back(builder->getInt32(it->second));
    return builder->CreateInBoundsGEP(l, indices);
}

Value* arrow_expression::make_rvalue()
{
    Value *ptr = make_lvalue();
    if (!ptr)
        error::reject(op);
    return builder->CreateLoad(ptr);
}

Value* arrow_expression::make_lvalue()
{
    Value *l = pfe->make_rvalue();
    Type *type = l->getType();
    if (type->getNumContainedTypes() != 1)
        error::reject(op);

    type = type->getContainedType(0);
    if (!type->isStructTy())
        error::reject(op);

    StructType *stype = (StructType*)type;
    extern map<string, tag*> htags;
    tag *t = htags[stype->getName().str()];
    auto it = t->indices.find(id.str);
    if (it == t->indices.end())
        error::reject(id);

    vector<Value*> indices;
    indices.push_back(builder->getInt32(0));
    indices.push_back(builder->getInt32(it->second));
    return builder->CreateInBoundsGEP(l, indices);
}

Value* postfix_increment_expression::make_rvalue()
{
    Value *addr = pfe->make_lvalue();
    if (!addr)
        error::reject(op);
    Value *oval = builder->CreateLoad(addr);
    Value *nval = create_add(oval, ConstantInt::get(Type::getInt32Ty(context), 1));
    if (!nval)
        error::reject(op);
    store(nval, addr);
    return oval;
}

Value* postfix_increment_expression::make_lvalue()
{
    return nullptr;
}

Value* postfix_decrement_expression::make_rvalue()
{
    Value *addr = pfe->make_lvalue();
    if (!addr)
        error::reject(op);
    Value *oval = builder->CreateLoad(addr);
    Value *nval = create_sub(oval, ConstantInt::get(Type::getInt32Ty(context), 1));
    if (!nval)
        error::reject(op);
    store(nval, addr);
    return oval;
}

Value* postfix_decrement_expression::make_lvalue()
{
    return nullptr;
}

Value* unary_expression::make_rvalue()
{
    return pe->make_rvalue();
}

Value* unary_expression::make_lvalue()
{
    return pe->make_lvalue();
}

Value* prefix_increment_expression::make_rvalue()
{
    Value *addr = ue->make_lvalue();
    if (!addr)
        error::reject(op);
    Value *oval = builder->CreateLoad(addr);
    Value *nval = create_add(oval, ConstantInt::get(Type::getInt32Ty(context), 1));
    if (!nval)
        error::reject(op);
    store(nval, addr);
    return builder->CreateLoad(addr);
}

Value* prefix_increment_expression::make_lvalue()
{
    return nullptr;
}

Value* prefix_decrement_expression::make_rvalue()
{
    Value *addr = ue->make_lvalue();
    if (!addr)
        error::reject(op);
    Value *oval = builder->CreateLoad(addr);
    Value *nval = create_sub(oval, ConstantInt::get(Type::getInt32Ty(context), 1));
    if (!nval)
        error::reject(op);
    store(nval, addr);
    return builder->CreateLoad(addr);
}

Value* prefix_decrement_expression::make_lvalue()
{
    return nullptr;
}

Value* unary_and_expression::make_rvalue()
{
    return ce->make_lvalue();
}

Value* unary_and_expression::make_lvalue()
{
    return nullptr;
}

Value* unary_star_expression::make_rvalue()
{
    Value *addr = ce->make_rvalue();
    Type *type = addr->getType();
    if (!type->isPointerTy())
        error::reject(op);
    if (type->getContainedType(0)->isFunctionTy())
        return addr;
    return builder->CreateLoad(addr);
}

Value* unary_star_expression::make_lvalue()
{
    return ce->make_rvalue();
}

Value* unary_plus_expression::make_rvalue()
{
    return ce->make_rvalue();
}

Value* unary_plus_expression::make_lvalue()
{
    return nullptr;
}

Value* unary_minus_expression::make_rvalue()
{
    Value* r = negative(ce->make_rvalue());
    if (!r)
        error::reject(op);
    return r;
}

Value* unary_minus_expression::make_lvalue()
{
    return nullptr;
}

Value* unary_tilde_expression::make_rvalue()
{
    Value* r = ce->make_rvalue();
    return builder->CreateXor(ConstantInt::get(r->getType(), -1), r);
}

Value* unary_tilde_expression::make_lvalue()
{
    return nullptr;
}

Value* unary_not_expression::make_rvalue()
{
    Value* r = ce->make_rvalue();
    Value *zero = builder->getInt32(0);
    r = cast(r, zero->getType());
    if (!r)
        error::reject(op);
    return builder->CreateICmpEQ(r, zero);
}

Value* unary_not_expression::make_lvalue()
{
    return nullptr;
}

Value* sizeof_expression::make_rvalue()
{
    Value *val = ue->make_rvalue();
    return get_size(val->getType());
}

Value* sizeof_expression::make_lvalue()
{
    return nullptr;
}

Value* sizeof_type_expression::make_rvalue()
{
    return get_size(tn->type);
}

Value* sizeof_type_expression::make_lvalue()
{
    return nullptr;
}

Value* cast_expression::make_rvalue()
{
    if (ue)
        return ue->make_rvalue();

    Value *v = cast(ce->make_rvalue(), tn->type);
    if (!v)
        error::reject(op);
    return v;
}

Value* cast_expression::make_lvalue()
{
    if (ue)
        return ue->make_lvalue();
    return nullptr;
}

Value* multiplicative_expression::make_rvalue()
{
    return ce->make_rvalue();
}

Value* multiplicative_expression::make_lvalue()
{
    return ce->make_lvalue();
}

Value* mul_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    Value *v = create_mul(l, r);
    if (!v)
        error::reject(op);
    return v;
}

Value* mul_expression::make_lvalue()
{
    return nullptr;
}

Value* div_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    Value *d = create_div(l, r);
    if (!d)
        error::reject(op);
    return d;
}

Value* div_expression::make_lvalue()
{
    return nullptr;
}

Value* mod_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    Value *v = create_rem(l, r);
    if (!v)
        error::reject(op);
    return v;
}

Value* mod_expression::make_lvalue()
{
    return nullptr;
}

Value* additive_expression::make_rvalue()
{
    return me->make_rvalue();
}

Value* additive_expression::make_lvalue()
{
    return me->make_lvalue();
}

Value* add_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    Value *v = create_add(l, r);
    if (!v)
        error::reject(op);
    return v;
}

Value* add_expression::make_lvalue()
{
    return nullptr;
}

Value* sub_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    Value *v =create_sub(l, r);
    if (!v)
        error::reject(op);
    return v;
}

Value* sub_expression::make_lvalue()
{
    return nullptr;
}

Value* shift_expression::make_rvalue()
{
    return ae->make_rvalue();
}

Value* shift_expression::make_lvalue()
{
    return ae->make_lvalue();
}

Value* rshift_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateAShr(l, r);
}

Value* rshift_expression::make_lvalue()
{
    return nullptr;
}

Value* lshift_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateShl(l, r);
}

Value* lshift_expression::make_lvalue()
{
    return nullptr;
}

Value* relational_expression::make_rvalue()
{
    return se->make_rvalue();
}

Value* relational_expression::make_lvalue()
{
    return se->make_lvalue();
}

Value* less_expression::make_rvalue()
{
    Value *l = lhs->make_rvalue();
    Value *r = rhs->make_rvalue();
    if (!adjust_int_ptr(l, r))
        error::reject(op);
    return builder->CreateICmpSLT(l, r);
}

Value* less_expression::make_lvalue()
{
    return nullptr;
}

Value* greater_expression::make_rvalue()
{
    Value *l = lhs->make_rvalue();
    Value *r = rhs->make_rvalue();
    if (!adjust_int_ptr(l, r))
        error::reject(op);
    return builder->CreateICmpSGT(l, r);
}

Value* greater_expression::make_lvalue()
{
    return nullptr;
}

Value* less_equal_expression::make_rvalue()
{
    Value *l = lhs->make_rvalue();
    Value *r = rhs->make_rvalue();
    if (!adjust_int_ptr(l, r))
        error::reject(op);
    return builder->CreateICmpSLE(l, r);
}

Value* less_equal_expression::make_lvalue()
{
    return nullptr;
}

Value* greater_equal_expression::make_rvalue()
{
    Value *l = lhs->make_rvalue();
    Value *r = rhs->make_rvalue();
    if (!adjust_int_ptr(l, r))
        error::reject(op);
    return builder->CreateICmpSGE(l, r);
}

Value* greater_equal_expression::make_lvalue()
{
    return nullptr;
}

Value* equality_expression::make_rvalue()
{
    return re->make_rvalue();
}

Value* equality_expression::make_lvalue()
{
    return re->make_lvalue();
}

Value* equal_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    if (!adjust_int_ptr(l, r))
        error::reject(op);
    return builder->CreateICmpEQ(l, r);
}

Value* equal_expression::make_lvalue()
{
    return nullptr;
}

Value* not_equal_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    if (!adjust_int_ptr(l, r))
        error::reject(op);
    return builder->CreateICmpNE(l, r);
}

Value* not_equal_expression::make_lvalue()
{
    return nullptr;
}

Value* and_expression::make_rvalue()
{
    if (ee)
        return ee->make_rvalue();

    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    if (!adjust_int(l, r))
        error::reject(op);
    return builder->CreateAnd(l, r);
}

Value* and_expression::make_lvalue()
{
    if (ee)
        return ee->make_lvalue();
    return nullptr;
}

Value* exclusive_or_expression::make_rvalue()
{
    if (ae)
        return ae->make_rvalue();

    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    if (!adjust_int(l, r))
        error::reject(op);
    return builder->CreateXor(l, r);
}

Value* exclusive_or_expression::make_lvalue()
{
    if (ae)
        return ae->make_lvalue();
    return nullptr;
}

Value* inclusive_or_expression::make_rvalue()
{
    if (xe)
        return xe->make_rvalue();

    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    if (!adjust_int(l, r))
        error::reject(op);
    return builder->CreateOr(l, r);
}

Value* inclusive_or_expression::make_lvalue()
{
    if (xe)
        return xe->make_lvalue();
    return nullptr;
}

Value* logical_and_expression::make_lvalue()
{
    if (oe)
        return oe->make_lvalue();
    return nullptr;
}

Value* logical_and_expression::make_rvalue()
{
    if (oe)
        return oe->make_rvalue();

    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "and-header", function);
    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = truncate(lhs->make_rvalue());
    if (!cond)
        error::reject(op);

    BasicBlock *true_block = BasicBlock::Create(context, "true", function);
    BasicBlock *false_block = BasicBlock::Create(context, "false", function);
    BasicBlock *ttrue_block = BasicBlock::Create(context, "ttrue", function);
    BasicBlock *merge_block = BasicBlock::Create(context, "merge", function);

    builder->CreateCondBr(cond, true_block, false_block);

    builder->SetInsertPoint(true_block);
    Value *tcond = truncate(rhs->make_rvalue());
    if (!tcond)
        error::reject(op);
    builder->CreateCondBr(tcond, ttrue_block, false_block);

    builder->SetInsertPoint(ttrue_block);
    Value *tval = builder->getInt1(1);
    builder->CreateBr(merge_block);
    builder->SetInsertPoint(false_block);
    Value *fval = builder->getInt1(0);
    builder->CreateBr(merge_block);
    builder->SetInsertPoint(merge_block);

    PHINode *pn = builder->CreatePHI(Type::getInt1Ty(context), 2, "phi");
    pn->addIncoming(tval, ttrue_block);
    pn->addIncoming(fval, false_block);
    return pn;
}

Value* logical_or_expression::make_lvalue()
{
    if (ae)
        return ae->make_lvalue();
    return nullptr;
}

Value* logical_or_expression::make_rvalue()
{
    if (ae)
        return ae->make_rvalue();

    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "or-header", function);
    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = truncate(lhs->make_rvalue());
    if (!cond)
        error::reject(op);

    BasicBlock *true_block = BasicBlock::Create(context, "true", function);
    BasicBlock *false_block = BasicBlock::Create(context, "false", function);
    BasicBlock *ffalse_block = BasicBlock::Create(context, "ffalse", function);
    BasicBlock *merge_block = BasicBlock::Create(context, "merge", function);

    builder->CreateCondBr(cond, true_block, false_block);

    builder->SetInsertPoint(false_block);
    Value *fcond = truncate(rhs->make_rvalue());
    if (!fcond)
        error::reject(op);
    builder->CreateCondBr(fcond, true_block, ffalse_block);

    builder->SetInsertPoint(true_block);
    Value *tval = builder->getInt1(1);
    builder->CreateBr(merge_block);
    builder->SetInsertPoint(ffalse_block);
    Value *fval = builder->getInt1(0);
    builder->CreateBr(merge_block);
    builder->SetInsertPoint(merge_block);

    PHINode *pn = builder->CreatePHI(Type::getInt1Ty(context), 2, "phi");
    pn->addIncoming(tval, true_block);
    pn->addIncoming(fval, ffalse_block);
    return pn;
}

Value* conditional_expression::make_lvalue()
{
    if (oe)
        return oe->make_lvalue();
    return nullptr;
}

Value* conditional_expression::make_rvalue()
{
    if (oe)
        return oe->make_rvalue();

    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "cond-header", function);
    BasicBlock *true_block = BasicBlock::Create(context, "true", function);
    BasicBlock *false_block = BasicBlock::Create(context, "false", function);
    BasicBlock *end_block = BasicBlock::Create(context, "end", function);

    builder->CreateBr(header_block);

    builder->SetInsertPoint(header_block);
    Value *cond = truncate(expr1->make_rvalue());
    if (!cond)
        error::reject(op);
    builder->CreateCondBr(cond, true_block, false_block);

    builder->SetInsertPoint(true_block);
    Value *tval = expr2->make_rvalue();
    builder->CreateBr(end_block);

    builder->SetInsertPoint(false_block);
    Value *fval = expr3->make_rvalue();
    builder->CreateBr(end_block);

    if (!conditional_adjust(tval, fval, true_block, false_block))
        error::reject(op);

    builder->SetInsertPoint(end_block);
    if (tval->getType()->isVoidTy())
        return tval;

    PHINode *pn = builder->CreatePHI(tval->getType(), 2, "phi");
    pn->addIncoming(tval, true_block);
    pn->addIncoming(fval, false_block);
    return pn;
}

Value* assignment_expression::make_lvalue()
{
    if (op.type == INVALID)
        return lhs->make_lvalue();
    return nullptr;
}

Value* assignment_expression::make_rvalue()
{
    if (op.type == INVALID)
        return lhs->make_rvalue();

    Value* l = lhs->make_lvalue(); // conditional_expression
    if (!l)
        error::reject(op);

    Value* r = rhs->make_rvalue(); // assignment_expression
    if (op.str == "=")
    {
        if (!store(r, l))
            error::reject(op);
        return r;
    }
    Value *lv = builder->CreateLoad(l);
    if (op.str == "*=")
    {
        Value *v = create_mul(lv, r);
        if (!v)
            error::reject(op);
        store(v, l);
        return v;
    }
    if (op.str == "/=")
    {
        Value *v = create_div(lv, r);
        if (!v)
            error::reject(op);
        store(v, l);
        return v;
    }
    if (op.str == "%=")
    {
        Value *v = create_rem(lv, r);
        if (!v)
            error::reject(op);
        store(v, l);
        return v;
    }
    if (op.str == "+=")
    {
        Value *v = create_add(lv, r);
        if (!v)
            error::reject(op);
        store(v, l);
        return v;
    }
    if (op.str == "-=")
    {
        Value *v = create_sub(lv, r);
        if (!v)
            error::reject(op);
        store(v, l);
        return v;
    }
    if (op.str == "<<=")
    {
        Value *v = builder->CreateShl(lv, r);
        store(v, l);
        return v;
    }
    if (op.str == ">>=")
    {
        Value *v = builder->CreateAShr(lv, r);
        store(v, l);
        return v;
    }
    if (op.str == "&=")
    {
        Value *v = builder->CreateAnd(lv, r);
        store(v, l);
        return v;
    }
    if (op.str == "^=")
    {
        Value *v = builder->CreateXor(lv, r);
        store(v, l);
        return v;
    }
    if (op.str == "|=")
    {
        Value *v = builder->CreateOr(lv, r);
        store(v, l);
        return v;
    }
    error::reject(op);
}

Value* constant_expression::make_rvalue()
{
    return ce->make_rvalue();
}

Value* constant_expression::make_lvalue()
{
    return ce->make_lvalue();
}

Value* expression::make_rvalue()
{
    Value* last = nullptr;
    for (assignment_expression* a : ae)
        last = a->make_rvalue();
    return last;
}

Value* expression::make_lvalue()
{
    for (int i = 0; i < ae.size() - 1; ++i)
        ae[i]->make_rvalue();

    if (!ae.empty())
        return ae.back()->make_lvalue();
    return nullptr;
}

void goto_label::codegen()
{
    builder->CreateBr(block);
    builder->SetInsertPoint(block);
    stat->codegen();
}

void case_label::codegen()
{
}

void default_label::codegen()
{
}

void expression_statement::codegen()
{
    if (expr)
        expr->make_rvalue();
}

void if_statement::codegen()
{
    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "if-header", function);
    BasicBlock *then_block = BasicBlock::Create(context, "then", function);
    BasicBlock *else_block = BasicBlock::Create(context, "else", function);
    BasicBlock *end_block = BasicBlock::Create(context, "end", function);

    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = truncate(expr->make_rvalue());
    if (!cond)
        error::reject(op);
    builder->CreateCondBr(cond, then_block, else_block);

    builder->SetInsertPoint(then_block);
    stat->codegen();
    builder->CreateBr(end_block);

    builder->SetInsertPoint(else_block);
    if (estat)
        estat->codegen();
    builder->CreateBr(end_block);

    builder->SetInsertPoint(end_block);
}

void switch_statement::codegen()
{
}

void while_statement::codegen()
{
    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "while-header", function);
    BasicBlock *body_block = BasicBlock::Create(context, "while-body", function);
    BasicBlock *end_block = BasicBlock::Create(context, "while-end", function);

    continue_block = header_block;
    break_block = end_block;

    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = truncate(expr->make_rvalue());
    if (!cond)
        error::reject(op);
    builder->CreateCondBr(cond, body_block, end_block);

    builder->SetInsertPoint(body_block);
    stat->codegen();
    builder->CreateBr(header_block);

    builder->SetInsertPoint(end_block);

    continue_block = break_block = nullptr;
}

void do_while_statement::codegen()
{
    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "do-while-header", function);
    BasicBlock *check_block = BasicBlock::Create(context, "do-while-check", function);
    BasicBlock *end_block = BasicBlock::Create(context, "do-while-end", function);

    continue_block = check_block;
    break_block = end_block;

    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    stat->codegen();
    builder->CreateBr(check_block);
    builder->SetInsertPoint(check_block);
    Value *cond = truncate(expr->make_rvalue());
    if (!cond)
        error::reject(op);
    builder->CreateCondBr(cond, header_block, end_block);

    builder->SetInsertPoint(end_block);

    continue_block = break_block = nullptr;
}

void for_statement::codegen()
{
    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "for-header", function);
    BasicBlock *check_block = BasicBlock::Create(context, "for-check", function);
    BasicBlock *body_block = BasicBlock::Create(context, "for-body", function);
    BasicBlock *end_block = BasicBlock::Create(context, "for-end", function);

    continue_block = check_block;
    break_block = end_block;

    builder->CreateBr(header_block);

    builder->SetInsertPoint(header_block);
    if (expr1) expr1->make_rvalue();
    builder->CreateBr(check_block);

    builder->SetInsertPoint(check_block);
    Value *cond;
    if (expr2)
    {
        cond = truncate(expr2->make_rvalue());
        if (!cond)
            error::reject(op);
    }
    else
        cond = builder->getInt1(1);

    builder->CreateCondBr(cond, body_block, end_block);

    builder->SetInsertPoint(body_block);
    stat->codegen();
    if (expr3) expr3->make_rvalue();
    builder->CreateBr(check_block);

    builder->SetInsertPoint(end_block);

    continue_block = break_block = nullptr;
}

void goto_statement::codegen()
{
    builder->CreateBr(gl->block);
}

void break_statement::codegen()
{
    builder->CreateBr(break_block);
}

void continue_statement::codegen()
{
    builder->CreateBr(continue_block);
}

void return_statement::codegen()
{
    Function *function = builder->GetInsertBlock()->getParent();

    if (expr)
    {
        Value *val = expr->make_rvalue();
        val = cast(val, function->getReturnType());
        if (!val)
            error::reject(nxt);
        builder->CreateRet(val);
    }
    else
    {
        builder->CreateRetVoid();
    }

    BasicBlock *dead_block = BasicBlock::Create(
        context,
        "DEAD_BLOCK",
        function,
        0);

    builder->SetInsertPoint(dead_block);
}

void declaration_item::codegen()
{
    decl->codegen();
}

void statement_item::codegen()
{
    stat->codegen();
}

void compound_statement::codegen()
{
    if (sc) scopes.push_back(sc);
    for (block_item* b : bi)
        b->codegen();
    if (sc) scopes.pop_back();
}

void function_definition::codegen()
{
    scopes.push_back(sc);

    function_object *fo = find_function(get_identifier().str);
    if (!fo->function)
    {
        fo->function = Function::Create(
            fo->type,
            GlobalValue::ExternalLinkage,
            get_identifier().str,
            *module);
    }

    BasicBlock *entry_block = BasicBlock::Create(
        context,
        "entry",
        fo->function,
        0);

    builder->SetInsertPoint(entry_block);
    alloca_builder->SetInsertPoint(entry_block);

    Function::arg_iterator arg_iter = fo->function->arg_begin();
    declarator* decl = dec->unparenthesize();
    function_declarator* fdecl = dynamic_cast<function_declarator*>(decl->dd);
    for (parameter_declaration* pard : fdecl->pl)
    {
        if (pard && pard->decl)
        {
            // todo: decl should be object with storage not function declaration
            Value *val = pard->decl->codegen();
            store(arg_iter, val);
            arg_iter++;
        }
    }

    for (auto& [id, lab] : labels)
        lab->block = BasicBlock::Create(context, id, fo->function);

    cs->codegen();

    if (!builder->GetInsertBlock()->getTerminator())
    {
        Type *ret_type = builder->getCurrentFunctionReturnType();
        if (ret_type->isVoidTy())
            builder->CreateRetVoid();
        else
            builder->CreateRet(Constant::getNullValue(ret_type));
    }

    // todo dead return
    verifyFunction(*fo->function);

    scopes.pop_back();
}

void external_declaration::codegen()
{
    if (fd)
        fd->codegen();
    else
        decl->codegen();
}

void translation_unit::codegen(const char* filename)
{
    module = make_unique<Module>(filename, context);
    builder = make_unique<IRBuilder<>>(context);
    alloca_builder = make_unique<IRBuilder<>>(context);

    scopes.push_back(sc);
    for (external_declaration* d : ed)
        d->codegen();
    scopes.pop_back();

    verifyModule(*module);
}
