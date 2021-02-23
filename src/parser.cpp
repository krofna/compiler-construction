#include "parser.h"
#include <set>

#include <iostream>
//#define dbg(x) cerr << #x << " = " << x << endl
#define dbg(x)

primary_expression* parser::parse_primary_expression()
{
    if (tokit->type == IDENTIFIER)
    {
        primary_expression* pe = new primary_expression;
        pe->var = accept(find_var((pe->tok = *tokit).str));
        tokit++;
        return pe;
    }
    if (tokit->type == CONSTANT)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = parse_token();
        return pe;
    }
    if (tokit->type == STRING_LITERAL)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = parse_token();
        return pe;
    }
    if (check("("))
    {
        parenthesized_expression* pe = new parenthesized_expression;
        pe->expr = accept(parse_expression());
        accepts(")");
        return pe;
    }
    return nullptr;
}

postfix_expression* parser::parse_postfix_expression()
{
    primary_expression* pe = parse_primary_expression();
    if (!pe)
        return nullptr;

    postfix_expression* e = new postfix_expression;
    e->pe = pe;

    while (true)
    {
        if (check("["))
        {
            subscript_expression* se = new subscript_expression;
            se->pfe = e;
            se->expr = accept(parse_expression());
            accepts("]");
            e = se;
        }
        else if (check("("))
        {
            call_expression* ce = new call_expression;
            ce->pfe = e;
            if (assignment_expression* ae = parse_assignment_expression())
            {
                ce->args.push_back(ae);
                while (check(","))
                    ce->args.push_back(accept(parse_assignment_expression()));
            }
            accepts(")");
            e = ce;
        }
        else if (check("."))
        {
            dot_expression* de = new dot_expression;
            de->pfe = e;
            de->id = parse_identifier();
            e = de;
        }
        else if (check("->"))
        {
            arrow_expression* ae = new arrow_expression;
            ae->pfe = e;
            ae->id = parse_identifier();
            e = ae;
        }
        else if (check("++"))
        {
            postfix_increment_expression* ie = new postfix_increment_expression;
            ie->pfe = e;
            e = ie;
        }
        else if (check("--"))
        {
            postfix_decrement_expression* de = new postfix_decrement_expression;
            de->pfe = e;
            e = de;
        }
        else
            break;
    }
    return e;
}

unary_expression* parser::parse_unary_expression()
{
    if (postfix_expression* pe = parse_postfix_expression())
    {
        unary_expression* ue = new unary_expression;
        ue->pe = pe;
        return ue;
    }
    if (check("++"))
    {
        prefix_increment_expression* ie = new prefix_increment_expression;
        ie->ue = accept(parse_unary_expression());
        return ie;
    }
    if (check("--"))
    {
        prefix_decrement_expression* de = new prefix_decrement_expression;
        de->ue = accept(parse_unary_expression());
        return de;
    }
    if (check("&"))
    {
        unary_and_expression* ae = new unary_and_expression;
        ae->ce = accept(parse_cast_expression());
        return ae;
    }
    if (check("*"))
    {
        unary_star_expression* se = new unary_star_expression;
        se->ce = accept(parse_cast_expression());
        return se;
    }
    if (check("+"))
    {
        unary_plus_expression* pe = new unary_plus_expression;
        pe->ce = accept(parse_cast_expression());
        return pe;
    }
    if (check("-"))
    {
        unary_minus_expression* me = new unary_minus_expression;
        me->ce = accept(parse_cast_expression());
        return me;
    }
    if (check("~"))
    {
        unary_tilde_expression* te = new unary_tilde_expression;
        te->ce = accept(parse_cast_expression());
        return te;
    }
    if (check("!"))
    {
        unary_not_expression* ne = new unary_not_expression;
        ne->ce = accept(parse_cast_expression());
        return ne;
    }
    if (check("sizeof"))
    {
        if (check("("))
        {
            if (type_name* tn = parse_type_name())
            {
                sizeof_type_expression* se = new sizeof_type_expression;
                se->tn = tn;
                accepts(")");
                return se;
            }
            tokit--;
        }
        if (unary_expression* ue = parse_unary_expression())
        {
            sizeof_expression* se = new sizeof_expression;
            se->ue = ue;
            return se;
        }
        reject();
    }
    return nullptr;
}

