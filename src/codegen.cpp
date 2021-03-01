#include "ast.h"
#include "llvm/IR/Verifier.h"

static LLVMContext context;
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

Value* declaration::codegen()
{
    // todo: alloca samo za lokalne
    for (declarator* de : d)
    {
        string identifier = de->get_identifier().str;
        variable_object* vo = find_variable(identifier);
        vo->alloca = create_alloca(builder->getInt32Ty(), identifier);
    }
}

Value* primary_expression::make_lvalue()
{
    if (tok.type == IDENTIFIER)
    {
        // todo: može biti i poziv ili globalna
        return find_variable(tok.str)->alloca;
    }
    error::reject(tok);
}

Value* primary_expression::make_rvalue()
{
    if (tok.type == IDENTIFIER)
    {
        // todo: može biti i poziv ili globalna
        return builder->CreateLoad(find_variable(tok.str)->alloca);
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
        return ConstantDataArray::getString(context, tok.str, true);
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
}

Value* subscript_expression::make_lvalue()
{
}

Value* call_expression::make_rvalue()
{
}

Value* call_expression::make_lvalue()
{
    error::reject();
}

Value* dot_expression::make_rvalue()
{
}

Value* dot_expression::make_lvalue()
{
}

Value* arrow_expression::make_rvalue()
{
}

Value* arrow_expression::make_lvalue()
{
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
    error::reject();
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
    error::reject();
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
    if (op.str == "*=")
    {
        Value *v = builder->CreateMul(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "/=")
    {
        Value *v = builder->CreateSDiv(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "%=")
    {
        Value *v = builder->CreateSRem(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "+=")
    {
        Value *v = builder->CreateAdd(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "-=")
    {
        Value *v = builder->CreateSub(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "<<=")
    {
        Value *v = builder->CreateShl(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == ">>=")
    {
        Value *v = builder->CreateAShr(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "&=")
    {
        Value *v = builder->CreateAnd(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "^=")
    {
        Value *v = builder->CreateXor(l, r);
        builder->CreateStore(v, l);
        return v;
    }
    if (op.str == "|=")
    {
        Value *v = builder->CreateOr(l, r);
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
    // builder->CreateBr(goto_block);
    // builder->SetInsertPoint(goto_block);
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
    builder->CreateBr(header_block);
    builder->SetInsertPoint(header_block);
    Value *cond = expr->make_rvalue();

    BasicBlock *then_block = BasicBlock::Create(context, "then", function);
    BasicBlock *else_block = BasicBlock::Create(context, "else", function);
    BasicBlock *end_block = BasicBlock::Create(context, "end", function);

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
    // builder->CreateBr(goto_block);
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
    vector<Type *> argument_types;
    // todo: arg types

    FunctionType *func_type = FunctionType::get(
        builder->getInt32Ty(), argument_types, false);

    Function *function = Function::Create(
        func_type,
        GlobalValue::ExternalLinkage,
        get_identifier().str,
        *module);

    BasicBlock *entry_block = BasicBlock::Create(
        context,
        "entry",
        function,
        0);

    builder->SetInsertPoint(entry_block);
    alloca_builder->SetInsertPoint(entry_block);

    
    // BasicBlock *goto_block = BasicBlock::Create(context, "goto", function);

    // todo: pohrani parametre na stack

    cs->codegen();

    // todo: ovo samo za main
    builder->CreateRet(builder->getInt32(0));

    // todo dead return
    verifyFunction(*function);

    scopes.pop_back();
    return function;
}

Value* external_declaration::codegen()
{
    if (fd)
        return fd->codegen();
    return decl->codegen();
}

Value* translation_unit::codegen()
{
    module = make_unique<Module>("tomo-i-mislav.c", context);
    builder = make_unique<IRBuilder<>>(context);
    alloca_builder = make_unique<IRBuilder<>>(context);

    scopes.push_back(sc);
    for (external_declaration* d : ed)
        d->codegen();
    scopes.pop_back();

    verifyModule(*module);
    module->dump();
}
