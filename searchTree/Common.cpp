//
// Created by xie on 11/29/15.
//

#include "Common.h"
#include "llvm/IR/Constants.h"
using namespace termloop;
Value* termloop::isIterativeVar(Value* var){
    if(SExtInst* sin=dyn_cast<SExtInst>(var))
        return isIterativeVar(sin->getOperand(0));
    if(isa<PHINode>(var))
        return var;

        return nullptr;
}
int termloop::isConstantValue(Value *v) {


    if (BinaryOperator *bop=dyn_cast<BinaryOperator>(v)) {
        Value* op1=bop->getOperand(0);
        Value* op2=bop->getOperand(1);
        if(ConstantInt *c1=dyn_cast<ConstantInt>(op1)){
            if(ConstantInt *c2=dyn_cast<ConstantInt>(op2)){
                switch (bop->getOpcode()) {
                    case Instruction::Add: {
                        return c1->getZExtValue()+c2->getZExtValue();
                        break;
                    }

                    case Instruction::Sub: {
                        return c1->getZExtValue()-c2->getZExtValue();
                        break;
                    }
                    case Instruction::Mul:{
                        return c1->getZExtValue()*c2->getZExtValue();
                    }

                }
            }
        }
    }
    return INT32_MIN;
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

struct condition  termloop::checkCond(ICmpInst* inst, int tag){
    Value* v1=inst->getOperand(0);
    Value* v2=inst->getOperand(1);
    cmpSymbol conData[2][5]={{cmpSymbol::get,cmpSymbol::gt,cmpSymbol::let,cmpSymbol::lt,cmpSymbol::other},
                             {cmpSymbol::lt,cmpSymbol::gt,cmpSymbol::gt,cmpSymbol::get,cmpSymbol::other}};

    condition cond;
    Value* vv1=isIterativeVar(v1);
    Value* vv2=isIterativeVar(v2);
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