cast_expression* parser::parse_cast_expression()
{
    if (unary_expression* ue = parse_unary_expression())
    {
        cast_expression* ce = new cast_expression;
        ce->ue = ue;
        return ce;
    }
    if (check("("))
    {
        cast_expression* ce = new cast_expression;
        ce->tn = accept(parse_type_name());
        accepts(")");
        ce->ce = accept(parse_cast_expression());
        return ce;
    }
    return nullptr;
}

multiplicative_expression* parser::parse_multiplicative_expression()
{
    cast_expression* ce = parse_cast_expression();
    if (!ce)
        return nullptr;

    multiplicative_expression* lhs = new multiplicative_expression;
    lhs->ce = ce;

    while (true)
    {
        if (check("*"))
        {
            mul_expression* me = new mul_expression;
            me->lhs = lhs;
            me->rhs = accept(parse_cast_expression());
            lhs = me;
        }
        else if (check("/"))
        {
            div_expression* de = new div_expression;
            de->lhs = lhs;
            de->rhs = accept(parse_cast_expression());
            lhs = de;
        }
        else if (check("%"))
        {
            mod_expression* me = new mod_expression;
            me->lhs = lhs;
            me->rhs = accept(parse_cast_expression());
            lhs = me;
        }
        else
            break;
    }
    return lhs;
}

additive_expression* parser::parse_additive_expression()
{
    multiplicative_expression* me = parse_multiplicative_expression();
    if (!me)
        return nullptr;

    additive_expression* lhs = new additive_expression;
    lhs->me = me;

    while (true)
    {
        if (check("+"))
        {
            add_expression* ae = new add_expression;
            ae->lhs = lhs;
            ae->rhs = accept(parse_multiplicative_expression());
            lhs = ae;
        }
        else if (check("-"))
        {
            sub_expression* se = new sub_expression;
            se->lhs = lhs;
            se->rhs = accept(parse_multiplicative_expression());
            lhs = se;
        }
        else
            break;
    }
    return lhs;
}

shift_expression* parser::parse_shift_expression()
{
    additive_expression* ae = parse_additive_expression();
    if (!ae)
        return nullptr;

    shift_expression* lhs = new shift_expression;
    lhs->ae = ae;

    while (true)
    {
        if (check("<<"))
        {
            lshift_expression* ae = new lshift_expression;
            ae->lhs = lhs;
            ae->rhs = accept(parse_additive_expression());
            lhs = ae;
        }
        else if (check(">>"))
        {
            rshift_expression* ae = new rshift_expression;
            ae->lhs = lhs;
            ae->rhs = accept(parse_additive_expression());
            lhs = ae;
        }
        else
            break;
    }
    return lhs;
}

relational_expression* parser::parse_relational_expression()
{
    shift_expression* se = parse_shift_expression();
    if (!se)
        return nullptr;

    relational_expression* lhs = new relational_expression;
    lhs->se = se;

    while (true)
    {
        if (check("<"))
        {
            less_expression* le = new less_expression;
            le->lhs = lhs;
            le->rhs = accept(parse_shift_expression());
            lhs = le;
        }
        else if (check(">"))
        {
            greater_expression* ge = new greater_expression;
            ge->lhs = lhs;
            ge->rhs = accept(parse_shift_expression());
            lhs = ge;
        }
        else if (check("<="))
        {
            less_equal_expression* le = new less_equal_expression;
            le->lhs = lhs;
            le->rhs = accept(parse_shift_expression());
            lhs = le;
        }
        else if (check(">="))
        {
            greater_equal_expression* ge = new greater_equal_expression;
            ge->lhs = lhs;
            ge->rhs = accept(parse_shift_expression());
            lhs = ge;
        }
        else
            break;
    }
    return lhs;
}

equality_expression* parser::parse_equality_expression()
{
    relational_expression* re = parse_relational_expression();
    if (!re)
        return nullptr;

    equality_expression* lhs = new equality_expression;
    lhs->re = re;

    while (true)
    {
        if (check("=="))
        {
            equal_expression* ee = new equal_expression;
            ee->lhs = lhs;
            ee->rhs = accept(parse_relational_expression());
            lhs = ee;
        }
        if (check("!="))
        {
            not_equal_expression* ne = new not_equal_expression;
            ne->lhs = lhs;
            ne->rhs = accept(parse_relational_expression());
            lhs = ne;
        }
        else
            break;
    }
    return lhs;
}

