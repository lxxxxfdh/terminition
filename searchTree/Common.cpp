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
        int type=isIterativeVar(v,l).getType();
       if(type!=-1)
           return  true;

    }
    return  false;
}
twoVals termloop::isIterativeVar(Value* var, Loop* l){
    twoVals tv;
    if(Instruction* ins=dyn_cast<Instruction>(var)){
        if(!l->contains(ins))
            return tv;
    }
    if(SExtInst* sin=dyn_cast<SExtInst>(var))
        return isIterativeVar(sin->getOperand(0),l);
    if(isa<PHINode>(var)){
        tv.iterVal1=var;
        return tv;
    }
    if(ZExtInst* zin=dyn_cast<ZExtInst>(var))
        return  isIterativeVar(zin->getOperand(0),l);
    if(BinaryOperator* bop=dyn_cast<BinaryOperator>(var)){ //for the while(i-=12>=2)
        Value* op1=bop->getOperand(0);
        Value* op2=bop->getOperand(1);
        Instruction::BinaryOps bops=bop->getOpcode();
        twoVals tv1=isIterativeVar(op1,l);
        twoVals tv2=isIterativeVar(op2,l);
        int tv1Type=tv1.getType(), tv2Type=tv2.getType();  //-1: no iterval, 0: x-y  ,1 :x, 2: x+c
        if(tv1Type==1&&tv2Type==1){
            if(bops!=Instruction::Sub){
                assert(false&&"Unsupport loop form");
            }
            tv.iterVal1=tv1.iterVal1;
            tv.iterVal2=tv2.iterVal1;
            return tv;
        }
        if(tv1Type==1&&tv2Type==-1){
            tv.iterVal1=tv1.iterVal1;
            tv.c=tv2.c;
            tv.posOrNeg=bops;
            return tv;
        }
        if(tv1Type==0&&tv2Type==-1){
            tv.iterVal1=tv1.iterVal1;
            tv.iterVal2=tv1.iterVal2;
            tv.c=tv2.c;
            tv.posOrNeg=bops;
            return tv;
        }
        if(tv1Type==-1&&tv2Type==-1){
            tv.c=var;
            return tv;
        }
        errs()<<tv1Type<<"   "<<tv2Type<<"  \m";
        assert(false&&"Unsupport loop form");
    }
    if(GetElementPtrInst* getEle=dyn_cast<GetElementPtrInst>(var)){ //char  *-3 or
        Value* base=getEle->getOperand(0);
        twoVals tv1=isIterativeVar(base,l);
        if(tv1.getType()==1&&!isAlliterator(gep_type_begin(getEle), gep_type_end(getEle),l)){
            tv.iterVal1=base;
            return tv;
        }
    }
    tv.c=var;
        return tv;
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
    twoVals tv1=isIterativeVar(v1,l);
    twoVals tv2=isIterativeVar(v2,l);
    int type1=tv1.getType(), type2=tv2.getType();  //-1: no iterval, 0: x-y  ,1 :x, 2: x+c
    cmpSymbol  cm;

    if(type1==1&&type2==-1){//x~c
        cond.controlVar=tv1.iterVal1;
        cond.c=tv2.c;
    }else if(type1==1&&type2==2){//x~y+c
        cond.controlVar=tv1.iterVal1;
        cond.controlVar2=tv2.iterVal1;
        cond.c=tv2.c;
        cond.cPosOrNeg=tv2.posOrNeg;
    }else if(type1==0&&type2==-1){//x-y~c
        cond.controlVar=tv1.iterVal1;
        cond.controlVar2=tv1.iterVal2;
        cond.c=tv2.c;
        cond.cPosOrNeg=Instruction::Add;
    }else{
        assert(false&&"Unsupport loop form");
    }
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
    return cond;

}

