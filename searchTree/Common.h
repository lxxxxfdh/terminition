//
// Created by xie on 11/27/15.
//

#ifndef TERMINITION_COMMON_H
#define TERMINITION_COMMON_H
#include <vector>
#include <algorithm>
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
using namespace std;
using namespace llvm;
namespace termloop {
    const int UNOWN=INT32_MIN;
    enum satiType{
        satisfied, unsatisfied, nonfixed
    };

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
        termination, nontermination, nosupport, unknowterm
    };

    template<class T>
    bool isInVector(vector<T*>*vb,T* ins){

        if(std::find(vb->begin(),vb->end(),ins)!=vb->end())
            return true;
        return false;

    };
    template<class T>
    bool isInVector(vector<T>*vb,T ins){

        if(std::find(vb->begin(),vb->end(),ins)!=vb->end())
            return true;
        return false;

    };
    bool symbolCmp(int x,int y,cmpSymbol sym);
    struct condition{
        Value* controlVar;
        cmpSymbol sym;
        Value* c;
        condition():controlVar(nullptr),sym(other),c(nullptr){}
        bool isEmpty(){
            if(controlVar==nullptr||c== nullptr)
                return true;
            return false;
        }
        void output(){
            if(controlVar== nullptr)
                errs()<<"Control Var Null!\r\n";
            else {
                errs() << "Control Variable: " << controlVar->getName() << "\r\n";
                errs() << "Symbole: " << (cmpSymbol) sym << "\r\n";
                errs() << "C: " << *c << "\r\n";
            }
        }
    };

    //whether the var is changing in the loop iteration
    //coarse implementation

    Value* isIterativeVar(Value* var, Loop* l);
    int isConstantValue(Value *v);

    //find the condition with the form x~c ~ is {<,<=,>,>=}
    //tag: 0 normal, 1 reverse
    //coarse implementation
    struct condition checkCond(ICmpInst* inst, int tag, Loop* l);
    bool  isAlliterator(gep_type_iterator I,  gep_type_iterator E, Loop* l);



}
#endif //TERMINITION_COMMON_H
