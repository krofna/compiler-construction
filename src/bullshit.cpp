#include "types.h"

extern LLVMContext context;

vector<scope*> scopes;
map<string, tag*> htags;
int tag_counter;

Type* valid_type_specifier(vector<type_specifier*> tsps)
{
    static const vector<pair<vector<vector<string>>, Type*>> valid = {
        {{{"void"}}, Type::getVoidTy(context)},
        {{{"char"}}, Type::getInt8Ty(context)},
        {{{"signed", "char"}}, Type::getInt8Ty(context)},
        {{{"unsigned", "char"}}, Type::getInt8Ty(context)},
        {{{"short"},
          {"signed", "short"},
          {"short", "int"},
          {"signed", "short", "int"}}, Type::getInt16Ty(context)},
        {{{"unsigned", "short"},
          {"unsigned", "short", "int"}}, Type::getInt16Ty(context)},
        {{{"int"},
          {"signed"},
          {"signed", "int"}}, Type::getInt32Ty(context)},
        {{{"unsigned"},
          {"unsigned", "int"}}, Type::getInt32Ty(context)},
        {{{"long"},
          {"signed", "long"},
          {"long", "int"},
          {"signed", "long", "int"}}, Type::getInt64Ty(context)},
        {{{"unsigned", "long"},
          {"unsigned", "long", "int"}}, Type::getInt64Ty(context)},
        {{{"long", "long"},
          {"signed", "long", "long"},
          {"long", "long", "int"},
          {"signed", "long", "long", "int"}}, Type::getInt64Ty(context)},
        {{{"unsigned", "long", "long"},
          {"unsigned", "long", "long", "int"}}, Type::getInt64Ty(context)},
        {{{"float"}}, Type::getFloatTy(context)},
        {{{"double"}}, Type::getDoubleTy(context)},
        {{{"long", "double"}}, Type::getFP128Ty(context)},
        {{{"_Bool"}}, Type::getInt1Ty(context)},
        {{{"float", "_Complex"}}, nullptr},
        {{{"double", "_Complex"}}, nullptr},
        {{{"long", "double", "_Cmplex"}}, nullptr}
    };

    map<string, int> freqb;
    for (type_specifier* ts : tsps)
    {
        builtin_type_specifier* bts = dynamic_cast<builtin_type_specifier*>(ts);
        freqb[bts->tok.str]++;
    }

    for (auto& [tsv, type] : valid)
    {
        for (auto& ts : tsv)
        {
            map<string, int> freqa;
            for (const string& s : ts)
                freqa[s]++;
            if (freqa == freqb)
                return type;
        }
    }
    return nullptr;
}

void function_definition::resolve_gotos()
{
    for (goto_statement* gs : gotos)
    {
        auto glp = labels.find(gs->id.str);
        if (glp == labels.end())
            error::reject(gs->id);

        gs->gl = glp->second;
    }
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

function_object* find_function(const string& id)
{
    return dynamic_cast<function_object*>(find_var(id));
}

tag::tag() : is_complete(false)
{
    type = StructType::create(context, h = to_string(tag_counter++));
}

void tag::complete(vector<struct_declaration*>& sds)
{
    vector<Type*> members;
    for (struct_declaration* sd : sds)
    {
        for (declarator* dec : sd->ds)
        {
            token tok = dec->get_identifier();
            if (indices.find(tok.str) != indices.end())
                error::reject(tok);

            int next = indices.size();
            indices[tok.str] = next;
            members.push_back(dec->gen_type(sd->type));
        }
    }
    type->setBody(members);
    is_complete = true;
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

Type* register_type(struct_or_union_specifier* ss)
{
    auto& table = scopes.back()->tags;
    if (!ss->has_sds)
    {
        if (tag* t = find_tag(ss->id.str))
            return t->type;

        tag *t = new tag;
        table[ss->id.str] = t;
        htags[t->h] = t;
        return t->type;
    }
    else
    {
        auto it = table.find(ss->id.str);
        if (it != table.end())
        {
            tag *t = it->second;
            if (t->is_complete)
                error::reject(ss->id); // redefinicija
            else
                t->complete(ss->sds);
            return t->type;
        }
        else
        {
            // definicija
            tag *t = new tag;
            t->complete(ss->sds);
            table[ss->id.str] = t;
            htags[t->h] = t;
            return t->type;
        }
    }
}
