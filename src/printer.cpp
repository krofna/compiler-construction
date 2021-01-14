#include "parser.h"
#include <iostream>

struct printer
{
    int depth = 0;
    vector<char> buffer;

    ~printer()
    {
        flush(cout);
    }

    void indent()
    {
        for (int i = 0; i < depth; ++i)
            buffer.push_back('\t');
    }

    void unindent()
    {
        for (int i = 0; i < depth; ++i)
            buffer.pop_back();
    }

    printer& operator << (const string& x)
    {
        for (char c : x)
            buffer.push_back(c);
        return *this;
    }

    void flush(ostream& out)
    {
        for (char c : buffer)
            out << c;

        buffer = vector<char>();
    }
} pout;

void type_qualifier::print()
{
    pout << tok.str;
}

void builtin_type_specifier::print()
{
    pout << tok.str;
}

void struct_declaration::print()
{
    ts->print();
    pout << " ";
    bool flg = false;
    for (declarator* d : ds)
    {
        if (flg)
            pout << ", ";
        flg = true;
        d->print();
    }
    pout << ";";
}

void struct_or_union_specifier::print()
{
    pout << sou.str;
    if (id.type != INVALID)
        pout << " " << id.str;

    if (has_sds)
    {
        pout << "\n";
        pout.indent();
        pout << "{\n";
        pout.depth++;
        for (struct_declaration* sd : sds)
        {
            pout.indent();
            sd->print();
            pout << "\n";
        }
        pout.depth--;
        pout.indent();
        pout << "}";
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
    pout << tok.str;
}

void function_specifier::print()
{
    pout << tok.str;
}

void direct_declarator::print()
{
    pout << tok.str;
}

void parenthesized_declarator::print()
{
    pout << "(";
    decl->print();
    pout << ")";
}

void parameter_declaration::print()
{
    ds->print();
    if (decl)
    {
        pout << " ";
        decl->print();
    }
    if (ad)
        ad->print();
}

void function_declarator::print()
{
    dd->print();
    pout << "(";
    bool flg = false;
    for (parameter_declaration* pd : pl)
    {
        if (flg)
            pout << ", ";
        flg = true;
        pd->print();
    }
    pout << ")";
}

void pointer::print()
{
    pout << "*";
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
    bool flg = false;
    for (declarator* d : d)
    {
        if (flg)
            pout << ", ";
        else
            pout << " ";
        flg = true;
        d->print();
    }
    pout << ";";
}

void primary_expression::print()
{
    pout << tok.str;
}

void parenthesized_expression::print()
{
    pout << "(";
    expr->print();
    pout << ")";
}

void postfix_expression::print()
{
    pe->print();
}

void subscript_expression::print()
{
    pfe->print();
    pout << "[";
    expr->print();
    pout << "]";
}

void call_expression::print()
{
    pfe->print();
    pout << "(";
    bool flg = false;
    for (assignment_expression* ae : args)
    {
        if (flg)
            pout << ", ";
        flg = true;
        ae->print();
    }
    pout << ")";
}

void dot_expression::print()
{
    pfe->print();
    pout << "." << id.str;
}

void arrow_expression::print()
{
    pfe->print();
    pout << "->" << id.str;
}

void postfix_increment_expression::print()
{
    pfe->print();
    pout << "++";
}

void postfix_decrement_expression::print()
{
    pfe->print();
    pout << "--";
}

void unary_expression::print()
{
    pe->print();
}

void prefix_increment_expression::print()
{
    pout << "++";
    ue->print();
}

void prefix_decrement_expression::print()
{
    pout << "--";
    ue->print();
}

void unary_and_expression::print()
{
    pout << "&";
    ce->print();
}

void unary_star_expression::print()
{
    pout << "*";
    ce->print();
}

void unary_plus_expression::print()
{
    pout << "+";
    ce->print();
}

void unary_minus_expression::print()
{
    pout << "-";
    ce->print();
}

void unary_tilde_expression::print()
{
    pout << "~";
    ce->print();
}

void unary_not_expression::print()
{
    pout << "!";
    ce->print();
}

void sizeof_expression::print()
{
    pout << "sizeof ";
    ue->print();
}

void sizeof_type_expression::print()
{
    pout << "sizeof(";
    tn->print();
    pout << ")";
}

void cast_expression::print()
{
    if (ue)
        ue->print();

    if (tn)
    {
        pout << "(";
        tn->print();
        pout << ")";
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
    pout << " * ";
    rhs->print();
}

void div_expression::print()
{
    lhs->print();
    pout << " / ";
    rhs->print();
}

void mod_expression::print()
{
    lhs->print();
    pout << " % ";
    rhs->print();
}

void additive_expression::print()
{
    me->print();
}

void add_expression::print()
{
    lhs->print();
    pout << " + ";
    rhs->print();
}

void sub_expression::print()
{
    lhs->print();
    pout << " - ";
    rhs->print();
}

void shift_expression::print()
{
    ae->print();
}

void rshift_expression::print()
{
    lhs->print();
    pout << " >> ";
    rhs->print();
}

void lshift_expression::print()
{
    lhs->print();
    pout << " << ";
    rhs->print();
}

void relational_expression::print()
{
    se->print();
}

void less_expression::print()
{
    lhs->print();
    pout << " < ";
    rhs->print();
}

void greater_expression::print()
{
    lhs->print();
    pout << " > ";
    rhs->print();
}

void less_equal_expression::print()
{
    lhs->print();
    pout << " <= ";
    rhs->print();
}

void greater_equal_expression::print()
{
    lhs->print();
    pout << " >= ";
    rhs->print();
}

void equality_expression::print()
{
    re->print();
}

void equal_expression::print()
{
    lhs->print();
    pout << " == ";
    rhs->print();
}

void not_equal_expression::print()
{
    lhs->print();
    pout << " != ";
    rhs->print();
}

void and_expression::print()
{
    if (ee)
        ee->print();
    else
    {
        lhs->print();
        pout << " & ";
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
        pout << " ^ ";
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
        pout << " | ";
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
        pout << " && ";
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
        pout << " || ";
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
        pout << " ? ";
        expr2->print();
        pout << " : ";
        expr3->print();
    }
}

void assignment_expression::print()
{
    lhs->print();
    if (rhs)
    {
        pout << " " << op.str << " ";
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
            pout << ", ";
        flg = true;
        ae->print();
    }
}

void goto_label::print()
{
    pout.unindent();
    pout << id.str << ":\n";
    pout.indent();
    stat->print();
}

void case_label::print()
{
    pout << "case ";
    ce->print();
    pout << ":";
    stat->print();
}

void default_label::print()
{
    pout << "default:";
    stat->print();
}

void expression_statement::print()
{
    if (expr)
        expr->print();
    pout << ";";
}

static bool selection_helper(statement* stat, bool el = false)
{
    bool no_block = dynamic_cast<compound_statement*>(stat) == nullptr;
    if (el) no_block = no_block && dynamic_cast<if_statement*>(stat) == nullptr;

    if (no_block)
    {
        pout << "\n";
        pout.depth++;
        pout.indent();
    }
    else
        pout << " ";

    stat->print();
    if (no_block)
        pout.depth--;

    return no_block;
}

void if_statement::print()
{
    pout << "if (";
    expr->print();
    pout << ")";
    bool no_block = selection_helper(stat);
    if (estat)
    {
        if (no_block)
        {
            pout << "\n";
            pout.indent();
        }

        if (!no_block)
            pout << " ";

        pout << "else";
        selection_helper(estat, true);
    }
}

void switch_statement::print()
{
    pout << "switch (";
    expr->print();
    pout << ")";
    stat->print();
}

void while_statement::print()
{
    pout << "while (";
    expr->print();
    pout << ")";
    selection_helper(stat);
}

void do_while_statement::print()
{
    pout << "do";
    stat->print();
    pout << "while";
    expr->print();
}

void for_statement::print()
{
    pout << "for (";
    expr1->print();
    pout << ";";
    expr2->print();
    pout << ";";
    expr3->print();
    pout << ")";
    selection_helper(stat);
}

void goto_statement::print()
{
    pout << "goto " << id.str << ";";
}

void continue_statement::print()
{
    pout << "continue;";
}

void break_statement::print()
{
    pout << "break;";
}

void return_statement::print()
{
    pout << "return ";
    expr->print();
    pout << ";";
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
    pout << "{\n";
    pout.depth++;
    for (block_item* i : bi)
    {
        pout.indent();
        i->print();
        pout << "\n";
    }
    pout.depth--;
    pout.indent();
    pout << "}";
}

void function_definition::print()
{
    ds->print();
    pout << " ";
    dec->print();
    pout << "\n";
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
    bool flg = false;
    for (external_declaration* ed : ed)
    {
        if (flg)
            pout << "\n";
        flg = true;
        ed->print();
        pout << "\n";
    }
}
