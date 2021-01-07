#include "parser.h"
#include <iostream>

static int depth = 0;

static void indent()
{
    for (int i = 0; i < depth; ++i)
        cout << "\t";
}

void type_qualifier::print()
{
    cout << tok.str;
}

void builtin_type_specifier::print()
{
    cout << tok.str;
}

void struct_declaration::print()
{
    ts->print();
    cout << " ";
    bool flg = false;
    for (declarator* d : ds)
    {
        if (flg)
            cout << ",";
        flg = true;
        d->print();
    }
    cout << ";";
}

void struct_or_union_specifier::print()
{
    cout << sou.str;
    if (id.type != INVALID)
        cout << " " << id.str;

    if (has_sds)
    {
        cout << "\n";
        indent();
        cout << "{\n";
        depth++;
        for (struct_declaration* sd : sds)
        {
            indent();
            sd->print();
            cout << "\n";
        }
        depth--;
        indent();
        cout << "}";
    }
}

void direct_abstract_declarator::print()
{
    if (ad)
        ad->print();
    for (parameter_declaration* pd : pl)
        pd->print();
}

void abstract_declarator::print()
{
    if (p)
        p->print();
    if (dad)
        dad->print();
}

void type_name::print()
{
    for (specifier_qualifier* sq : sqs)
        sq->print();

    if (ad)
        ad->print();
}

void declaration_specifiers::print()
{
    ts->print();
}

void storage_class_specifier::print()
{
    cout << tok.str;
}

void function_specifier::print()
{
    cout << tok.str;
}

void direct_declarator::print()
{
    cout << tok.str;
}

void parenthesized_declarator::print()
{
    cout << "(";
    decl->print();
    cout << ")";
}

void parameter_declaration::print()
{
    ds->print();
    if (decl)
    {
        cout << " ";
        decl->print();
    }
    if (ad)
        ad->print();
}

void function_declarator::print()
{
    dd->print();
    cout << "(";
    bool flg = false;
    for (parameter_declaration* pd : pl)
    {
        if (flg)
            cout << ", ";
        flg = true;
        pd->print();
    }
    cout << ")";
}

void pointer::print()
{
    cout << "*";
    for (type_qualifier* tq : tql)
        tq->print();

    if (p)
        p->print();
}

void declarator::print()
{
    if (p)
        p->print();

    dd->print();
}

void declaration::print()
{
    ds->print();
    if (d)
    {
        cout << " ";
        d->print();
    }
    cout << ";";
}

void primary_expression::print()
{
    cout << tok.str;
}

void parenthesized_expression::print()
{
    cout << "(";
    expr->print();
    cout << ")";
}

void postfix_expression::print()
{
    pe->print();
}

void subscript_expression::print()
{
    pfe->print();
    cout << "[";
    expr->print();
    cout << "]";
}

void call_expression::print()
{
    pfe->print();
    cout << "(";
    bool flg = false;
    for (assignment_expression* ae : args)
    {
        if (flg)
            cout << ", ";
        flg = true;
        ae->print();
    }
    cout << ")";
}

void dot_expression::print()
{
    pfe->print();
    cout << "." << id.str;
}

void arrow_expression::print()
{
    pfe->print();
    cout << "->" << id.str;
}

void postfix_increment_expression::print()
{
    pfe->print();
    cout << "++";
}

void postfix_decrement_expression::print()
{
    pfe->print();
    cout << "--";
}

void unary_expression::print()
{
    pe->print();
}

void prefix_increment_expression::print()
{
    cout << "++";
    ue->print();
}

void prefix_decrement_expression::print()
{
    cout << "--";
    ue->print();
}

void unary_and_expression::print()
{
    cout << "&";
    ce->print();
}

void unary_star_expression::print()
{
    cout << "*";
    ce->print();
}

void unary_plus_expression::print()
{
    cout << "+";
    ce->print();
}

void unary_minus_expression::print()
{
    cout << "-";
    ce->print();
}

void unary_tilde_expression::print()
{
    cout << "~";
    ce->print();
}

void unary_not_expression::print()
{
    cout << "!";
    ce->print();
}

void sizeof_expression::print()
{
    cout << "sizeof ";
    ue->print();
}

void sizeof_type_expression::print()
{
    cout << "sizeof(";
    tn->print();
    cout << ")";
}

void cast_expression::print()
{
    if (ue)
        ue->print();

    if (tn)
    {
        cout << "(";
        tn->print();
        cout << ")";
        ce->print();
    }
}

void multiplicative_expression::print()
{
    ce->print();
}

void mul_expression::print()
{
    lhs->print();
    cout << " * ";
    rhs->print();
}

void div_expression::print()
{
    lhs->print();
    cout << " / ";
    rhs->print();
}

void mod_expression::print()
{
    lhs->print();
    cout << " % ";
    rhs->print();
}

void additive_expression::print()
{
    me->print();
}

