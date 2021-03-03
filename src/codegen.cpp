#include "ast.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

LLVMContext context;
static unique_ptr<Module> module;
static unique_ptr<IRBuilder<>> builder, alloca_builder;
static BasicBlock *continue_block = nullptr;
static BasicBlock *break_block = nullptr;

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

Value* declarator::codegen()
{
    // todo: function pointer
    string identifier = get_identifier().str;
    if (dd->is_identifier() || dd->is_definition())
    {
        variable_object* vo = find_variable(identifier);
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

Value* declaration::codegen()
{
    for (declarator* de : d)
        de->codegen();
}

Value* primary_expression::make_lvalue()
{
    if (tok.type == IDENTIFIER)
    {
        // todo: može biti i poziv ili globalna
        return find_variable(tok.str)->store;
    }
    error::reject(tok);
}

Value* primary_expression::make_rvalue()
{
    if (tok.type == IDENTIFIER)
    {
        if (variable_object* vo = find_variable(tok.str))
            return builder->CreateLoad(vo->store);
        if (function_object* fo = find_function(tok.str))
            return fo->function;
        assert(false);
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
        return builder->CreateGlobalStringPtr(StringRef(tok.str.c_str() + 1, tok.str.size() - 2));
    }
    else
    {
        // error
    }
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
    error::reject();
}

Value* subscript_expression::make_rvalue()
{
    Value *ptr = make_lvalue();
    return builder->CreateLoad(ptr);
}

Value* subscript_expression::make_lvalue()
{
    Value *l = pfe->make_rvalue();
    vector<Value*> indices;
    indices.push_back(expr->make_rvalue());
    return builder->CreateInBoundsGEP(l, indices);
}

Value* call_expression::make_rvalue()
{
    Value* lhs = pfe->make_rvalue();
    Function *function = (Function*)lhs;
    FunctionType *ftype = function->getFunctionType();

    if (ftype->params().size() != args.size())
    {
        if (ftype->params().size() > args.size())
            error::reject();
        if (!ftype->isVarArg())
            error::reject();
    }

    vector<Value*> cargs;
    for (int i = 0; i < args.size(); ++i)
        cargs.push_back(args[i]->make_rvalue());

    for (int i = 0; i < ftype->params().size(); ++i)
        if (cargs[i]->getType() != ftype->params()[i])
            error::reject();

    return builder->CreateCall(function, cargs);
}

// TODO: forbid this nonsense
Value* call_expression::make_lvalue()
{
    Value *val = make_rvalue();
    Value *alloca = create_alloca(val->getType(), "tmp");
    builder->CreateStore(val, alloca);
    return alloca;
}

Value* dot_expression::make_rvalue()
{
    Value *ptr = make_lvalue();
    return builder->CreateLoad(ptr);
}

Value* dot_expression::make_lvalue()
{
    Value *l = pfe->make_lvalue();
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
    Value *oval = builder->CreateLoad(addr);
    Value *nval = builder->CreateAdd(oval, builder->getInt32(1));
    builder->CreateStore(nval, addr);
    return oval;
}

Value* postfix_increment_expression::make_lvalue()
{
    error::reject(op);
}

Value* postfix_decrement_expression::make_rvalue()
{
    Value *addr = pfe->make_lvalue();
    Value *oval = builder->CreateLoad(addr);
    Value *nval = builder->CreateSub(oval, builder->getInt32(1));
    builder->CreateStore(nval, addr);
    return oval;
}

Value* postfix_decrement_expression::make_lvalue()
{
    error::reject(op);
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
    Value *oval = builder->CreateLoad(addr);
    Value *nval = builder->CreateAdd(oval, builder->getInt32(1));
    builder->CreateStore(nval, addr);
    return builder->CreateLoad(addr);
}

Value* prefix_increment_expression::make_lvalue()
{
    error::reject();
}

Value* prefix_decrement_expression::make_rvalue()
{
    Value *addr = ue->make_lvalue();
    Value *oval = builder->CreateLoad(addr);
    Value *nval = builder->CreateSub(oval, builder->getInt32(1));
    builder->CreateStore(nval, addr);
    return builder->CreateLoad(addr);
}

Value* prefix_decrement_expression::make_lvalue()
{
    error::reject();
}

Value* unary_and_expression::make_rvalue()
{
    return ce->make_lvalue();
}

Value* unary_and_expression::make_lvalue()
{
    error::reject();
}

Value* unary_star_expression::make_rvalue()
{
    Value *addr = ce->make_rvalue();
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
    error::reject();
}

Value* unary_minus_expression::make_rvalue()
{
    Value* r = ce->make_rvalue();
    return builder->CreateSub(builder->getInt32(0), r);
}

Value* unary_minus_expression::make_lvalue()
{
    error::reject();
}

Value* unary_tilde_expression::make_rvalue()
{
    Value* r = ce->make_rvalue();
    return builder->CreateXor(builder->getInt32(-1), r);
}

Value* unary_tilde_expression::make_lvalue()
{
    error::reject();
}

Value* unary_not_expression::make_rvalue()
{
    Value* r = ce->make_rvalue();
    return builder->CreateXor(builder->getInt1(1), r);
}

Value* unary_not_expression::make_lvalue()
{
    error::reject();
}

Value* sizeof_expression::make_rvalue()
{
}

Value* sizeof_expression::make_lvalue()
{
    error::reject();
}

Value* sizeof_type_expression::make_rvalue()
{
}

Value* sizeof_type_expression::make_lvalue()
{
    error::reject();
}

Value* cast_expression::make_rvalue()
{
    return ue->make_rvalue();
}

Value* cast_expression::make_lvalue()
{
    return ue->make_lvalue();
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
    return builder->CreateMul(l, r);
}

Value* mul_expression::make_lvalue()
{
    error::reject();
}

Value* div_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateSDiv(l, r);
}

Value* div_expression::make_lvalue()
{
    error::reject();
}

Value* mod_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateSRem(l, r);
}

