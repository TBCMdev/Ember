#pragma once
#include "Base.h"
#include "lexertk.hpp"
#include <unordered_map>
#include <any>
#include <vector>
#include <iostream>

#undef STRICT // to allow INTERLINK_TYPE to have STRICT inside it.

#define C_FLAG(x, s) \
    {                \
        x, s         \
    }

std::unordered_map<std::string, bool> COMPILER_FLAGS = {
    C_FLAG("GAME_LIB_ENABLE", 0),

};
enum class INTERLINK_TYPE
{
    MUTABLE,
    STRICT,
    SOLID
};
struct CPPClassInterlink {
    Base::Decl type = Base::Decl::UNKNWN;
    INTERLINK_TYPE link_type = INTERLINK_TYPE::MUTABLE;
    //_Val value = nullptr;
    void setType(Base::Decl d) { type = d; }
    //void setValue(_Val a) { value = a; }
    void setLinkType(INTERLINK_TYPE t) { link_type = t; }

};
struct CPPVInterlink
{
    Base::Decl type = Base::Decl::UNKNWN;
    INTERLINK_TYPE link_type = INTERLINK_TYPE::MUTABLE;
    std::any value = nullptr;
    void setType(Base::Decl d) { type = d; }
    void setValue(std::any a) { value = a; }
    void setLinkType(INTERLINK_TYPE t) { link_type = t; }

};
struct _PRECOMPILE_INFO
{
    bool __complete = false;
    struct DefinerCFLAG {
        std::vector<lexertk::token> toks;
        bool functional = false;
        std::shared_ptr<std::vector<lexertk::token>> params;
        std::vector<lexertk::token> insert_params(std::vector<std::vector<lexertk::token*>> parameters) {

            std::vector<lexertk::token> c;
            for (int i = 0; i < toks.size(); i++) {

                int index = -1;
                for (int ix = 0; ix < params->size(); ix++) {
                    if ((*params)[ix].value == toks[i].value) {
                        index = ix;
                        break;
                    }
                }

                if (index == -1)
                    c.push_back(toks[i]);
                else {
                    for (const auto& token : parameters[index]) {
                        c.push_back(*token);
                    }
                    // if (index < params->size() - 1)
                    //     c.push_back(lexertk::token(","));
                }
            }
            return c;
        }
    };
    std::unordered_map<std::string, DefinerCFLAG> _CFLAG_DEFINES;
    std::vector<CPPVInterlink> _CFLAG_INTERLINKS;
    std::unordered_map<std::string, CPPClassInterlink> _CFLAG_C_INTERLINKS;
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
                int e_start = index;
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
                    std::vector<lexertk::token> defined_to;
                    std::vector<lexertk::token>* parameters = nullptr;

                    if (advance().value != "{")
                        if (token->value != "(")
                            continue;
                        else
                            parameters = new std::vector<lexertk::token>();

                    bool is_functional = (parameters != nullptr);
                    if (is_functional) {
                        int b = 1;
                        int c = 0;
                        while (b > 0) {
                            advance();
                            if (token->value == "(") b++;
                            else if (token->value == ")") { b--; }
                            else if (token->value == ",") c++;
                            else
                                parameters->push_back(*token);

                        }
                        advance();
                    }

                    while (index < __size && getnext().value != "}") defined_to.push_back(advance());

                    changes._CFLAG_DEFINES.insert({ name, _PRECOMPILE_INFO::DefinerCFLAG{defined_to, is_functional, std::shared_ptr<std::vector<lexertk::token>>(parameters)} });

                    if (is_functional)
                        advance();
                }
                else if (token->value == "interlink")
                {
                    lexertk::token& d_type = advance();
                    std::string d_name = advance().value;
                    lexertk::token* creation_type = getnext().value != "{" ? &advance() : nullptr;
                    CPPClassInterlink i{Base::declareParse(d_type), creation_type == nullptr ? INTERLINK_TYPE::MUTABLE : (creation_type->value == "solid" ? INTERLINK_TYPE::SOLID :
                        (creation_type->value == "strict" ? INTERLINK_TYPE::STRICT : INTERLINK_TYPE::MUTABLE) )};
                    
                    
                    ClassStructure structure;
                    bool has_value = (getnext().value == "{");

                    //[TODO]
                    if (has_value) {

                        advance();
                        int b = 1;
                        while (b > 0) {
                            advance();
                            if (token->value == "{") b++;
                            else if (token->value == "}") { b--; }
                            else if (auto x = Base::declareParse(*token); x != Base::Decl::UNKNWN || ClassHandler::hasClass(token->value)) {
                                std::string name = advance().value;
                                bool method = (advance().value == "(");
                                if (!method && getnext().value != ",") break;
                                bool constructor = (name == "constructor");
                                if (method) 
                                {
                                    // parse parameters, and make:
                                    cpp_Lambda function({});
                                    structure.members.insert({ name, std::make_shared<ClassStructure::ClassFunction>(name, ClassStructure::MEMBERPROTECTION::PUBLIC, constructor) });
                                }
                                //else structure.members.push_back(std::make_shared<ClassStructure::ClassVariable>(name, ClassStructure::MEMBERPROTECTION::PUBLIC, Base::Decl));
                            }
                        }
                        if (advance().value != "}") throw marine::errors::SyntaxError("Expected closing '}' after interlink precompile statement.");
                    }
                }
                generator.snipout(generator.ibegin() + e_start, generator.ibegin() + index + 1);
                index -= __size - generator.size() - 1;
                __size = generator.size();
                token = &generator[index];
            }

            advance();
        }
        auto concatenate_v_tok = [](lexertk::generator& g, int& index, _PRECOMPILE_INFO::DefinerCFLAG& flag) {
            int start = index;
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
                        parameter.clear();
                        continue;
                    }
                    parameter.push_back(&g[start + i]);

                }
                parameters.push_back(parameter);

                auto x = flag.insert_params(parameters);
                auto begin = g.ibegin() + start;
                const int s = x.size();
                g.snipout(begin, begin + s);


                for (int i = 0; i < s; i++) {
                    g.insert(g.ibegin() + i + start, x[i]);
                }
            }
            else
            {
                auto begin = g.ibegin() + start;
                g.snipout(begin, begin + 1);
                // inserts the specified DEFINE value at the index of the DEFINE key name found.
                for (int i = 0; i < flag.toks.size(); i++) {
                    g.insert(g.ibegin() + i + start, flag.toks[i]);
                }

            }
        };


        // perform changes to generator
        int cpy_index = 0;
        while (cpy_index < __size) {
            if (changes._CFLAG_DEFINES.find(generator[cpy_index].value) != changes._CFLAG_DEFINES.end()) {
                concatenate_v_tok(generator, cpy_index, changes._CFLAG_DEFINES[generator[cpy_index].value]);
            }
            cpy_index++;
        }
        changes.__complete = true;
        return changes;
    }
    catch (std::exception& e)
    {
        //throw e;
        return changes;
    }
}