and_expression* parser::parse_and_expression()
{
    equality_expression* ee = parse_equality_expression();
    if (!ee)
        return nullptr;

    and_expression* lhs = new and_expression;
    lhs->ee = ee;

    while (check("&"))
    {
        and_expression* ae = new and_expression;
        ae->lhs = lhs;
        ae->rhs = accept(parse_equality_expression());
        lhs = ae;
    }
    return lhs;
}

exclusive_or_expression* parser::parse_exclusive_or_expression()
{
    and_expression* ae = parse_and_expression();
    if (!ae)
        return nullptr;

    exclusive_or_expression* lhs = new exclusive_or_expression;
    lhs->ae = ae;

    while (check("^"))
    {
        exclusive_or_expression* xe = new exclusive_or_expression;
        xe->lhs = lhs;
        xe->rhs = accept(parse_and_expression());
        lhs = xe;
    }
    return lhs;
}

inclusive_or_expression* parser::parse_inclusive_or_expression()
{
    exclusive_or_expression* xe = parse_exclusive_or_expression();
    if (!xe)
        return nullptr;

    inclusive_or_expression* lhs = new inclusive_or_expression;
    lhs->xe = xe;

    while (check("|"))
    {
        inclusive_or_expression* oe = new inclusive_or_expression;
        oe->lhs = lhs;
        oe->rhs = accept(parse_exclusive_or_expression());
        lhs = oe;
    }
    return lhs;
}

logical_and_expression* parser::parse_logical_and_expression()
{
    inclusive_or_expression* oe = parse_inclusive_or_expression();
    if (!oe)
        return nullptr;

    logical_and_expression* lhs = new logical_and_expression;
    lhs->oe = oe;

    while (check("&&"))
    {
        logical_and_expression* ae = new logical_and_expression;
        ae->lhs = lhs;
        ae->rhs = accept(parse_inclusive_or_expression());
        lhs = ae;
    }
    return lhs;
}

logical_or_expression* parser::parse_logical_or_expression()
{
    logical_and_expression* ae = parse_logical_and_expression();
    if (!ae)
        return nullptr;

    logical_or_expression* lhs = new logical_or_expression;
    lhs->ae = ae;

    while (check("||"))
    {
        logical_or_expression* oe = new logical_or_expression;
        oe->lhs = lhs;
        oe->rhs = accept(parse_logical_and_expression());
        lhs = oe;
    }
    return lhs;
}

conditional_expression* parser::parse_conditional_expression()
{
    if (logical_or_expression* oe = parse_logical_or_expression())
    {
        conditional_expression* ce = new conditional_expression;
        if (check("?"))
        {
            ce->expr1 = oe;
            ce->expr2 = accept(parse_expression());
            accepts(":");
            ce->expr3 = accept(parse_conditional_expression());
        }
        else
            ce->oe = oe;

        return ce;
    }
    return nullptr;
}

assignment_expression* parser::parse_assignment_expression()
{
    if (conditional_expression* ce = parse_conditional_expression())
    {
        assignment_expression* ae = new assignment_expression;
        ae->lhs = ce;
        if (check_any({"=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|="}))
        {
            ae->op = parse_token();
            ae->rhs = accept(parse_assignment_expression());
        }
        return ae;
    }
    return nullptr;
}

constant_expression* parser::parse_constant_expression()
{
    if (conditional_expression* ce = parse_conditional_expression())
    {
        constant_expression* c = new constant_expression;
        c->ce = ce;
        return c;
    }
    return nullptr;
}

expression* parser::parse_expression()
{
    if (assignment_expression* ae = parse_assignment_expression())
    {
        expression* expr = new expression;
        expr->ae.push_back(ae);
        while (check(","))
            expr->ae.push_back(accept(parse_assignment_expression()));
        return expr;
    }
    return nullptr;
}