Value* mod_expression::make_lvalue()
{
    error::reject();
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
    return builder->CreateAdd(l, r);
}

Value* add_expression::make_lvalue()
{
    error::reject();
}

Value* sub_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateSub(l, r);
}

Value* sub_expression::make_lvalue()
{
    error::reject();
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
    error::reject();
}

Value* lshift_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateShl(l, r);
}

Value* lshift_expression::make_lvalue()
{
    error::reject();
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
    return builder->CreateICmpSLT(l, r);
}

Value* less_expression::make_lvalue()
{
    error::reject();
}

Value* greater_expression::make_rvalue()
{
    Value *l = lhs->make_rvalue();
    Value *r = rhs->make_rvalue();
    return builder->CreateICmpSGT(l, r);
}

Value* greater_expression::make_lvalue()
{
    error::reject();
}

Value* less_equal_expression::make_rvalue()
{
    Value *l = lhs->make_rvalue();
    Value *r = rhs->make_rvalue();
    return builder->CreateICmpSLE(l, r);
}

Value* less_equal_expression::make_lvalue()
{
    error::reject();
}

Value* greater_equal_expression::make_rvalue()
{
    Value *l = lhs->make_rvalue();
    Value *r = rhs->make_rvalue();
    return builder->CreateICmpSGE(l, r);
}

Value* greater_equal_expression::make_lvalue()
{
    error::reject();
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
    return builder->CreateICmpEQ(l, r);
}

Value* equal_expression::make_lvalue()
{
    error::reject();
}

Value* not_equal_expression::make_rvalue()
{
    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateICmpNE(l, r);
}

Value* not_equal_expression::make_lvalue()
{
    error::reject();
}

Value* and_expression::make_rvalue()
{
    if (ee)
        return ee->make_rvalue();

    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateAnd(l, r);
}

Value* and_expression::make_lvalue()
{
    if (ee)
        return ee->make_lvalue();
    error::reject();
}

Value* exclusive_or_expression::make_rvalue()
{
    if (ae)
        return ae->make_rvalue();

    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateXor(l, r);
}

Value* exclusive_or_expression::make_lvalue()
{
    if (ae)
        return ae->make_lvalue();
    error::reject();
}

Value* inclusive_or_expression::make_rvalue()
{
    if (xe)
        return xe->make_rvalue();

    Value* l = lhs->make_rvalue();
    Value* r = rhs->make_rvalue();
    return builder->CreateOr(l, r);
}

Value* inclusive_or_expression::make_lvalue()
{
    if (xe)
        return xe->make_lvalue();
    error::reject();
}

