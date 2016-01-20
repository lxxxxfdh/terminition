//
// Created by xie on 11/27/15.
//
#include <iostream>
#include "termLoopPass.h"
#include "searchTree/Node.h"
#include "llvm/IR/LLVMContext.h"
#include <stack>
using namespace llvm;
using namespace termloop;
using namespace std;

namespace llvm
{
    void initializeLoopTermAnalysisPass(PassRegistry &Registry);
}


namespace termloop {


    Value *getInitValFromPhi(Value* ph,BasicBlock *entry){
        if(ph==nullptr)
            return nullptr;
        PHINode* phi=dyn_cast<PHINode>(ph);


        int i=phi->getBasicBlockIndex(entry);
        assert(i!=-1&&"Unspport loop, initial value is not fixed");
        Value * v=phi->getIncomingValue(i);

        // errs()<<*phi;
        if(!isa<Constant>(v)){
            int num=isConstantValue(v);
            if(num!=INT32_MIN){
                return ConstantInt::get(IntegerType::getInt32Ty(getGlobalContext()),num);
            }
        }
        return v;

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
        /*assert(Node::outBlocks->size()==1&&"Unsupport loop && Multiple exit condition");
        if(Node::outBlocks->size()>1){
            errs()<<"Multiple exit condition and only choose the first one, ";
        }*/

        BasicBlock* bb=Node::outBlocks->front();
        //errs()<<*bb;
        TerminatorInst* term=bb->getTerminator();
        if(BranchInst* br=dyn_cast<BranchInst>(term)){
            if(br->isUnconditional()) return false;
            BasicBlock* dest = br->getSuccessor(0);
            int tag= l->contains(dest)? 0:1;
            if(ICmpInst* icmp=dyn_cast<ICmpInst>(br->getOperand(0))){
                //errs()<<*icmp;
                condition con=checkCond(icmp,tag,l);
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
                Node::con_var2=con.controlVar2;
                Node::c = con.c;
                Node::cmp=con.sym;
                Node::cPosOrNeg=con.cPosOrNeg;
                Node::convar_value=getInitValFromPhi(con.controlVar,entry);
                Node::convar2_value=getInitValFromPhi(con.controlVar2,entry);
                //assert(isa<Constant>(Node::convar_value)&&"Initial value of control variable is not constant");
                return true;

            }else
                return false;
        }
        return false;
    }