void add_expression::print()
{
    lhs->print();
    cout << " + ";
    rhs->print();
}

void sub_expression::print()
{
    lhs->print();
    cout << " - ";
    rhs->print();
}

void shift_expression::print()
{
    ae->print();
}

void rshift_expression::print()
{
    lhs->print();
    cout << " >> ";
    rhs->print();
}

void lshift_expression::print()
{
    lhs->print();
    cout << " << ";
    rhs->print();
}

void relational_expression::print()
{
    se->print();
}

void less_expression::print()
{
    lhs->print();
    cout << " < ";
    rhs->print();
}

void greater_expression::print()
{
    lhs->print();
    cout << " > ";
    rhs->print();
}

void less_equal_expression::print()
{
    lhs->print();
    cout << " <= ";
    rhs->print();
}

void greater_equal_expression::print()
{
    lhs->print();
    cout << " >= ";
    rhs->print();
}

void equality_expression::print()
{
    re->print();
}

void equal_expression::print()
{
    lhs->print();
    cout << " == ";
    rhs->print();
}

void not_equal_expression::print()
{
    lhs->print();
    cout << " != ";
    rhs->print();
}

void and_expression::print()
{
    if (ee)
        ee->print();
    else
    {
        lhs->print();
        cout << " & ";
        rhs->print();
    }
}

void exclusive_or_expression::print()
{
    if (ae)
        ae->print();
    else
    {
        lhs->print();
        cout << " ^ ";
        rhs->print();
    }
}

void inclusive_or_expression::print()
{
    if (xe)
        xe->print();
    else
    {
        lhs->print();
        cout << " | ";
        rhs->print();
    }
}

void logical_and_expression::print()
{
    if (oe)
        oe->print();
    else
    {
        lhs->print();
        cout << " && ";
        rhs->print();
    }
}

void logical_or_expression::print()
{
    if (ae)
        ae->print();
    else
    {
        lhs->print();
        cout << " || ";
        rhs->print();
    }
}

void conditional_expression::print()
{
    if (oe)
        oe->print();
    else
    {
        expr1->print();
        cout << " ? ";
        expr2->print();
        cout << " : ";
        expr3->print();
    }
}

void assignment_expression::print()
{
    lhs->print();
    if (rhs)
    {
        cout << " " << op.str << " ";
        rhs->print();
    }
}

void constant_expression::print()
{
    ce->print();
}

void expression::print()
{
    bool flg = false;
    for (assignment_expression* ae : ae)
    {
        if (flg)
            cout << ",";
        flg = true;
        ae->print();
    }
}

void goto_label::print()
{
    cout << id.str << ":\n";
    stat->print();
}

void case_label::print()
{
    cout << "case ";
    ce->print();
    cout << ":";
    stat->print();
}

void default_label::print()
{
    cout << "default:";
    stat->print();
}

void expression_statement::print()
{
    expr->print();
    cout << ";";
}

void if_statement::print()
{
    cout << "if (";
    expr->print();
    cout << ") ";
    bool no_block = dynamic_cast<compound_statement*>(stat) == nullptr;
    if (no_block)
    {
        cout << "\n";
        depth++;
        indent();
        depth--;
    }
    stat->print();
    if (no_block)
    {
        cout << "\n";
        indent();
    }
    if (estat)
    {
        cout << "else ";
        estat->print();
    }
}

void switch_statement::print()
{
    cout << "switch (";
    expr->print();
    cout << ") ";
    stat->print();
}

void while_statement::print()
{
    cout << "while (";
    expr->print();
    cout << ") ";
    stat->print();
}

void do_while_statement::print()
{
    cout << "do";
    stat->print();
    cout << "while";
    expr->print();
}

void for_statement::print()
{
    cout << "for (";
    expr1->print();
    cout << ";";
    expr2->print();
    cout << ";";
    expr3->print();
    cout << ")";
    stat->print();
}

void goto_statement::print()
{
    cout << "goto " << id.str << ";";
}

void continue_statement::print()
{
    cout << "continue;";
}

void break_statement::print()
{
    cout << "break;";
}

void return_statement::print()
{
    cout << "return ";
    expr->print();
    cout << ";";
}

void declaration_item::print()
{
    decl->print();
}

void statement_item::print()
{
    stat->print();
}

void compound_statement::print()
{
    cout << "{\n";
    depth++;
    for (block_item* i : bi)
    {
        indent();
        i->print();
        cout << '\n';
    }
    depth--;
    indent();
    cout << "}";
}

void function_definition::print()
{
    ds->print();
    cout << " ";
    dec->print();
    cout << "\n";
    cs->print();
}

void external_declaration::print()
{
    if (fd)
        fd->print();
    if (decl)
        decl->print();
}

void translation_unit::print()
{
    for (external_declaration* ed : ed)
    {
        ed->print();
        cout << "\n\n";
    }
}