Value* logical_and_expression::make_lvalue()
{
    if (oe)
        return oe->make_lvalue();
    error::reject();
}

Value* logical_and_expression::make_rvalue()
{
    if (oe)
        return oe->make_rvalue();

    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "and-header", function);
    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = lhs->make_rvalue();

    BasicBlock *true_block = BasicBlock::Create(context, "true", function);
    BasicBlock *false_block = BasicBlock::Create(context, "false", function);
    BasicBlock *ttrue_block = BasicBlock::Create(context, "ttrue", function);
    BasicBlock *merge_block = BasicBlock::Create(context, "merge", function);

    builder->CreateCondBr(cond, true_block, false_block);

    builder->SetInsertPoint(true_block);
    Value *tcond = rhs->make_rvalue();
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
    error::reject();
}

Value* logical_or_expression::make_rvalue()
{
    if (ae)
        return ae->make_rvalue();

    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "or-header", function);
    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = lhs->make_rvalue();

    BasicBlock *true_block = BasicBlock::Create(context, "true", function);
    BasicBlock *false_block = BasicBlock::Create(context, "false", function);
    BasicBlock *ffalse_block = BasicBlock::Create(context, "ffalse", function);
    BasicBlock *merge_block = BasicBlock::Create(context, "merge", function);

    builder->CreateCondBr(cond, true_block, false_block);

    builder->SetInsertPoint(false_block);
    Value *fcond = rhs->make_rvalue();
    builder->CreateCondBr(fcond, true_block, ffalse_block);

    builder->SetInsertPoint(true_block);
    Value *tval = builder->getInt1(1);
    builder->CreateBr(merge_block);
    builder->SetInsertPoint(ffalse_block);
    Value *fval = builder->getInt1(0);
    builder->CreateBr(merge_block);
    builder->SetInsertPoint(merge_block);

    PHINode *pn = builder->CreatePHI(Type::getInt1Ty(context), 2, "phi");
    pn->addIncoming(tval, ffalse_block);
    pn->addIncoming(fval, true_block);
    return pn;
}

Value* conditional_expression::make_lvalue()
{
    if (oe)
        return oe->make_lvalue();
    error::reject();
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
    Value *cond = expr1->make_rvalue();
    builder->CreateCondBr(cond, true_block, false_block);

    builder->SetInsertPoint(true_block);
    Value *tval = expr2->make_rvalue();
    builder->CreateBr(end_block);

    builder->SetInsertPoint(false_block);
    Value *fval = expr3->make_rvalue();
    builder->CreateBr(end_block);

    builder->SetInsertPoint(end_block);
    PHINode *pn = builder->CreatePHI(Type::getInt32Ty(context), 2, "phi");
    pn->addIncoming(tval, true_block);
    pn->addIncoming(fval, false_block);
    return pn;
}

Value* assignment_expression::make_lvalue()
{
    if (op.type == INVALID)
        return lhs->make_lvalue();
    error::reject();
}

