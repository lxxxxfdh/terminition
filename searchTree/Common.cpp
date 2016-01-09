//
// Created by xie on 11/29/15.
//

#include "Common.h"

#include "llvm/IR/Constants.h"
using namespace termloop;

bool  termloop::isAlliterator(gep_type_iterator I,  gep_type_iterator E, Loop* l)
{
    int num=0;
    for(; I != E; ++I)
    {
        Value* v=I.getOperand();
       if(isIterativeVar(v,l))
           return  true;

    }
    return  false;
}
Value* termloop::isIterativeVar(Value* var, Loop* l){
    if(Instruction* ins=dyn_cast<Instruction>(var)){
        if(!l->contains(ins))
            return nullptr;
    }
    if(SExtInst* sin=dyn_cast<SExtInst>(var))
        return isIterativeVar(sin->getOperand(0),l);
    if(isa<PHINode>(var))
        return var;
    if(ZExtInst* zin=dyn_cast<ZExtInst>(var))
        return  isIterativeVar(zin->getOperand(0),l);
    if(BinaryOperator* bop=dyn_cast<BinaryOperator>(var)){ //for the while(i-=12>=2)
        Value* op1=bop->getOperand(0);
        Value* op2=bop->getOperand(1);
        if(isIterativeVar(op1,l)&&!isIterativeVar(op2,l))
            return op1;
        if(!isIterativeVar(op1,l)&&isIterativeVar(op2,l))
            return op2;
    }
    if(GetElementPtrInst* getEle=dyn_cast<GetElementPtrInst>(var)){ //char  *-3 or
        Value* base=getEle->getOperand(0);
        if(isIterativeVar(base,l)&&!isAlliterator(gep_type_begin(getEle), gep_type_end(getEle),l))
            return base;
    }

        return nullptr;
}
int termloop::isConstantValue(Value *v) {

    if(ConstantInt* cons=dyn_cast<ConstantInt>(v))
        return cons->getZExtValue();
    if(SExtInst* sex=dyn_cast<SExtInst>(v))
        return isConstantValue(sex->getOperand(0));
    if (BinaryOperator *bop=dyn_cast<BinaryOperator>(v)) {
        Value* op1=bop->getOperand(0);
        Value* op2=bop->getOperand(1);
        int num1=isConstantValue(op1);
        int num2=isConstantValue(op2);
        if(num1!=UNOWN&&num2!=UNOWN){
            switch (bop->getOpcode()) {
                case Instruction::Add: {
                    return num1+num2;
                    break;
                }

                case Instruction::Sub: {
                    return num1-num2;
                    break;
                }
                case Instruction::Mul:{
                    return num1*num2;
                }

            }
        }

    }
    return UNOWN;
}

bool termloop::symbolCmp(int x,int y,cmpSymbol sym){
    switch (sym){
        case lt:
            return x<y;
        case let:
            return x<=y;
        case cmpSymbol:: get:
            return x>=y;
        case gt:
            return x>y;
        default:
            assert(false);
    }
}

struct condition  termloop::checkCond(ICmpInst* inst, int tag, Loop* l){
    Value* v1=inst->getOperand(0);
    Value* v2=inst->getOperand(1);
    cmpSymbol conData[2][5]={{cmpSymbol::get,cmpSymbol::gt,cmpSymbol::let,cmpSymbol::lt,cmpSymbol::other},
                             {cmpSymbol::lt,cmpSymbol::gt,cmpSymbol::gt,cmpSymbol::get,cmpSymbol::other}};

    condition cond;
    Value* vv1=isIterativeVar(v1,l);
    Value* vv2=isIterativeVar(v2,l);

    if(vv1!=nullptr&&vv2== nullptr){
       // errs()<<*vv1;
        cond.controlVar=vv1;
        cond.c=v2;
        cmpSymbol  cm;
        cond.controlVar=vv1;
        cond.c=v2;
        switch (inst->getPredicate()){
            case ICmpInst::ICMP_UGE:
            case ICmpInst::ICMP_SGE:
                cm=cmpSymbol::get;
                break;
            case ICmpInst::ICMP_ULT:
            case ICmpInst::ICMP_SLT:
                cm=cmpSymbol::lt;
                break;
            case ICmpInst::ICMP_ULE:
            case ICmpInst::ICMP_SLE:
                cm=cmpSymbol::let;
                break;
            case ICmpInst::ICMP_UGT:
            case ICmpInst::ICMP_SGT:
                cm=cmpSymbol::gt;
                break;
            default:
                cm=cmpSymbol::other;
        }
        cond.sym=conData[tag][cm];

    }
    return cond;

}

