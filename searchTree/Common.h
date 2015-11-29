//
// Created by xie on 11/27/15.
//

#ifndef TERMINITION_COMMON_H
#define TERMINITION_COMMON_H
#include <vector>
#include <algorithm>
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
using namespace std;
using namespace llvm;
namespace termloop {
    const int UNOWN=-99999;

    enum nodeType {
        pathnum, monotonic, controlabove, result, empty, unsupport
    };
    enum monotonicity {
        increasing, decreasing, constant, unknown
    };
    enum cmpSymbol {
        get=0, gt,let,lt,other
    };
    enum Result{
        termination, nontermination, nosupport
    };

    template<class T>
    bool isInVector(vector<T*>*vb,T* ins){

        if(std::find(vb->begin(),vb->end(),ins)!=vb->end())
            return true;
        return false;

    };

    struct condition{
        Value* controlVar;
        cmpSymbol sym;
        Value* c;
        condition():controlVar(nullptr),sym(other),c(nullptr){}
        void output(){
            errs()<<"Control Variable: "<<controlVar->getName()<<"\r\n";
            errs()<<"Symbole: "<<(cmpSymbol)sym<<"\r\n";
            errs()<<"C: "<< *c<<"\r\n";

        }
    };

    //whether the var is changing in the loop iteration
    //coarse implementation

    bool isIterativeVar(Value* var);

    //find the condition with the form x~c ~ is {<,<=,>,>=}
    //tag: 0 normal, 1 reverse
    //coarse implementation
    struct condition checkCond(ICmpInst* inst, int tag);



}
#endif //TERMINITION_COMMON_H
