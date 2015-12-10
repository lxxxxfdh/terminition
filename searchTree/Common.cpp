//
// Created by xie on 11/29/15.
//

#include "Common.h"
using namespace termloop;
bool termloop::isIterativeVar(Value* var){
    if(isa<PHINode>(var))
        return true;
    else
        return false;
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
    if(isIterativeVar(v1)&&!isIterativeVar(v2)){
        cond.controlVar=v1;
        cond.c=v2;
        cmpSymbol  cm;
        cond.controlVar=v1;
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