declaration* parser::parse_declaration()
{
    token_iter old = tokit;
    if (declaration_specifiers* ds = parse_declaration_specifiers())
    {
        declaration* decl = new declaration;
        decl->ds = ds;
        if (declarator* d = parse_declarator())
        {
            decl->d.push_back(d);
            while (check(","))
                decl->d.push_back(accept(parse_declarator()));
        }

        if (!check(";"))
        {
            tokit = old;
            delete decl;
            return nullptr;
        }

        for (declarator* d : decl->d)
        {
            auto& table = scopes.back()->vars;
            token identifier = d->get_identifier();
            if (d->dd->is_identifier() || d->dd->is_definition())
            {
                // TOOD: check which tag (union or struct)
                if (struct_or_union_specifier* sus = dynamic_cast<struct_or_union_specifier*>(ds->ts))
                    if (!sus->has_sds && !find_tag(sus->id.str)->is_defined)
                        error::reject(identifier);

                if (table.find(identifier.str) != table.end())
                    error::reject(identifier); // redefinition

                table[identifier.str] = new variable_object;
            }
            else
            {
                auto table_elem = table.find(identifier.str);
                if (table_elem != table.end())
                {
                    if (dynamic_cast<variable_object*>(table_elem->second))
                        error::reject(identifier); // redeclaration as different kind
                }
                else
                    table[identifier.str] = new function_object(false);
            }
        }
        return decl;
    }
    return nullptr;
}

declaration_specifiers* parser::parse_declaration_specifiers()
{
    if (type_specifier* ts = parse_type_specifier())
    {
        declaration_specifiers* ds = new declaration_specifiers;
        ds->ts = ts;
        return ds;
    }
    return nullptr;
}

storage_class_specifier* parser::parse_storage_class_specifier()
{
    static const vector<string> specifiers = {
        "typedef", "extern", "static", "_Thread_local", "auto", "register"
    };
    if (check_any(specifiers))
    {
        storage_class_specifier* ss = new storage_class_specifier;
        ss->tok = parse_token();
        return ss;
    }
    return nullptr;
}

type_specifier* parser::parse_type_specifier()
{
    static const vector<string> builtin_types = {
        "void", "char", "short", "int", "long", "float", "double",
        "signed", "unsigned", "_Bool", "_Complex"
    };
    if (check_any(builtin_types))
    {
        builtin_type_specifier* ts = new builtin_type_specifier;
        ts->tok = parse_token();
        return ts;
    }
    if (struct_or_union_specifier* ss = parse_struct_or_union_specifier())
        return ss;
    return nullptr;
}

struct_or_union_specifier* parser::parse_struct_or_union_specifier()
{
    if (check_any({"struct", "union"}))
    {
        struct_or_union_specifier* ss = new struct_or_union_specifier;
        ss->sou = parse_token();
        if (check("{"))
        {
            ss->has_sds = true;
            ss->sds = parse_struct_declaration_list();
            if (ss->sds.empty())
                reject();
            accepts("}");
        }
        else
        {
            ss->id = parse_identifier();
            if (check("{"))
            {
                ss->has_sds = true;
                ss->sds = parse_struct_declaration_list();
                if (ss->sds.empty())
                    reject();
                accepts("}");
            }
        }
        // todo: deal with forward declarations
        if (ss->has_sds)
        {
            auto& table = scopes.back()->tags;
            auto it = table.find(ss->id.str);
            if (it != table.end())
            {
                tag* tg = it->second;
                if (ss->has_sds && tg->is_defined)
                    error::reject(ss->id); // redefinicija
                else
                    tg->is_defined = ss->has_sds; // definicija deklariranog
            }
            else
                table[ss->id.str] = new tag(ss->has_sds); // definicija
        }
        return ss;
    }
    return nullptr;
}

struct_declaration* parser::parse_struct_declaration()
{
    if (type_specifier* ts = parse_type_specifier())
    {
        struct_declaration* sd = new struct_declaration;
        sd->ts = ts;
        sd->ds = parse_struct_declarator_list();
        if (sd->ds.empty())
            reject();
        accepts(";");
        return sd;
    }
    return nullptr;
}

vector<declarator*> parser::parse_struct_declarator_list()
{
    vector<declarator*> ds;
    if (declarator* d = parse_declarator())
    {
        ds.push_back(d);
        while (check(","))
            ds.push_back(accept(parse_declarator()));
    }
    for (declarator* dec : ds)
        if (!dec->dd->is_identifier() && !dec->dd->is_definition())
            error::reject(dec->dd->get_identifier());

    return ds;
}

