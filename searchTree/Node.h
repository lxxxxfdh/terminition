//
// Created by xie on 11/27/15.
//

#ifndef TERMINITION_NODE_H
#define TERMINITION_NODE_H

#include "Common.h"
#include <iostream>
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include <vector>
#include <llvm/IR/LLVMContext.h>

using namespace llvm;
using namespace termloop;
using namespace std;
namespace termloop {

    class Path {
    public:
        bool pathAbove;
        varIncrease con_var1;
        varIncrease con_var2;
        monotonicity increase;
        satiType initialSat;
        Value *b;
        //Value *con_var;
        //Value *con_var2;
        Value* c1;
        cmpSymbol cmp;
        Loop* l;
        static DataLayout* DT;
        int step;
        monotoneType  monotype;
        bool validForm;
        vector<BasicBlock *> edges;

        Path(vector<BasicBlock *> *vb, Value *var, Value* var2, Loop* loop) : edges(*vb), l(loop) {
            con_var1.var=var;
            con_var2.var=var2;
            step = UNOWN;
            increase = unknown;
            initialSat = nonfixed;
            validForm=true;
            monotype=Irregular;

        }

        Path(Value *var, cmpSymbol sym, vector<BasicBlock *> *vb,Loop* loop) : cmp(sym), edges(*vb),l(loop) {
            con_var1.var=var;
            step = UNOWN;
            increase = unknown;
            initialSat = nonfixed;
            validForm=true;
            monotype=Irregular;
        }

        BasicBlock *getPrev(BasicBlock *bb) {

            for (int i = 0; i < edges.size(); i++) {
                if (edges[i] == bb && i > 0)
                    return edges[i - 1];

            }

            if (bb == edges[0])
                return edges.back();
            return nullptr;
        }

        Value *getValueForPhi(PHINode *p) {
           // errs()<<*p;
            BasicBlock *b = getPrev(p->getParent());

            if (p->getBasicBlockIndex(b) != -1)
                return p->getIncomingValueForBlock(b);
            return nullptr;
        }

        bool isInPath(BasicBlock *b) {
            for (vector<BasicBlock *>::iterator it = edges.begin(), ed = edges.end(); it != ed; it++) {
                BasicBlock *bb = *it;
                if (bb == b)
                    return true;
            }
            return false;
        }






        //find the x~c1
        void branConditionHandle(vector<BasicBlock *> *outBlocks, Value* conv,Value* convarValue) {


            ICmpInst* icmp=nullptr;
            int tag;
            for(vector<BasicBlock*>::iterator it=edges.begin();it!=edges.end();it++){
                BasicBlock* bb=*it;
                vector<BasicBlock*>::iterator it1;
                if(isInVector(outBlocks,bb))
                    continue;
                TerminatorInst* term=bb->getTerminator();
                //errs()<<*bb;
                if(BranchInst* br=dyn_cast<BranchInst>(term)){
                    if(br->isUnconditional()) continue;

                    if(icmp!= nullptr){ //"Multiple branches"
                        return;
                       // assert(false&&"Multiple branches");
                    }

                    BasicBlock* dest = br->getSuccessor(0);
                    it++;
                    assert(it!=edges.end());
                    tag= dest==*it? 0:1;
                    it--;
                    //errs()<<*br;
                    if(icmp=dyn_cast<ICmpInst>(br->getOperand(0))){
                        condition con=checkCond(icmp,tag,l);

                        //assert(con_var==con.controlVar);
                        if(conv!=con.controlVar||con.isEmpty()){

                            //errs()<<*con_var<<"  sss   "<<con.controlVar;
                            //con_var=nullptr;
                           // c1= nullptr;
                            validForm=false;

                        }else{
                            c1=con.c;
                            cmp=con.sym;
                            switch(con.sym){
                                case other:
                                    //assert(false);
                                    break;
                                case gt:
                                case get:
                                    pathAbove=false;
                                    break;
                                case lt:
                                case let:
                                    pathAbove=true;
                                    break;

                            }
                            //errs()<<*cv<<"\r\n"<<*con.c;
                            ConstantInt *initial=dyn_cast<ConstantInt>(convarValue);

                            //errs()<<*initial<<"!!!!!!!!!!!!!!!!";

                            ConstantInt *cc1=dyn_cast<ConstantInt>(con.c);
                            if(initial!=nullptr&& cc1!=nullptr&&cmp!=other)
                                initialSat=symbolCmp(initial->getZExtValue(),cc1->getZExtValue(),cmp)? satisfied:unsatisfied;
                        }


                    } else
                        assert(false);
                } else
                    assert(false);
            }

        }
        static int getOffsetForGep(gep_type_iterator I,  gep_type_iterator E)
        {
            assert(DT!= nullptr);
            int num=0;
            for(; I != E; ++I)
            {
                if(StructType *STy = dyn_cast<StructType>(*I))
                {
                    assert(false);   //no change and support now
                    const StructLayout *SLO = DT->getStructLayout(STy);
                    const ConstantInt *CPU = cast<ConstantInt>(I.getOperand());
                    unsigned Index = unsigned(CPU->getZExtValue());
                    num += SLO->getElementOffset(Index);
                }
                else
                {
                    SequentialType *ST = cast<SequentialType>(*I);
                    // Get the index number for the array... which must be long type...
                    //GenericValue IdxGV = getOperandValue(I.getOperand(), SF);
                    Value* IdxGV = I.getOperand();
                    // int64_t Idx;
                    // Total += TD.getTypeAllocSize(ST->getElementType())*Idx;
                    ConstantInt *AllocSize = ConstantInt::get(IntegerType::getInt32Ty(getGlobalContext()), DT->getTypeAllocSize(ST->getElementType()));
                    //Value* temp = SemanticValue::tryCal(Instruction::Mul, AllocSize, IdxGV, unkownlist);
                    //Total = SemanticValue:: tryCal(Instruction::Add, Total, temp, unkownlist);
                    int nnn=isConstantValue(IdxGV);
                    if(nnn==INT32_MIN)
                        return UNOWN;
                    num+= (AllocSize->getZExtValue())*nnn;
                }
                return num;
            }
        }

