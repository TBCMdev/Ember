#pragma once
#include "../lexertk.hpp"
#include <unordered_map>
#include <any>
#include <vector>
#include <iostream>

#undef STRICT // to allow INTERLINK_TYPE to have STRICT inside it.

#define C_FLAG(x, s) \
    {                \
        x, s         \
    }
namespace Base {
    enum class Decl {
        INT,
        FLOAT,
        STRING,
        BOOL,
        LIST,
        DYNAMIC_OBJECT,
        STATIC_OBJECT,
        CUSTOM,
        RUNTIME_DECIDED,
        UNKNWN,
        LAMBDA,
        NULLIFIED
    };
};
std::unordered_map<std::string, bool> COMPILER_FLAGS = {
    C_FLAG("GAME_LIB_ENABLE", 0),

};
struct CPPInterlink
{
    enum class INTERLINK_TYPE
    {
        MUTABLE,
        STRICT,
        SOLID
    };
    template <typename T>
    struct InterlinkWantedValueRaw
    {
        Base::Decl internal_decl = Base::Decl::UNKNWN;
        T item_container;
    };
    struct InterlinkWantedValueClass : public InterlinkWantedValueRaw<std::vector<lexertk::token*>>
    {
    public:
        InterlinkWantedValueClass(std::vector<lexertk::token*> parsable_value, Base::Decl internal_decl) : InterlinkWantedValueRaw<std::vector<lexertk::token*>>{ internal_decl, parsable_value } {}
        void parse()
        {
        }
    };
    struct InterlinkWantedValue : public InterlinkWantedValueRaw<lexertk::token*>
    {

    public:
        InterlinkWantedValue(lexertk::token& singular_value, Base::Decl internal_decl) : InterlinkWantedValueRaw<lexertk::token*>{ internal_decl, &singular_value } {}
        void cast()
        {
        }
    };

    Base::Decl type;
    lexertk::token& name;
    INTERLINK_TYPE link_type;
    InterlinkWantedValue rawValFromTo;
};
struct _PRECOMPILE_INFO
{
    bool __complete = false;
    struct DefinerCFLAG {
        std::vector<lexertk::token*> toks;
        bool functional = false;
        std::shared_ptr<std::vector<lexertk::token*>> params;
        std::vector<lexertk::token> insert_params(std::vector<std::vector<lexertk::token*>> parameters) {
            
            std::vector<lexertk::token> c;
            for (int i = 0; i < toks.size(); i++) {

                int index = -1;
                for (int ix = 0; ix < params->size(); ix++) {
                        // std::cout << (*params)[i]->value << " == " << toks[ti]->value << '\n';
					if ((*params)[ix]->value == toks[i]->value) {
						index = ix;
						break;
					}
                }
                std::cout << index << ", " << toks[i]->value << '\n';
                if (index == -1)
                    c.push_back(*toks[i]);
                else {
                    for (const auto& token : parameters[index]) {
                        std::cout << "ADDING +:" << token->value << '\n';
                        c.push_back(*token);
                    }
                    if (index < params->size() - 1)
                        c.push_back(lexertk::token(","));
                }
            }
            for (const auto& x : c) std::cout << x.value << '\n';
            return c;
        }
    };
    std::unordered_map<std::string, DefinerCFLAG> _CFLAG_DEFINES;
    std::vector<CPPInterlink> _CFLAG_INTERLINKS;
};
_PRECOMPILE_INFO pre_compile(lexertk::generator& generator)
{
    _PRECOMPILE_INFO changes;
    if (generator.size() == 0)
        return changes;
    int index = 0;

    try
    {

        int __size = generator.size();
        lexertk::token* token = &generator[index];

        auto advance = [&index, &generator, &token](int b = 1) -> lexertk::token&
        {index += b; token = &generator[index]; return generator[index]; };
        auto getnext = [&index, &generator, &token](int b = 1) -> lexertk::token&
        { token = &generator[index + b]; return generator[index + b]; };


        while (index < __size)
        {
            // COMPILER FLAG
            if (token->value == "~")
            {
                advance();
                if (token->value == "cflag")
                {
                    if (COMPILER_FLAGS.find(token->value) != COMPILER_FLAGS.end())
                    {
                        COMPILER_FLAGS[token->value] = std::stoi(advance().value);
                    }
                }
                else if (token->value == "define")
                {
                    std::string name = advance().value;
                    // check that there are no invalid chars
                    if (name == " ")
                        continue;

                    std::vector<lexertk::token*> defined_to;
                    std::vector<lexertk::token*>* parameters = nullptr;

                    if (advance().value != "{")
                        if (token->value != "(")
                            continue;
                        else
                            parameters = new std::vector<lexertk::token*>();

                    if (parameters != nullptr) {
                        int b = 1;
                        int c = 0;
                        while (b > 0) {
                            advance();
                            if (token->value == "(") b++;
                            else if (token->value == ")") { b--;}
                            else if (token->value == ",") c++;
                            else
                                parameters->push_back(token);

                        }
                    }
                    advance();
                    std::cout << "CUR BEFORE:" << token->value << '\n';
                    while (index < __size && getnext().value != "}") {
                        std::cout << "ADDING:" << getnext().value << '\n';
                        defined_to.push_back(&advance());
                    }
                    changes._CFLAG_DEFINES.insert({ name, _PRECOMPILE_INFO::DefinerCFLAG{defined_to, (parameters != nullptr), std::shared_ptr<std::vector<lexertk::token*>>(parameters)}});
                }
                else if (token->value == "interlink")
                {
                }
            }
            index++;
        }
        auto concatenate_v_tok = [](lexertk::generator& g, int& index, _PRECOMPILE_INFO::DefinerCFLAG& flag) {
            size_t start = (size_t)index;
            const int s = flag.toks.size();
            if (flag.functional) {
                std::vector<std::vector<lexertk::token*>> parameters;
                index++;
                if (g[index].value != "(") return;

                int b = 1;
                std::vector<lexertk::token*> parameter;
                for (int i = 2; i < s - 1; i++) {

                    if (g[start + i].value == ",") {
                        parameters.push_back(parameter);
                        parameter = std::vector<lexertk::token*>();

                        continue;
                    }
                    parameter.push_back(&g[start + i]);
                    
                }
                if (parameters.size() == 0)
                    parameters.push_back(parameter);

                auto x = flag.insert_params(parameters);
                auto begin = g.ibegin() + start;
                const int s = x.size();
                g.snipout(begin, begin + s);

                g.resize(s);

                for (int i = 0; i < s; i++) {
                    g.insert(g.ibegin() + i + start, x[i]);
                }
            }
            else 
            {
                auto begin = g.ibegin() + start;
                g.snipout(begin, begin + 1);
                g.resize(1);
                // inserts the specified DEFINE value at the index of the DEFINE key name found.
                for (int i = 0; i < flag.toks.size(); i++) {
                    g.insert(g.ibegin() + i + start, *flag.toks[i]);
                }
                
            }
        };


        // perform changes to generator
        int cpy_index = 0;
        while (cpy_index < __size) {
            if (changes._CFLAG_DEFINES.find(generator[cpy_index].value) != changes._CFLAG_DEFINES.end()) {
                if (generator[cpy_index - 1].value != "define")
                    concatenate_v_tok(generator, cpy_index, changes._CFLAG_DEFINES[generator[cpy_index].value]);
            }
            cpy_index++;
        }
        std::cout << "Pre-compilation completed.\n";
        changes.__complete = true;
        return changes;
    }
    catch (std::exception& e)
    {
        throw e;
        return changes;
    }
}