vector<struct_declaration*> parser::parse_struct_declaration_list()
{
    vector<struct_declaration*> sds;
    while (struct_declaration* sd = parse_struct_declaration())
        sds.push_back(sd);

    set<string> s;
    for (struct_declaration* sd : sds)
    {
        for (declarator* dec : sd->ds)
        {
            token tok = dec->get_identifier();
            if (s.find(tok.str) != s.end())
                error::reject(tok);
            s.insert(tok.str);
        }
    }

    return sds;
}

type_qualifier* parser::parse_type_qualifier()
{
    static const vector<string> qualifiers = {
        "const", "restrict", "volatile", "_Atomic"
    };
    if (check_any(qualifiers))
    {
        type_qualifier* tq = new type_qualifier;
        tq->tok = parse_token();
        return tq;
    }
    return nullptr;
}

function_specifier* parser::parse_function_specifier()
{
    static const vector<string> specifiers = {
        "inline", "_Noreturn"
    };
    if (check_any(specifiers))
    {
        function_specifier* fs = new function_specifier;
        fs->tok = parse_token();
        return fs;
    }
    return nullptr;
}

declarator* parser::parse_declarator()
{
    token_iter old = tokit;
    if (pointer* p = parse_pointer())
    {
        declarator* decl = new declarator;
        decl->p = p;
        decl->dd = parse_direct_declarator();
        if (!decl->dd)
        {
            tokit = old;
            delete decl;
            return nullptr;
        }
        return decl;
    }
    if (direct_declarator* dd = parse_direct_declarator())
    {
        declarator* decl = new declarator;
        decl->dd = dd;
        return decl;
    }
    return nullptr;
}

direct_declarator* parser::parse_nof_direct_declarator()
{
    if (check_identifier())
    {
        direct_declarator* dd = new direct_declarator;
        dd->tok = parse_identifier();
        return dd;
    }
    if (check("("))
    {
        parenthesized_declarator* pd = new parenthesized_declarator;
        pd->decl = accept(parse_declarator());
        accepts(")");
        return pd;
    }
    return nullptr;
}

direct_declarator* parser::parse_direct_declarator()
{
    if (direct_declarator* dd = parse_nof_direct_declarator())
    {
        if (check("("))
        {
            function_declarator* fd = new function_declarator;
            fd->dd = dd;
            fd->pl = parse_parameter_type_list();
            accepts(")");
            return fd;
        }
        return dd;
    }
    return nullptr;
}

vector<parameter_declaration*> parser::parse_parameter_type_list()
{
    vector<parameter_declaration*> pl;
    if (parameter_declaration* pd = parse_parameter_declaration())
    {
        pl.push_back(pd);
        while (check(","))
            pl.push_back(accept(parse_parameter_declaration()));
    }
    return pl;
}

pointer* parser::parse_pointer()
{
    if (check("*"))
    {
        pointer* p = new pointer;
        while (type_qualifier* tq = parse_type_qualifier())
            p->tql.push_back(tq);
        if (pointer* nxt = parse_pointer())
            p->p = nxt;
        return p;
    }
    return nullptr;
}

direct_abstract_declarator* parser::parse_direct_abstract_declarator()
{
    if (check("("))
    {
        direct_abstract_declarator* dad = new direct_abstract_declarator;
        if (abstract_declarator* ad = parse_abstract_declarator())
        {
            dad->ad = ad;
            accepts(")");
            if (check("("))
            {
                dad->pl = parse_parameter_type_list();
                accepts(")");
            }
        }
        else
        {
            dad->pl = parse_parameter_type_list();
            accepts(")");
        }
        return dad;
    }
    return nullptr;
}

abstract_declarator* parser::parse_abstract_declarator()
{
    if (pointer* p = parse_pointer())
    {
        abstract_declarator* ad = new abstract_declarator;
        ad->p = p;
        if (direct_abstract_declarator* dad = parse_direct_abstract_declarator())
            ad->dad = dad;
        return ad;
    }
    else if (direct_abstract_declarator* dad = parse_direct_abstract_declarator())
    {
        abstract_declarator* ad = new abstract_declarator;
        ad->dad = dad;
        return ad;
    }
    return nullptr;
}

