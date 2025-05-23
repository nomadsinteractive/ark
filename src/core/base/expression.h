#pragma once

#include "core/forwarding.h"
#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/operators.h"
#include "core/util/strings.h"

namespace ark {

class Expression {
public:
    template<typename T> struct Operator {
        typedef sp<Builder<Variable<T>>> V;

        typedef V (*Evaluator)(const V& lvalue, const V& rvalue);

        const char*  op;
        uint32_t     priority;
        Evaluator    evaluator;

        size_t isMatch(const String& expr, size_t o) const {
            const char* c = op;
            while(*c) {
                if(*c != expr.at(o++))
                    return 0;
                c++;
            }
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
                const sp<Builder<N>> builder = *str == '@' ? factory.getBuilderByRef<sp<N>>(Identifier::parseRef(str + 1)) : factory.getBuilderByArg<sp<N>>(str + 1);
                DCHECK(builder, "Cannot build \"%s\"", expr.c_str());
                return builder;
            }
            return nullptr;
        }

        V toPhrase(const String& expr, BeanFactory& factory, bool compilePhrase = false) const {
            if(OP::isConstant(expr))
                return sp<typename Builder<N>::Prebuilt>::make(sp<typename N::Impl>::make(Strings::eval<T>(expr)));

            if(V vBuilder = getVariableBuilder(factory, expr))
                return vBuilder;

            return compilePhrase ? compile(factory, expr) : V();
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

            if(V s = toPhrase(expr, factory)) {
                remaining = "";
                return s;
            }

            const size_t size = expr.length();
            for(size_t i = 1; i < size; i ++) {
                const char c = expr.at(i);
                if(isspace(c))
                    continue;
                if(c == '(')
                    i = Strings::parentheses(expr, i) + 1;
                else
                    for(const Operator<T>& j : OP::OPS)
                        if(size_t c = j.isMatch(expr, i)) {
                            op = j;
                            lvalue = expr.substr(0, i).strip();
                            remaining = expr.substr(c).strip();
                            return compile(factory, lvalue);
                        }
            }
            return OP::eval(factory, expr.strip());
        }

        void opExpect(const String& expr, Operator<T>& op, String& rvalue) const {
            const size_t size = expr.length();
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

template<typename T> class NumericOperation {
public:
    typedef sp<Builder<Variable<T>>> BuilderType;

    static bool isConstant(const String& expr) {
        return Strings::isNumeric(expr);
    }

    static BuilderType add(const BuilderType& a1, const BuilderType& a2) {
        return sp<Operators::Builder<T, Operators::Add<T>>>::make(a1, a2);
    }

    static BuilderType subtract(const BuilderType& a1, const BuilderType& a2) {
        return sp<Operators::Builder<T, Operators::Sub<T>>>::make(a1, a2);
    }

    static BuilderType multiply(const BuilderType& a1, const BuilderType& a2) {
        return sp<Operators::Builder<T, Operators::Mul<T>>>::make(a1, a2);
    }

    static BuilderType divide(const BuilderType& a1, const BuilderType& a2) {
        return sp<Operators::Builder<T, Operators::Div<T>>>::make(a1, a2);
    }

    static BuilderType eval(BeanFactory& /*factory*/, const String& expr) {
        if(Strings::isNumeric(expr))
            return sp<typename Builder<Variable<T>>::Prebuilt>::make(sp<typename Variable<T>::Const>::make(Strings::eval<T>(expr)));
        return nullptr;
    }

    static Expression::Operator<T> OPS[4];
};

template<typename T> Expression::Operator<T> NumericOperation<T>::OPS[4] = {
    {"+", 1, NumericOperation<T>::add},
    {"-", 1, NumericOperation<T>::subtract},
    {"*", 2, NumericOperation<T>::multiply},
    {"/", 2, NumericOperation<T>::divide}
};

}