    //Compute path number in the loop
    void pathnumHandle(Loop* l){
        if(Node::paths->size()>0){

            for(vector<Path *>::iterator it=Node::paths->begin();it!=Node::paths->end();it++){
                Path* p=*it;
                p->con_var1.init(Node::con_var);
                p->con_var2.init(Node::con_var2);

            }
            return;
        }

      //  if(!controlVarAnalysis(l))
     //       assert(false&&"Unsupport loop&& Guard form not match");
        BasicBlock* header=l->getHeader();
        stack<BasicBlock*> trave;
        stack<vector<BasicBlock*>*> paths;
        trave.push(header);
        vector<BasicBlock*> first;
        first.push_back(header);
        paths.push(&first);
      //  vector<BasicBlock*> already;
       // already.push_back(header);

        while(!trave.empty()){
            vector<BasicBlock*> *path=paths.top();
            BasicBlock* bb=trave.top();
            trave.pop();
            paths.pop();


            bool hasChildren=false;
            for(succ_iterator it=succ_begin(bb);it!=succ_end(bb);it++){
                BasicBlock* bl=*it;
               // errs()<<*bl<<"\r\n";
                if(!l->contains(bl)||isInVector(path,bl))
                    continue;
                vector<BasicBlock*> *tmp=new vector<BasicBlock*>();
                //errs()<<path->begin()-path->end();
                std::copy(path->begin(),path->end(), std::back_inserter(*tmp));
                tmp->push_back(bl);
                hasChildren=true;
               // already.push_back(bl);
                trave.push(bl);

                paths.push(tmp);

            }
            if(!hasChildren){// the leafnode

                Path* pa=new Path(path,Node::con_var,Node::con_var2,l);

                Node::paths->push_back(pa);
            }
        }


    };
    int traverse(Node* root, Loop* l){
        if(root->type==result){
            return  root->result;
        }
        if(root== nullptr){
            assert(false&&"error");
            return 1;
        }

        if(root->type==pathnum){
            pathnumHandle(l);
            // cout<<"Num:  "<<Node::paths->size()<<endl;

        }else if(root->type==unsupport){
            cout<<"Not Support Loop\r\n";
            return 1;
        }else if(root->type==controlabove||root->type==empty){

        }else if(root->type==monotonic){
            Node::monotonicCompute();
        }else if(root->type==gaphandle){

        }

        Node* next=root->chooseNext(root);
        traverse(next,l);

    };
//0 unknown, 1 unsupport, 2 termination 3 nonterminitaion
    int getTermiByExitBB(ICmpInst* icmp, Loop* l,Node* root, int tag){


            BasicBlock *entry=l->getLoopPreheader();

                //errs()<<*icmp;
                condition con=checkCond(icmp,tag,l);
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
                Node::con_var2=con.controlVar2;
                Node::c = con.c;
                Node::cmp=con.sym;
                Node::cPosOrNeg=con.cPosOrNeg;
                Node::convar_value=getInitValFromPhi(con.controlVar,entry);
                Node::convar2_value=getInitValFromPhi(con.controlVar2,entry);




                //assert(isa<Constant>(Node::convar_value)&&"Initial value of control variable is not constant");
                return traverse(root,l);


    }
    //0 unknown, 1 unsupport, 2 termination 3 nonterminitaion
    int getResult(Node* root, Loop* l){
        SmallVector<BasicBlock*, 20> exitBlocks;
        l->getExitBlocks(exitBlocks);



        //vector<BasicBlock*> breakBlocks;
        for(SmallVector<BasicBlock*, 20>::iterator it1=exitBlocks.begin();it1!=exitBlocks.end();it1++){
            BasicBlock* bb=*it1;
            for(pred_iterator it=pred_begin(bb); it!=pred_end(bb);it++){
                BasicBlock* re=*it;
                if(l->contains(re))
                    Node::outBlocks->push_back(re);

            }
        }
        /*assert(Node::outBlocks->size()==1&&"Unsupport loop && Multiple exit condition");
        if(Node::outBlocks->size()>1){
            errs()<<"Multiple exit condition and only choose the first one, ";
        }*/

        int re=0; //0 unknown, 1 unsupport, 2 termination 3 nonterminitaion
        //errs()<<*bb;
        BasicBlock* bb=Node::outBlocks->front();
        TerminatorInst* term=bb->getTerminator();

        if(BranchInst* br=dyn_cast<BranchInst>(term)){
            if(br->isUnconditional()) return 1;
            BasicBlock* dest = br->getSuccessor(0);
            int tag= l->contains(dest)? 0:1;
            if(ICmpInst* icmp=dyn_cast<ICmpInst>(br->getOperand(0))){

                return getTermiByExitBB(icmp,l,root,tag);

            }else if(PHINode* phi=dyn_cast<PHINode>(br->getOperand(0))){
                int terms=1;
                for(PHINode::block_iterator it=phi->block_begin(),ed=phi->block_end();it!=ed;it++){
                    BasicBlock* basicBlock=(*it);
                    Value* varl=phi->getIncomingValueForBlock(basicBlock);
                    ICmpInst* cmpinst=nullptr;
                    if(ConstantInt* cons=dyn_cast<ConstantInt>(varl)){
                        int num=cons->getZExtValue();
                        if(num==0){
                            TerminatorInst* term1=basicBlock->getTerminator();
                            if(BranchInst* br1=dyn_cast<BranchInst>(term1)){
                                if(br1->isUnconditional()) return 1;

                                int tag1= br1->getSuccessor(1)==bb? 0:1;
                                tag=tag==tag1?0:1;
                                cmpinst=dyn_cast<ICmpInst>(br1->getOperand(0));

                            }
                        }

                    }else if(ICmpInst* ic=dyn_cast<ICmpInst>(varl)){
                        cmpinst=ic;
                    }
                    //errs()<<*cmpinst<<"   !\n";
                    if(cmpinst!= nullptr){
                        terms=getTermiByExitBB(cmpinst,l,root,tag);
                        if(terms==2)
                            return 2;
                        if(terms==1&&(re==0||re==3))
                            re=1;
                        if(terms==3&&re==0)
                            re=3;
                    }

                }

            }


        } else
            re=1;

        return re;

    }


    class LoopTermAnalysis final : public FunctionPass {
    public:
        static char ID;
        static DataLayout* DT;
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
            int i=1;

            for(LoopInfo::iterator I = LI->begin(), E = LI->end(); I != E; ++I)
            {
                errs()<<"Loop "<<i++<<": ";
                Node::paths->clear();
                Node::c= nullptr;
                Node::con_var=nullptr;
                Node::outBlocks->clear();
                Loop *l = *I;
                Node::l=l;

                //traverse(root,l);
                int re=getResult(root,l);
                switch (re){//0 unknown, 1 unsupport, 2 termination 3 nonterminitaion
                    case  0: errs()<<"  Unknown!\n";
                        break;
                    case 1: errs()<<" UnSupport loop!\n";
                        break;
                    case 2: errs()<<" Termination!\n";
                        break;
                    case 3: errs()<<" NonTermination!\n";
                        break;
                }
            }
            return true;

        }


    };

}
char termloop::LoopTermAnalysis::ID = 0;
DataLayout* LoopTermAnalysis::DT=nullptr;
DataLayout* Path::DT=nullptr;
INITIALIZE_PASS_BEGIN(LoopTermAnalysis, "looppass", "looptest", false, false)
    //  INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
    // INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
    INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(LoopTermAnalysis, "looppass", "looptest", false, false)

llvm::FunctionPass *termloop::createLoopPass()
{
    return new LoopTermAnalysis();
}

void termloop::setDataLa(DataLayout* dt){
    LoopTermAnalysis::DT=dt;
    Path::DT=dt;
}