        void computeIncrease(vector<BasicBlock *> *outBlocks, varIncrease* varInc, Value* convar) {  //(outBlocks,con_var,convar_value);


           // branConditionHandle(outBlocks,convar);
            //errs()<<*con_var;
            if (PHINode *phi = dyn_cast<PHINode>(varInc->var)) {
                Value *v = getValueForPhi(phi);
                assert(v != nullptr);
                int change = 0;
                int isMul = 0; //0 add, 1 mul, 2 irreg
                int iregV=0;

                while (true) {

                  // errs() << *v << "\r\n";
                  //  errs()<<*con_var<<"\r\n";
                    if (v == varInc->var) {

                        if (isMul==1) {
                            //not consider the mul change symbol
                            varInc->increase=unknown;
                            if(ConstantInt* cons=dyn_cast<ConstantInt>(convar)){
                                varInc->varValue=cons->getZExtValue();
                                if(varInc->varValue==0||change==0)
                                    varInc->increase=constant;
                                else{
                                    if(varInc->varValue>0&&change>0)
                                        varInc->increase=increasing;
                                    if(varInc->varValue<0&&change>0)
                                        varInc->increase=decreasing;

                                }

                            }
                            varInc->step=change;
                            varInc->monoType=geometric;
                            break;
                        } else if(isMul==0){

                            varInc->increase = change > 0 ? increasing : decreasing;
                            varInc->step = abs(change);
                            varInc->monoType=arithmetic;
                            break;

                        }else{
                            varInc->increase=unknown;
                            if(ConstantInt* cons=dyn_cast<ConstantInt>(convar)){
                                varInc->varValue=cons->getZExtValue();
                                if(change>0){
                                    int var2=varInc->varValue*change+iregV;
                                    if(var2>varInc->varValue) varInc->increase=increasing;
                                    else if(var2==varInc->varValue) varInc->increase=constant;
                                    else varInc->increase=decreasing;
                                    /*
                                    if(varInc->varValue>0&&iregV>0)
                                        varInc->increase=increasing;
                                    if(varInc->varValue>0&&iregV<0||varInc->varValue<0&&iregV>0){

                                    }
                                    if(varInc->varValue<0&&iregV<0)
                                        varInc->increase=decreasing;*/
                                }



                            }
                            varInc->step=change;
                            varInc->v=iregV;
                            varInc->monoType=Irregular;
                            break;

                        }

                    }
                    if (GetElementPtrInst* getEle=dyn_cast<GetElementPtrInst>(v)){
                        int changeVal=getOffsetForGep( gep_type_begin(getEle), gep_type_end(getEle));
                        if(changeVal!=UNOWN){
                            v=getEle->getPointerOperand();
                            change=changeVal;
                        }else{
                            varInc->step=UNOWN;
                            varInc->increase=unknown;
                            break;
                        }


                    }else  if (BinaryOperator *bop = dyn_cast<BinaryOperator>(v)) {

                        Value *v1 = bop->getOperand(0);
                        Value *v2 = bop->getOperand(1);
                        Value *tempVal = nullptr;
                        int temp = 0;
                        Value *newVar = nullptr;
                        bool constT = false;
                        if (ConstantInt *cos1 = dyn_cast<ConstantInt>(v1)) {
                            temp = cos1->getZExtValue();
                            newVar = v2;
                            constT = true;
                        }
                        else if (ConstantInt *cos2 = dyn_cast<ConstantInt>(v2)) {
                            temp = cos2->getZExtValue();
                            newVar = v1;
                            constT = true;
                        } else{
                            int re=isConstantValue(v1);
                            if (re!=INT32_MIN) {
                                temp = re;
                                newVar = v2;
                                constT = true;
                            } else if ((re=isConstantValue(v2))!=INT32_MIN) {
                                temp = re;
                                newVar = v1;
                                constT = true;
                            }
                        }


                        v = newVar;
                        switch (bop->getOpcode()) {
                            case Instruction::Add: {
                                if (constT) {
                                    change += temp;
                                }
                                else {
                                    //errs()<<*cv;
                                    assert(false);
                                }
                                break;
                            }

                            case Instruction::Sub: {
                                if (constT)
                                    change -= temp;
                                else {
                                    assert(false);
                                }
                                break;
                            }
                            case Instruction::Mul:{
                                if (constT&&change ==0){
                                    isMul=1;
                                    change=temp;
                                }else {
                                    //errs()<<*cv;
                                    iregV=change;
                                    change=temp;
                                    isMul=2;

                                }
                                break;
                            }
                            default:

                                assert(false);

                        }
                    }else if(PHINode* ph=dyn_cast<PHINode>(v)){
                        v=getValueForPhi(ph);
                        assert(v!= nullptr);
                    }else if(ConstantInt* cs=dyn_cast<ConstantInt>(v)){
                        b=cs;
                        varInc->increase=constant;
                        break;
                    }else if(TruncInst* trunc=dyn_cast<TruncInst>(v)){
                        v=trunc->getOperand(0);
                    }else if(SExtInst* sest=dyn_cast<SExtInst>(v)){
                        v=sest->getOperand(0);
                    }
                }


            } else
                assert(false);

        }