type_name* parser::parse_type_name()
{
    type_name* tn = new type_name;
    while (true)
    {
        if (type_specifier* ts = parse_type_specifier())
        {
            tn->sqs.push_back(ts);
            continue;
        }
        if (type_qualifier* tq = parse_type_qualifier())
        {
            tn->sqs.push_back(tq);
            continue;
        }
        break;
    }
    if (tn->sqs.empty())
        return delete tn, nullptr;

    tn->ad = parse_abstract_declarator();
    return tn;
}

parameter_declaration* parser::parse_parameter_declaration()
{
    if (declaration_specifiers* ds = parse_declaration_specifiers())
    {
        parameter_declaration* pd = new parameter_declaration;
        pd->ds = ds;
        if (declarator* decl = parse_declarator())
            pd->decl = decl;
        else
            pd->ad = parse_abstract_declarator();
        return pd;
    }
    return nullptr;
}

statement* parser::parse_statement()
{
    if (labeled_statement* ls = parse_labeled_statement())
        return ls;
    if (compound_statement* cs = parse_compound_statement(true))
        return cs;
    if (expression_statement* es = parse_expression_statement())
        return es;
    if (selection_statement* ss = parse_selection_statement())
        return ss;
    if (iteration_statement* is = parse_iteration_statement())
        return is;
    if (jump_statement* js = parse_jump_statement())
        return js;
    return nullptr;
}

labeled_statement* parser::parse_labeled_statement()
{
    if (check_identifier())
    {
        token id = parse_identifier();
        if (!check(":"))
        {
            tokit--;
            return nullptr;
        }
        goto_label* gl = new goto_label;
        gl->id = id;
        auto it = labels.find(id.str);
        if (it != labels.end())
            error::reject(id);

        gl->stat = accept(parse_statement());
        return labels[id.str] = gl;
    }
    if (check("case"))
    {
        if (!current_switch)
            reject(1);

        case_label* cl = new case_label;
        cl->ce = accept(parse_constant_expression());
        accepts(":");
        cl->stat = accept(parse_statement());
        return cl;
    }
    if (check("default"))
    {
        if (!current_switch)
            reject(1);

        default_label* dl = new default_label;
        accepts(":");
        dl->stat = accept(parse_statement());
        return dl;
    }
    return nullptr;
}

compound_statement* parser::parse_compound_statement(bool open_scope)
{
    if (check("{"))
    {
        compound_statement* cs = new compound_statement;
        if (open_scope) scopes.push_back(cs->sc = new scope);
        while (!check("}"))
            cs->bi.push_back(accept(parse_block_item()));
        if (open_scope) scopes.pop_back();
        return cs;
    }
    return nullptr;
}

block_item* parser::parse_block_item()
{
    if (declaration* decl = parse_declaration())
    {
        declaration_item* di = new declaration_item;
        di->decl = decl;
        return di;
    }
    if (statement* stat = parse_statement())
    {
        statement_item* si = new statement_item;
        si->stat = stat;
        return si;
    }
    return nullptr;
}

expression_statement* parser::parse_expression_statement()
{
    if (expression* expr = parse_expression())
    {
        expression_statement* es = new expression_statement;
        es->expr = expr;
        accepts(";");
        return es;
    }
    if (check(";"))
        return new expression_statement;
    return nullptr;
}

selection_statement* parser::parse_selection_statement()
{
    if (check("if"))
    {
        if_statement* is = new if_statement;
        accepts("(");
        is->expr = accept(parse_expression());
        accepts(")");
        is->stat = accept(parse_statement());
        if (check("else"))
            is->estat = accept(parse_statement());
        return is;
    }
    if (check("switch"))
    {
        switch_statement* ss = new switch_statement;
        accepts("(");
        ss->expr = accept(parse_expression());
        accepts(")");
        switch_statement* old_switch = current_switch;
        current_switch = ss;
        ss->stat = accept(parse_statement());
        current_switch = old_switch;
        return ss;
    }
    return nullptr;
}