Value* assignment_expression::make_rvalue()
{
    if (op.type == INVALID)
        return lhs->make_rvalue();

    Value* l = lhs->make_lvalue(); // conditional_expression
    Value* r = rhs->make_rvalue(); // assignment_expression
    if (op.str == "=")
    {
        builder->CreateStore(r, l);
        return r;
    }
    Value *lv = builder->CreateLoad(l);
    if (op.str == "*=")
    {
        Value *v = builder->CreateMul(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "/=")
    {
        Value *v = builder->CreateSDiv(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "%=")
    {
        Value *v = builder->CreateSRem(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "+=")
    {
        Value *v = builder->CreateAdd(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "-=")
    {
        Value *v = builder->CreateSub(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "<<=")
    {
        Value *v = builder->CreateShl(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == ">>=")
    {
        Value *v = builder->CreateAShr(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "&=")
    {
        Value *v = builder->CreateAnd(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "^=")
    {
        Value *v = builder->CreateXor(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "|=")
    {
        Value *v = builder->CreateOr(lv, r);
        builder->CreateStore(v, l);
        return v;
    }
    error::reject();
}

Value* constant_expression::make_rvalue()
{
    return ce->make_rvalue();
}

Value* constant_expression::make_lvalue()
{
    error::reject();
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
    error::reject();
}

Value* goto_label::codegen()
{
    builder->CreateBr(block);
    builder->SetInsertPoint(block);
    stat->codegen();
}

Value* case_label::codegen()
{
}

Value* default_label::codegen()
{
}

Value* expression_statement::codegen()
{
    if (expr)
        return expr->make_rvalue();
    return nullptr;
}

Value* if_statement::codegen()
{
    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "if-header", function);
    BasicBlock *then_block = BasicBlock::Create(context, "then", function);
    BasicBlock *else_block = BasicBlock::Create(context, "else", function);
    BasicBlock *end_block = BasicBlock::Create(context, "end", function);

    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = expr->make_rvalue();
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

Value* switch_statement::codegen()
{
}

Value* while_statement::codegen()
{
    Function *function = builder->GetInsertBlock()->getParent();

    BasicBlock *header_block = BasicBlock::Create(context, "while-header", function);
    BasicBlock *body_block = BasicBlock::Create(context, "while-body", function);
    BasicBlock *end_block = BasicBlock::Create(context, "while-end", function);

    continue_block = header_block;
    break_block = end_block;

    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = expr->make_rvalue();
    builder->CreateCondBr(cond, body_block, end_block);

    builder->SetInsertPoint(body_block);
    stat->codegen();
    builder->CreateBr(header_block);

    builder->SetInsertPoint(end_block);

    continue_block = break_block = nullptr;
}

Value* do_while_statement::codegen()
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
    Value *cond = expr->make_rvalue();
    builder->CreateCondBr(cond, header_block, end_block);

    builder->SetInsertPoint(end_block);

    continue_block = break_block = nullptr;
}

Value* for_statement::codegen()
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
    expr1->make_rvalue();
    builder->CreateBr(check_block);

    builder->SetInsertPoint(check_block);
    Value *cond = expr2->make_rvalue();
    builder->CreateCondBr(cond, body_block, end_block);

    builder->SetInsertPoint(body_block);
    stat->codegen();
    expr3->make_rvalue();
    builder->CreateBr(check_block);

    builder->SetInsertPoint(end_block);

    continue_block = break_block = nullptr;
}

Value* goto_statement::codegen()
{
    builder->CreateBr(gl->block);
}

Value* break_statement::codegen()
{
    builder->CreateBr(break_block);
}

Value* continue_statement::codegen()
{
    builder->CreateBr(continue_block);
}

Value* return_statement::codegen()
{
    if (expr)
    {
        Value *val = expr->make_rvalue();
        builder->CreateRet(val);
    }
    else
    {
        builder->CreateRetVoid();
    }

    Function *function = builder->GetInsertBlock()->getParent();
    BasicBlock *dead_block = BasicBlock::Create(
        context,
        "DEAD_BLOCK",
        function,
        0);

    builder->SetInsertPoint(dead_block);
}

Value* declaration_item::codegen()
{
    return decl->codegen();
}

Value* statement_item::codegen()
{
    return stat->codegen();
}

Value* compound_statement::codegen()
{
    if (sc) scopes.push_back(sc);
    for (block_item* b : bi)
        b->codegen();
    if (sc) scopes.pop_back();
}

Value* function_definition::codegen()
{
    scopes.push_back(sc);

    function_object *fo = find_function(get_identifier().str);

    fo->function = Function::Create(
        fo->type,
        GlobalValue::ExternalLinkage,
        get_identifier().str,
        *module);

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
            builder->CreateStore(arg_iter, val);
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
    return fo->function;
}

Value* external_declaration::codegen()
{
    if (fd)
        return fd->codegen();
    return decl->codegen();
}

Value* translation_unit::codegen(const char* filename)
{
    module = make_unique<Module>(filename, context);
    builder = make_unique<IRBuilder<>>(context);
    alloca_builder = make_unique<IRBuilder<>>(context);

    scopes.push_back(sc);
    for (external_declaration* d : ed)
        d->codegen();
    scopes.pop_back();

    verifyModule(*module);

    string fn = filename;
    size_t pos = fn.find('/');
    if (pos != fn.npos)
        fn = fn.substr(pos + 1);
    pos = fn.find('.');
    if (pos != fn.npos)
        fn = fn.substr(0, pos);
    fn += ".ll";

    error_code EC;
    raw_fd_ostream stream(fn, EC, sys::fs::OpenFlags::F_Text);
    module->print(stream, nullptr);
}