        void dump(){
            errs()<<"Above: "<<(bool)pathAbove<<"\r\n";
            errs()<<"Condition: "<< con_var1.var->getName()<<"  "<<cmp<<"  "<<*c1<<"\r\n";
            errs()<<"Increase: "<<(monotonicity)con_var1.increase<<"\r\n";
            errs()<<"b: "<<*b<<"\r\n";
        }
    };


    class Node {
    public:
        nodeType type;
        static Value *con_var;
        static Value *con_var2;
        static Value* convar2_value;
        static Value *convar_value;  //control variable value
        static Value *c;
        static Instruction::BinaryOps cPosOrNeg;
        static Loop *l;
        //Value* b;
        static bool controlAbove;
        static cmpSymbol cmp;
        static vector<BasicBlock *> *outBlocks;

        static vector<Path *> *paths;
        Result term;
        vector<Node *> next;
        int result;  //if this is a result node, the result is recroded.


        Node(nodeType t) : type(t) { term = Result::nosupport; result=UNOWN; }
        Node(nodeType t, int res) : type(t), result(res) { term = Result::nosupport; }

        Node *(*chooseNext)(Node *);

        static void monotonicCompute() {
            for (vector<Path *>::iterator it = paths->begin(); it != paths->end(); it++) {
                Path *p = *it;
                if(p->con_var1.increase==unknown) {
                    p->computeIncrease(outBlocks, &p->con_var1, Node::convar_value);
                    // the default increase
                    p->increase = p->con_var1.increase;
                    p->step = p->con_var1.step;
                    p->monotype = p->con_var1.monoType;

                    p->branConditionHandle(outBlocks, con_var, convar_value);
                }
                //p->dump();


                if(Node::con_var2!=nullptr&&p->con_var2.increase==unknown)
                    p->computeIncrease(outBlocks,&p->con_var2,Node::convar2_value);


            }

        }

    };

    Node *constructTree();
}

#endif //TERMINITION_NODE_H
