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

using namespace llvm;
using namespace termloop;
using namespace std;
namespace termloop {

    class Path {
    public:
        bool pathAbove;
        monotonicity increase;
        Value *b;
        Value *con_var;
        cmpSymbol cmp;
        int step;
        vector<BasicBlock *> edges;

        Path(vector<BasicBlock *> *vb, Value *var) : edges(*vb), con_var(var) {
            step = UNOWN;
            increase = unknown;
        }

        Path(Value *var, cmpSymbol sym, vector<BasicBlock *> *vb) : con_var(var), cmp(sym), edges(*vb) {
            step = UNOWN;
            increase = unknown;
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
            BasicBlock *b = getPrev(p->getParent());
            if (p->getBasicBlockIndex(b) != -1)
                return p->getIncomingValueForBlock(b);
            return nullptr;
        }

        bool isInPath(BasicBlock *b, Path *p) {
            for (vector<BasicBlock *>::iterator it = edges.begin(), ed = edges.end(); it != ed; it++) {
                BasicBlock *bb = *it;
                if (bb == b)
                    return true;
            }
            return false;
        }

        bool isConstantValue(Value *v) {

            if (Instruction *inst = dyn_cast<Instruction>(v)) {
                BasicBlock *bb = inst->getParent();
                if (isInPath(bb, this))
                    return false;
            }
            return true;
        }

        void branConditionHandle() {

        }

        void computeIncrease() {
            branConditionHandle();
            if (PHINode *phi = dyn_cast<PHINode>(con_var)) {
                Value *v = getValueForPhi(phi);
                assert(v != nullptr);
                int change = 0;
                bool isMul = false;
                while (true) {
                    if (ConstantInt *con = dyn_cast<ConstantInt>(v)) {
                        assert(false);
                        change = 0;
                        break;
                    }
                   // errs() << *v << "\r\n";
                   // errs()<<*con_var<<"\r\n";
                    if (v == con_var) {
                        if (isMul) {
                            //not consider the mul change symbol
                        } else {
                            step = change;
                            increase = step > 0 ? increasing : decreasing;
                            break;

                        }
                    }
                    if (BinaryOperator *bop = dyn_cast<BinaryOperator>(v)) {
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
                        } else if (isConstantValue(v1)) {
                            tempVal = v1;
                            newVar = v2;
                        } else if (isConstantValue(v2)) {
                            tempVal = v2;
                            newVar = v1;
                        }
                        v = newVar;
                        switch (bop->getOpcode()) {
                            case Instruction::Add: {
                                if (constT) {
                                    change += temp;
                                }
                                else {
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
                        }

                        //errs()<<*var<<"---******----"<<change<<"   \n";

                    }
                }


            } else
                assert(false);

        }

    };

    class Node {
    public:
        nodeType type;
        static Value *con_var;
        static Value *convar_value;
        static Value *c;
        static Value *c1;
        static Loop *l;
        //Value* b;
        static bool controlAbove;
        static cmpSymbol cmp;
        static vector<BasicBlock *> *outBlocks;

        static vector<Path *> *paths;
        Result term;
        vector<Node *> next;

        Node(nodeType t) : type(t) { term = Result::nosupport; }

        Node *(*chooseNext)(Node *);

        static void monotonicCompute() {
            for (vector<Path *>::iterator it = paths->begin(); it != paths->end(); it++) {
                Path *p = *it;
                p->computeIncrease();
            }
        }

    };

    Node *constructTree();
}

#endif //TERMINITION_NODE_H
