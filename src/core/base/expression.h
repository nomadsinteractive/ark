#ifndef ARK_CORE_BASE_EXPRESSION_H_
#define ARK_CORE_BASE_EXPRESSION_H_

#include "core/forwarding.h"
#include "core/base/bean_factory.h"
#include "core/base/callable.h"
#include "core/base/plugin_manager.h"
#include "core/inf/variable.h"
#include "core/impl/builder/builder_by_instance.h"
#include "core/impl/builder/builder_by_argument.h"
#include "core/types/global.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

namespace ark {

class Expression {
private:
    template<typename T> class CallerBuilderV1 : public Builder<T> {
    public:
        CallerBuilderV1(const sp<Callable<sp<T>(const sp<T>&)>>& callable, const sp<Builder<T>>& a1)
            : _callable(callable), _a1(a1) {
        }

        virtual sp<T> build(const sp<Scope>& args) override {
            return _callable->call(_a1->build(args));
        }

    private:
        sp<Callable<sp<T>(const sp<T>&)>> _callable;
        sp<Builder<T>> _a1;
    };

    template<typename T> class CallerBuilderV2 : public Builder<T> {
    public:
        CallerBuilderV2(const sp<Callable<sp<T>(const sp<T>&, const sp<T>&)>>& callable, const sp<Builder<T>>& a1, const sp<Builder<T>>& a2)
            : _callable(callable), _a1(a1), _a2(a2) {
        }

        virtual sp<T> build(const sp<Scope>& args) override {
            return _callable->call(_a1->build(args), _a2->build(args));
        }

    private:
        sp<Callable<sp<T>(const sp<T>&, const sp<T>&)>> _callable;
        sp<Builder<T>> _a1;
        sp<Builder<T>> _a2;
    };

public:
    template<typename T> struct Operator {
        typedef sp<Builder<Variable<T>>> V;

        typedef V (*Evaluator)(const V& lvalue, const V& rvalue);

        const char*  op;
        uint32_t     priority;
        Evaluator    evaluator;

        size_t isMatch(const String& expr, size_t o) const {
            const char* c = op;
            while(*c)
                if(*c != expr.at(o++))
                    return 0;
                else
                    c++;
            return o;
        }
    };

    template<typename T, typename OP> class Compiler {
    public:
        typedef Variable<T> N;
        typedef sp<Builder<N>> V;

        V compile(BeanFactory& factory, const String& expr) const {
            V lvalue = toPhrase(expr, factory);
            if(lvalue)
                return lvalue;

            Operator<T> op;
            String remaining;
            lvalue = opCut(expr, factory, op, remaining);
            if(remaining.empty())
                return lvalue;

            do {
                Operator<T> op1 = op;
                V rvalue = reduce(op, remaining, factory, op1, remaining);
                lvalue = op.evaluator(lvalue, rvalue);
                op = op1;
            } while(remaining);

            return lvalue;
        }

    private:
        V getVariableBuilder(BeanFactory& factory, const String& expr) const {
            const char* str = expr.c_str();
            if(expr.length() > 1 && (*str == '@' || *str == '$') && Strings::isVariableName(str + 1)) {
                const sp<Builder<N>> builder = *str == '@' ? factory.getBuilderByRef<N>(str + 1) : factory.getBuilderByArg<N>(str + 1);
                DCHECK(builder, "Cannot build \"%s\"", expr.c_str());
                return builder;
            }
            return nullptr;
        }

        V toPhrase(const String& expr, BeanFactory& factory, bool compilePhrase = false) const {
            if(OP::isConstant(expr))
                return sp<BuilderByInstance<N>>::make(sp<typename N::Impl>::make(Strings::parse<T>(expr)));

            const V vBuilder = getVariableBuilder(factory, expr);
            if(vBuilder)
                return vBuilder;

            String func, params;
            if(Strings::splitFunction(expr, func, params)) {
                const Global<PluginManager> pluginManager;
                const std::vector<String> paramList = params.split(',');
                typedef sp<N> ParamType;
                if(paramList.size() == 1) {
                    const sp<Callable<ParamType(const ParamType&)>> callable = pluginManager->getCallable<ParamType(const ParamType&)>(func);
                    DCHECK(callable, "Undefined function \"%s\"", func.c_str());
                    return sp<CallerBuilderV1<N>>::make(callable, compile(factory, params));
                }
                else if(paramList.size() == 2) {
                    const sp<Callable<ParamType(const ParamType&, const ParamType&)>> callable = pluginManager->getCallable<ParamType(const ParamType&, const ParamType&)>(func);
                    DCHECK(callable, "Undefined function \"%s\"", func.c_str());
                    return sp<CallerBuilderV2<N>>::make(callable, compile(factory, paramList[0]), compile(factory, paramList[1]));
                }
                DFATAL("Unsupported parameter number: %d", paramList.size());
                return nullptr;
            }
            return compilePhrase ? compile(factory, expr) : V::null();
        }

        V reduce(const Operator<T>& op, const String& rexpr, BeanFactory& args, Operator<T>& op1, String& remaining) const {
            V s = toPhrase(rexpr, args);
            if(s) {
                remaining = "";
                return s;
            }
            s = opCut(rexpr, args, op1, remaining);
            while(op1.priority > op.priority && remaining) {
                Operator<T> tp = op1;
                s = tp.evaluator(s, opCut(remaining, args, op1, remaining));
            }
            return s;
        }

        V opCut(const String& expr, BeanFactory& factory, Operator<T>& op, String& remaining) const {
            String lvalue;
            if(expr.at(0) == '(') {
                Strings::parentheses(expr, lvalue, remaining);
                if(remaining)
                    opExpect(remaining, op, remaining);
                return compile(factory, lvalue);
            }

            const V s = toPhrase(expr, factory);
            if(s) {
                remaining = "";
                return s;
            }

            size_t size = expr.length();
            for(size_t i = 1; i < size; i ++) {
                char c = expr.at(i);
                if(isspace(c))
                    continue;
                if(c == '(')
                    i = Strings::parentheses(expr, i) + 1;
                else {
                    for(const Operator<T>& iter : OP::OPS) {
                        size_t c = iter.isMatch(expr, i);
                        if(c) {
                            op = iter;
                            lvalue = expr.substr(0, i).strip();
                            remaining = expr.substr(c).strip();
                            return compile(factory, lvalue);
                        }
                    }
                }
            }
            return OP::eval(factory, expr.strip());
        }

        void opExpect(const String& expr, Operator<T>& op, String& rvalue) const {
            size_t size = expr.length();
            for(size_t i = 0; i < size; i ++) {
                if(isspace(expr.at(i)))
                    continue;
                for(const Operator<T>& iter : OP::OPS) {
                    size_t c = iter.isMatch(expr, i);
                    if(c) {
                        op = iter;
                        rvalue = expr.substr(c).strip();
                        return;
                    }
                }
                FATAL("Illegal expression: \"%s\", unknow syntax", expr.c_str());
            }
            FATAL("Illegal expression: \"%s\", unknow syntax", expr.c_str());
        }

    };

};

}

#endif
