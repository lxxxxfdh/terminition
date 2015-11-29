//
// Created by xie on 11/27/15.
//
#include <iostream>
#include "termLoopPass.h"
#include "searchTree/Node.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include <stack>
using namespace llvm;
using namespace termloop;
using namespace std;

namespace llvm
{
    void initializeLoopTermAnalysisPass(PassRegistry &Registry);
}


namespace termloop {

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
    bool isIterativeVar(Value* var){
        if(isa<PHINode>(var))
            return true;
        else
            return false;
    }



    //find the condition with the form x~c ~ is {<,<=,>,>=}
    //tag: 0 normal, 1 reverse
    //coarse implementation
    struct condition checkCond(ICmpInst* inst, int tag){
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
    // Analysis the unique exit condition
    // True for the matched form, other false
    bool controlVarAnalysis(Loop* l){
        SmallVector<BasicBlock*, 20> exitBlocks;
        l->getExitBlocks(exitBlocks);
        BasicBlock *entry=l->getLoopPreheader();


        //vector<BasicBlock*> breakBlocks;
        for(SmallVector<BasicBlock*, 20>::iterator it1=exitBlocks.begin();it1!=exitBlocks.end();it1++){
            BasicBlock* bb=*it1;
            for(pred_iterator it=pred_begin(bb); it!=pred_end(bb);it++){
                BasicBlock* re=*it;
                if(l->contains(re))
                    Node::outBlocks->push_back(re);

            }
        }
        assert(Node::outBlocks->size()==1&&"Unsupport loop && Multiple exit condition");

        BasicBlock* bb=Node::outBlocks->front();
        //errs()<<*bb;
        TerminatorInst* term=bb->getTerminator();
        if(BranchInst* br=dyn_cast<BranchInst>(term)){
            if(br->isUnconditional()) return false;
            BasicBlock* dest = br->getSuccessor(0);
            int tag= l->contains(dest)? 0:1;
            if(ICmpInst* icmp=dyn_cast<ICmpInst>(br->getOperand(0))){
                condition con=checkCond(icmp,tag);
               // con.output();
                switch(con.sym){
                    case other:
                        return false;
                    case gt:
                    case get:
                        Node::controlAbove=false;
                        break;
                    case lt:
                    case let:
                        Node::controlAbove=true;
                        break;

                }
                Node::con_var = con.controlVar;
                Node::c = con.c;
                Node::cmp=con.sym;
                PHINode* phi=dyn_cast<PHINode>(con.controlVar);
                int i=phi->getBasicBlockIndex(entry);
                assert(i!=-1&&"The initial value is not fixed");
                Node::convar_value=phi->getIncomingValue(i);
                assert(isa<Constant>(Node::convar_value)&&"Initial value of control variable is not constant");
                return true;

            }else
                return false;



        }
        return false;
    }
    //Compute path number in the loop
    void pathnumHandle(Loop* l){
        if(!controlVarAnalysis(l))
            assert(false&&"Unsupport loop&& Guard form not match");
        BasicBlock* header=l->getHeader();
        stack<BasicBlock*> trave;
        stack<vector<BasicBlock*>*> paths;
        trave.push(header);
        vector<BasicBlock*> first;
        first.push_back(header);
        paths.push(&first);
        vector<BasicBlock*> already;
        already.push_back(header);

        while(!trave.empty()){
            vector<BasicBlock*> *path=paths.top();
            BasicBlock* bb=trave.top();
            trave.pop();
            paths.pop();


            bool hasChildren=false;
            for(succ_iterator it=succ_begin(bb);it!=succ_end(bb);it++){
                BasicBlock* bl=*it;
               // errs()<<*bl<<"\r\n";
                if(!l->contains(bl)||isInVector(&already,bl))
                    continue;
                vector<BasicBlock*> *tmp=new vector<BasicBlock*>();
                //errs()<<path->begin()-path->end();
                std::copy(path->begin(),path->end(), std::back_inserter(*tmp));
                tmp->push_back(bl);
                hasChildren=true;
                already.push_back(bl);
                trave.push(bl);

                paths.push(tmp);

            }
            if(!hasChildren){// the leafnode

                Path* pa=new Path(path,Node::con_var);

                Node::paths->push_back(pa);
            }
        }


    };
    void traverse(Node* root, Loop* l){
        if(root== nullptr)
            return;
        if(root->type==pathnum){
            pathnumHandle(l);
           // cout<<"Num:  "<<Node::paths->size()<<endl;

        }else if(root->type==unsupport){
            cout<<"Not Support Loop\r\n";
            return;
        }else if(root->type==controlabove){

        }else if(root->type==monotonic){
            Node::monotonicCompute();
        }

        Node* next=root->chooseNext(root);
        traverse(next,l);

    };

    class LoopTermAnalysis final : public FunctionPass {
    public:
        static char ID;

        LoopTermAnalysis() : FunctionPass(ID) {
            initializeLoopTermAnalysisPass(*PassRegistry::getPassRegistry());

        }

        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.setPreservesCFG();
            // AU.addRequired<DominatorTreeWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();

        }

        bool runOnFunction(Function &f) override {
            LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();//.getLoopInfo();
            Node* root=constructTree();
            for(LoopInfo::iterator I = LI->begin(), E = LI->end(); I != E; ++I)
            {
                Node::paths->clear();
                Node::c= nullptr;
                Node::con_var=nullptr;
                Node::outBlocks->clear();
                Loop *l = *I;
                Node::l=l;

                traverse(root,l);
            }
            return true;

        }


    };

}
char termloop::LoopTermAnalysis::ID = 0;

INITIALIZE_PASS_BEGIN(LoopTermAnalysis, "looppass", "looptest", false, false)
    //  INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
    // INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
    INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(LoopTermAnalysis, "looppass", "looptest", false, false)

llvm::FunctionPass *termloop::createLoopPass()
{
    return new LoopTermAnalysis();
}