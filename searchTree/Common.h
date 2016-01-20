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
    enum monotoneType{
        arithmetic, geometric, Irregular, unknownmono
    };
    enum satiType{
        satisfied, unsatisfied, nonfixed
    };

    enum nodeType {
        pathnum, monotonic, controlabove, result, empty, gapfree,gaphandle, unsupport
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
    struct varIncrease{
        Value* var;
        monotonicity increase;
        int varValue;
        monotoneType monoType;
        int step;
        int v;
        void init(Value* val){
            var= val;
            increase=unknown;
            monoType=unknownmono;
            step=UNOWN;
            varValue=UNOWN;
            v=UNOWN;
        }
        varIncrease(): var(nullptr), increase(unknown), monoType(unknownmono),step(UNOWN),varValue(UNOWN),v(UNOWN){}
    };
    struct twoVals{
        Value* iterVal1;
        Value* iterVal2;
        Value* c;
        Instruction::BinaryOps posOrNeg;
        //-1: no iterval, 0: x-y  ,1 :x, 2: x+c,3: c
        int getType(){
            if(iterVal1!=nullptr){
                if(iterVal2!=nullptr)
                    return 0;
                else if(c==nullptr)
                    return 1;
                else
                    return 2;
            }else
                return -1;

        }
        twoVals():iterVal1(nullptr),iterVal2(nullptr), c(nullptr),posOrNeg(Instruction::Add){}
    };
    struct condition{
        Value* controlVar;
        Value* controlVar2;
        cmpSymbol sym;
        Value* c;
        Instruction::BinaryOps cPosOrNeg;
        condition():controlVar(nullptr),controlVar2(nullptr),sym(other),c(nullptr),cPosOrNeg(Instruction::Add){}
        bool isEmpty(){
            if(controlVar==nullptr||c== nullptr)
                return true;
            return false;
        }
        void output(){
            if(controlVar== nullptr)
                errs()<<"Control Var Null!\r\n";
            else {
                errs() << "Control Variable x: " << controlVar->getName() << "\r\n";
                if(controlVar2!= nullptr)
                    errs()<<"Control Variable y: "<<controlVar2->getName()<<"\r\n";
                errs() << "Symbole: " << (cmpSymbol) sym << "\r\n";
                errs() << "C: " << *c << "\r\n";
            }
        }
    };

    //whether the var is changing in the loop iteration
    //coarse implementation

    twoVals isIterativeVar(Value* var, Loop* l);
    int isConstantValue(Value *v);

    //find the condition with the form x~c ~ is {<,<=,>,>=}
    //tag: 0 normal, 1 reverse
    //coarse implementation
    struct condition checkCond(ICmpInst* inst, int tag, Loop* l);
    bool  isAlliterator(gep_type_iterator I,  gep_type_iterator E, Loop* l);



}
#endif //TERMINITION_COMMON_H
