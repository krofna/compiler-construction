#include "types.h"

extern LLVMContext context;

vector<scope*> scopes;
vector<goto_statement*> gotos;
map<string, goto_label*> labels;

void resolve_gotos()
{
    for (goto_statement* gs : gotos)
    {
        auto glp = labels.find(gs->id.str);
        if (glp == labels.end())
            error::reject(gs->id);

        gs->gl = glp->second;
    }
    gotos.clear();
    labels.clear();
}

object* find_var(const string& id)
{
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i)
    {
        scope* s = *i;
        auto it = s->vars.find(id);
        if (it != s->vars.end())
            return it->second;
    }
    return nullptr;
}

variable_object* find_variable(const string& id)
{
    return dynamic_cast<variable_object*>(find_var(id));
}

tag* find_tag(const string& id)
{
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i)
    {
        scope* s = *i;
        auto it = s->tags.find(id);
        if (it != s->tags.end())
            return it->second;
    }
    return nullptr;
}

void register_type(type_specifier* ts)
{
    struct_or_union_specifier* ss = dynamic_cast<struct_or_union_specifier*>(ts);
    if (!ss)
        return;

    if (!ss->has_sds)
        return;

    auto& table = scopes.back()->tags;
    auto it = table.find(ss->id.str);
    if (it != table.end())
        error::reject(ss->id); // redefinicija

    // definicija
    tag *t = new tag;
    t->type = make_struct(ss);
    table[ss->id.str] = t;

    for (struct_declaration* sd : ss->sds)
    {
        for (declarator* dec : sd->ds)
        {
            token tok = dec->get_identifier();
            if (t->s.find(tok.str) != t->s.end())
                error::reject(tok);
            t->s.insert(tok.str);
        }
    }
}