iteration_statement* parser::parse_iteration_statement()
{
    if (check("while"))
    {
        while_statement* ws = new while_statement;
        accepts("(");
        ws->expr = accept(parse_expression());
        accepts(")");
        iteration_statement* old_loop = current_loop;
        current_loop = ws;
        ws->stat = accept(parse_statement());
        current_loop = old_loop;
        return ws;
    }
    if (check("do"))
    {
        do_while_statement* dws = new do_while_statement;
        iteration_statement* old_loop = current_loop;
        current_loop = dws;
        dws->stat = accept(parse_statement());
        current_loop = old_loop;
        accepts("while");
        accepts("(");
        dws->expr = accept(parse_expression());
        accepts(")");
        accepts(";");
        return dws;
    }
    if (check("for"))
    {
        for_statement* fs = new for_statement;
        accepts("(");
        fs->expr1 = parse_expression();
        accepts(";");
        fs->expr2 = parse_expression();
        accepts(";");
        fs->expr3 = parse_expression();
        accepts(")");
        iteration_statement* old_loop = current_loop;
        current_loop = fs;
        fs->stat = accept(parse_statement());
        current_loop = old_loop;
        return fs;
    }
    return nullptr;
}

jump_statement* parser::parse_jump_statement()
{
    if (check("goto"))
    {
        goto_statement* gs = new goto_statement;
        gs->id = parse_identifier();
        accepts(";");
        gotos.push_back(gs);
        return gs;
    }
    if (check("continue"))
    {
        if (!current_loop)
            reject(1);

        continue_statement* cs = new continue_statement;
        accepts(";");
        return cs;
    }
    if (check("break"))
    {
        if (!current_loop && !current_switch)
            reject(1);

        break_statement* bs = new break_statement;
        accepts(";");
        return bs;
    }
    if (check("return"))
    {
        return_statement* rs = new return_statement;
        rs->expr = parse_expression();
        accepts(";");
        return rs;
    }
    return nullptr;
}

function_definition* parser::parse_function_definition()
{
    function_definition* fd = new function_definition;
    fd->ds = accept(parse_declaration_specifiers());
    fd->dec = accept(parse_declarator());

    scopes.push_back(new scope);
    declarator* decl = fd->dec;
    while (parenthesized_declarator* pd = dynamic_cast<parenthesized_declarator*>(decl->dd))
        decl = pd->decl;

    function_declarator* fdecl = dynamic_cast<function_declarator*>(decl->dd);
    if (!fdecl)
        error::reject(decl->dd->get_identifier()); // nije funkcija

    auto& table = scopes.front()->vars;
    token identifier = fd->dec->get_identifier();
    auto table_elem = table.find(identifier.str);
    if (table_elem != table.end())
    {
        function_object* fnc = dynamic_cast<function_object*>(table_elem->second);
        if (fnc == nullptr || fnc->is_defined)
            error::reject(identifier); // redefinicija
        else
            fnc->is_defined = true; // definicija deklariranog
    }
    else
        table[identifier.str] = new function_object(true);

    for (parameter_declaration* pard : fdecl->pl)
    {
        if (!pard->decl) continue;
        declarator* decl = pard->decl;
        auto& table = scopes.back()->vars;
        token identifier = decl->get_identifier();
        if (decl->dd->is_identifier() || decl->dd->is_definition())
        {
            if (table.find(identifier.str) != table.end())
                error::reject(identifier); // redefinicija
            table[identifier.str] = new variable_object;
        }
        else
            reject(); // deklaracija | TOOD: je li ovo zbilja error?
    }

    fd->cs = accept(parse_compound_statement(false));
    resolve_gotos();
    scopes.pop_back();
    return fd;
}

external_declaration* parser::parse_external_declaration()
{
    if (declaration* decl = parse_declaration())
    {
        external_declaration* ed = new external_declaration;
        ed->decl = decl;
        return ed;
    }
    if (function_definition* fd = parse_function_definition())
    {
        external_declaration* ed = new external_declaration;
        ed->fd = fd;
        return ed;
    }
    return nullptr;
}

translation_unit* parser::parse_translation_unit()
{
    translation_unit* root = new translation_unit;
    scopes.push_back(root->sc = new scope);
    while (tokit->type != END_OF_FILE)
        root->ed.push_back(accept(parse_external_declaration()));
    scopes.pop_back();
    return root;
}
