//
// Created by xie on 11/27/15.
//

#include "Node.h"
using namespace termloop;

vector<Path*>* termloop::Node::paths=new vector<Path*>();
vector<BasicBlock*>* termloop::Node::outBlocks=new vector<BasicBlock*>();
Value* termloop::Node::con_var=nullptr;
Value* termloop::Node::con_var2=nullptr;
Value* termloop::Node::c= nullptr;
Value* Node::convar_value=nullptr;
Value* Node::convar2_value=nullptr;
Instruction::BinaryOps Node::cPosOrNeg=Instruction::Add;
Loop* Node::l=nullptr;
bool termloop::Node::controlAbove=true;
cmpSymbol Node::cmp=other;

// construct rules for all fomulas

Node* pathNumSplit(Node* pa){
    int num=Node::paths->size();
    if(num==1)
        return pa->next[0];
    else if(num==2)
        return pa->next[1];
    else
        return pa->next[2];
}
Node* singlePathSplit(Node* pa){
    assert(Node::cmp!=other&&"Cannot find control variable above or below");
    if(Node::controlAbove)
        return pa->next[0];
    else
        return pa->next[1];
}

Node* termOutput(Node* pa){
    errs()<<"Termination \r\n";
    return nullptr;
}
Node* nonTermOutput(Node* pa){
    errs()<<"Nontermination \r\n";
    return nullptr;
}
Node* unknownTermOut(Node* pa){
    errs()<<"Unknown \r\n";
    return nullptr;
}
Node* singleAboveBelowSplit(Node* pa){
  //  assert(Node::paths->size()==1);
    Path* pt=*Node::paths->begin();
    if(pt->increase==increasing)
        return pa->next[0];
    else if(pt->increase==decreasing)
        return pa->next[1];
    else if(pt->increase==unknown)
        return pa->next[2];
    assert(false);
}
//0 : both inc or dec  1: increasing and constant 2: increasing decreasing 3: constant constant
int adjustPostion(){
    vector<Path*>::iterator it=Node::paths->begin();
    it++;
    Path* p1=*Node::paths->begin();
    Path* p2=*it;
    if((p1->increase==increasing&&p2->increase==increasing)||(p1->increase==decreasing&&p2->increase==decreasing))
        return 0;

    if(!p1->validForm||!p2->validForm)
        return 4;

    if((p1->increase==increasing||p1->increase==decreasing)&&p2->increase==constant)
        return 1;
    if(p1->increase==constant&&(p2->increase==increasing||p2->increase==decreasing)) {
        iter_swap(Node::paths->begin(), Node::paths->begin() + 1);
        return 1;
    }

    if(p1->increase==increasing&&p2->increase==decreasing)
        return 2;
    if(p1->increase==decreasing&&p2->increase==increasing){
        iter_swap(Node::paths->begin(), Node::paths->begin() + 1);
        return 2;
    }
    if(p1->increase==constant&&p2->increase==constant)
        return 3;


}
Node* notImplement(Node* pa){
    errs()<<"Not implemented\r\n";
    return nullptr;
}
Node* multiPaths(Node* pa){

}
Node* multiPathsSplit(Node* pa){
    vector<Path*>::iterator it=Node::paths->begin();
    monotonicity inc=(*it)->increase;
    it++;
    for(;it!=Node::paths->end();it++){
        if((*it)->increase!=inc){
            return pa->next[1];
        }
    }
    return pa->next[0];

    /*switch (tag){
        case 0:
            return pa->next[0];
        case 1:
            return pa->next[1];
        default:
            assert(false);
    }*/

}
Node* twoPathSplit(Node* pa){
    int tag=adjustPostion();
    return pa->next[tag];

    /*switch (tag){
        case 0:
            return pa->next[0];
        case 1:
            return pa->next[1];
        default:
            assert(false);
    }*/

}
int incStep(int x, int step, int c1, cmpSymbol cmp){
    if(cmp==let)
        return c1+step-(c1-x)%step;
    else
        return c1-1+step-(c1-1-x)%step;
}
int decStep(int x, int step, int c1, cmpSymbol cmp){
    if(cmp==cmpSymbol:: get)
        return c1-step+(x-c1)%step;
    else
        return c1+1-step+(x-c1-1)%step;
}

Node* incAndConsSplit(Node* pa){


    vector<Path*>::iterator it=Node::paths->begin();
    it++;
    Path* p1=*Node::paths->begin();
    Path* p2=*it;
 //errs()<<*Node::c<<"\r\n"<<*p1->c1<<"\r\n"<<*Node::con_var<<"\r\n"<<*p1->b;
    ConstantInt* c=dyn_cast<ConstantInt>(Node::c);
    ConstantInt* c1=dyn_cast<ConstantInt>(p1->c1);
    ConstantInt* b=dyn_cast<ConstantInt>(p2->b);
    ConstantInt* cons_var=dyn_cast<ConstantInt>(Node::convar_value);

    if(Node::controlAbove){
        if(p1->pathAbove){
            if(p1->increase==increasing){   //x<c, x<c1 f1 ->
                assert(b!= nullptr&&c!= nullptr&&c1!=nullptr&&cons_var!= nullptr);
                int bb=b->getZExtValue(), cc=c->getZExtValue(),cc1=c1->getZExtValue(),convar=cons_var->getZExtValue();
                if(p1->initialSat==satisfied){//x|=B
                    if(symbolCmp(incStep(convar,p1->step,cc1,p1->cmp),cc,Node::cmp) ){
                        if(symbolCmp(bb,cc,Node::cmp)){
                            if(symbolCmp(bb,cc1,p2->cmp)|| symbolCmp(incStep(bb,p1->step,cc1,p1->cmp),cc,Node::cmp))
                                return pa->next[1];
                            else
                                return pa->next[0];
                        }else
                            return pa->next[0];

                    } else
                        return pa->next[0];

                }else{
                    if(symbolCmp(bb,cc,Node::cmp)){
                        if(symbolCmp(bb,cc1,p2->cmp)||symbolCmp(incStep(bb,p1->step,cc1,p1->cmp),cc,Node::cmp))
                            return pa->next[1];

                        else
                            return pa->next[0];


                    }else
                        return pa->next[0];

                }

            }else{ //x<c, x<c1 f1 <-
                assert(b!= nullptr&&c!= nullptr&&cons_var!= nullptr);
                if(p1->initialSat==satisfied||  symbolCmp(b->getZExtValue(),c->getZExtValue(),Node::cmp))
                    return pa->next[1];
                else
                    return pa->next[0];



            }
        }else{
            if(p1->increase==increasing){  //x<c, x>c1 f1->
                assert(b!= nullptr&&c!= nullptr&&c1!=nullptr);
                if(p1->initialSat==unsatisfied&&symbolCmp(b->getZExtValue(),c->getZExtValue(),Node::cmp)&&symbolCmp(b->getZExtValue(),c->getZExtValue(),p2->cmp))
                    return pa->next[0];
                else
                    return pa->next[1];

            }else{ //x<c, x>c1 f1 <-
                assert(b!= nullptr&&c!= nullptr);
                if(symbolCmp(b->getZExtValue(),c->getZExtValue(),Node::cmp))
                    return pa->next[1];
                else
                    return pa->next[0];

            }

        }

    }else{
        if(p1->pathAbove){
            if(p1->increase==increasing){ //x>c, x<c1 f1 ->
                assert(b!= nullptr&&c!= nullptr);
                if(symbolCmp(b->getZExtValue(),c->getZExtValue(),Node::cmp))
                    return pa->next[1];
                else
                    return pa->next[0];

            }else{ //x>c, x<c1 f1 <-
                assert(b!= nullptr&&c!= nullptr&&c1!=nullptr);
                if(p1->initialSat==unsatisfied&&symbolCmp(b->getZExtValue(),c->getZExtValue(),Node::cmp)&&symbolCmp(b->getZExtValue(),c1->getZExtValue(),p2->cmp))
                    return pa->next[0];
                else
                    return pa->next[1];


            }
        }else{
            if(p1->increase==increasing){ //x>c, x>c1 f1 ->
                assert(b!= nullptr&&c!= nullptr&&cons_var!= nullptr);
                if(p1->initialSat==satisfied||symbolCmp(b->getZExtValue(),c->getZExtValue(),Node::cmp))
                    return pa->next[1];
                else
                    return pa->next[0];

            }else{  //x>c, x>c1 f1 <-
                assert(b!= nullptr&&c!= nullptr&&c1!=nullptr&&cons_var!= nullptr);
                int bb=b->getZExtValue(), cc=c->getZExtValue(),cc1=c1->getZExtValue(),convar=cons_var->getZExtValue();
                if(p1->initialSat==satisfied){//x|=B
                    if(symbolCmp(decStep(convar,p1->step,cc1,p1->cmp),cc,Node::cmp) ){
                        if(symbolCmp(bb,cc,Node::cmp)){
                            if(symbolCmp(bb,cc1,p2->cmp)||symbolCmp(decStep(bb,p1->step,cc1,p1->cmp),cc,Node::cmp))
                                return pa->next[1];
                            else
                                return pa->next[0];
                        }else
                            return pa->next[0];

                    } else
                        return pa->next[0];

                }else{
                    if(symbolCmp(bb,cc,Node::cmp)){
                        if(symbolCmp(bb,cc1,p2->cmp)||symbolCmp(decStep(bb,p1->step,cc1,p1->cmp),cc,Node::cmp))

                            return pa->next[1];

                        else
                            return pa->next[0];


                    }else
                        return pa->next[0];

                }

            }

        }

    }

}


bool interleaving(int x, int inc, int dec, cmpSymbol guard, int c, cmpSymbol ifguard, int c1){
    vector<int> traversed;
    while(true){
        if(!symbolCmp(x,c,guard))
            return true;
        if(isInVector(&traversed,x))
            return false;
        traversed.push_back(x);
        if(symbolCmp(x,c1,ifguard))
            x=incStep(x,inc,c1,ifguard);
        else
            x=decStep(x,dec,c1,(cmpSymbol)(3-ifguard));
    }

}
Node* incAndDecSplit(Node* pa){
    vector<Path*>::iterator it=Node::paths->begin();
    it++;
    Path* p1=*Node::paths->begin();
    Path* p2=*it;
    //errs()<<*Node::c<<"\r\n"<<*p1->c1<<"\r\n"<<*Node::con_var<<"\r\n"<<*p1->b;
    ConstantInt* c=dyn_cast<ConstantInt>(Node::c);
    ConstantInt* c1=dyn_cast<ConstantInt>(p1->c1);
    ConstantInt* cons_var=dyn_cast<ConstantInt>(Node::convar_value);

    if(p1->pathAbove){//x<c x<c1 p1-> p2<-    x>c x<c1 p1-> p2 <-
        assert(c!= nullptr&&c1!=nullptr&&cons_var!= nullptr);
        int cc=c->getZExtValue();
        int cc1=c->getZExtValue();
        int initial=cons_var->getZExtValue();
        bool tag=interleaving(initial,p1->step,p2->step,Node::cmp,cc,p1->cmp,cc1);
        if(tag)
            return pa->next[0];
        else
            return pa->next[1];
    }
    else if(Node::controlAbove){ // x<c x>c1 p1-> p2<-
        if(p1->initialSat!=satisfied)
            return pa->next[1];
        else
            return pa->next[0];
    }else { // x>c x>c1 p1-> p2<-
        if (p1->initialSat == satisfied)
            return pa->next[1];
        else
            return pa->next[0];
    }
}


Node* consAndConsSplit(Node* pa){
    vector<Path*>::iterator it=Node::paths->begin();
    it++;
    Path* p1=*Node::paths->begin();
    Path* p2=*it;
    //errs()<<*Node::c<<"\r\n"<<*p1->c1<<"\r\n"<<*Node::con_var<<"\r\n"<<*p1->b;
    ConstantInt* c=dyn_cast<ConstantInt>(Node::c);
    ConstantInt* c1=dyn_cast<ConstantInt>(p1->c1);
    ConstantInt* cons_var=dyn_cast<ConstantInt>(Node::convar_value);
    ConstantInt* b1=dyn_cast<ConstantInt>(p1->b);
    ConstantInt* b2=dyn_cast<ConstantInt>(p2->b);
    assert(c!= nullptr&&c1!=nullptr&&cons_var!= nullptr&&b1!=nullptr&&b2!= nullptr);
    int cc=c->getZExtValue(),cc1=c1->getZExtValue(),bb1=b1->getZExtValue(),bb2=b2->getZExtValue();
    if((p1->initialSat==satisfied&&symbolCmp(bb1,cc,Node::cmp)&&symbolCmp(bb1,cc1,p1->cmp))||
            (p1->initialSat==unsatisfied&&symbolCmp(bb2,cc,Node::cmp)&&symbolCmp(bb2,cc1,p2->cmp))||
            (symbolCmp(bb1,cc,Node::cmp)&&symbolCmp(bb2,cc,Node::cmp))
            )
                    return pa->next[1];
    return pa->next[0];
}
Node* newSinglSplit(Node* pa){
     if(Node::con_var2!=nullptr)
         return pa->next[0];
     else
         return pa->next[1];
}
bool algo2(int x0,int y0,int u1, int u2,int v, cmpSymbol cmp, int tag){
    int x,y,n=1;

    return true;

}
bool algo1(int x0,int y0,int u1, int u2,int v, cmpSymbol cmp, int tag){
    int x,y,n=1;

    return true;

}

Node* gapSplit(Node* pa){
    Path *p=*Node::paths->begin();
    varIncrease var1=p->con_var1;
    varIncrease var2=p->con_var2;
    monotonicity  mo1=var1.increase;
    monotonicity  mo2=var2.increase;
    monotoneType type1=var1.monoType;
    monotoneType type2=var2.monoType;

    if(mo1==unknown||mo2==unknown)
        p->increase=unknown;
    if(mo1==increasing&&mo2==decreasing){
        p->increase=increasing;

    }

    if(mo1==decreasing&&mo2==increasing){
        p->increase=decreasing;

    }
    if(mo1==increasing&& mo2==increasing){
        if(type1==type2&&(type1==arithmetic||type1==geometric)){
            if(var1.step>var2.step)
                p->increase=increasing;
            if(var1.step==var2.step){
                if(type1==arithmetic) p->increase=constant;
                if(type1==geometric) {
                    if(var1.varValue>var2.varValue) p->increase=increasing;
                    if(var1.varValue==var2.varValue) p->increase=constant;
                    if(var1.varValue<var2.varValue) p->increase=decreasing;
                }
            }
            if(var1.step<var2.step)
                p->increase=decreasing;

        }
        if(type1==arithmetic&&(type2==geometric||type2==Irregular)) p->increase=decreasing;
        if((type1==geometric||type1==Irregular)&&type2==arithmetic) p->increase=increasing;
        if(type1==geometric&&type2==Irregular){
            //Algo2
            bool result=algo2(var1.varValue,var2.varValue,var1.step,var2.step,var2.v,Node::cmp,1);
            if(result)
                return pa->next[1];
            else
                return pa->next[2];
        }
        if(type1==Irregular&&type2==geometric){
            bool result=algo2(var1.varValue,var2.varValue,var1.step,var2.step,var2.v,Node::cmp,1);
            if(result)
                return pa->next[1];
            else
                return pa->next[2];
        }
        if(type1==Irregular&&type2==Irregular){
            //?????????
            assert(false&&"don't know");
        }
    }
    if(mo1==decreasing&& mo2==decreasing){
        if(type1==type2&&(type1==arithmetic||type1==geometric)){
            if(var1.step<var2.step)
                p->increase=increasing;
            if(var1.step==var2.step){
                if(type1==arithmetic) p->increase=constant;
                if(type1==geometric) {
                    if(var1.varValue>var2.varValue) p->increase=increasing;
                    if(var1.varValue==var2.varValue) p->increase=constant;
                    if(var1.varValue<var2.varValue) p->increase=decreasing;
                }
            }
            if(var1.step>var2.step)
                p->increase=decreasing;

        }
        if(type1==arithmetic&&(type2==geometric||type2==Irregular));  //need algo1
        if((type1==geometric||type1==Irregular)&&type2==arithmetic);  //need algo1
        if(type1==geometric&&type2==Irregular){    //need algo2
            //Algo2
            bool result=algo2(var1.varValue,var2.varValue,var1.step,var2.step,var2.v,Node::cmp,1);
            if(result)
                return pa->next[1];
            else
                return pa->next[2];
        }
        if(type1==Irregular&&type2==geometric){  //need algo2
            bool result=algo2(var1.varValue,var2.varValue,var1.step,var2.step,var2.v,Node::cmp,1);
            if(result)
                return pa->next[1];
            else
                return pa->next[2];
        }
        if(type1==Irregular&&type2==Irregular){
            //?????????
            assert(false&&"don't know");
        }

    }
    return pa->next[0];
}
Node* termloop::constructTree(){

    Node*root= new Node(nodeType::pathnum);

    Node* singlePath= new Node(nodeType::monotonic);
    Node*twoPath=new Node(nodeType::monotonic);
    Node* multiPaths=new Node(nodeType::monotonic);
    Node*unsupport=new Node(nodeType::unsupport);

    Node* terminate=new Node(nodeType::result,2);
    Node* unTerminate=new Node(nodeType::result,3);
    Node* unknown=new Node(nodeType::result,0);
    terminate->chooseNext=termOutput;
    unTerminate->chooseNext=nonTermOutput;
    unknown->chooseNext=unknownTermOut;
    root->next.push_back(singlePath);
    root->next.push_back(twoPath);
    root->next.push_back(multiPaths);
    root->chooseNext=pathNumSplit;

    Node*gapfree= new Node(nodeType::controlabove);
    Node* gapinclude=new Node(nodeType::empty);
    singlePath->next.push_back(gapinclude);
    singlePath->next.push_back(gapfree);
    singlePath->chooseNext=newSinglSplit;

    gapinclude->next.push_back(gapfree);
    gapinclude->next.push_back(terminate);
    gapinclude->next.push_back(unTerminate);
    gapinclude->chooseNext=gapSplit;

    Node *singleAbove=new Node(nodeType::monotonic);
    Node *singleBelow=new Node(nodeType::monotonic);
    gapfree->next.push_back(singleAbove);
    gapfree->next.push_back(singleBelow);
    gapfree->chooseNext=singlePathSplit;



    Node* incAndCons=new Node(nodeType::empty);
    Node* incAndDec=new Node(nodeType::empty);
    Node* consAndCons=new Node(nodeType::empty);
    twoPath->next.push_back(singlePath);
    twoPath->next.push_back(incAndCons);
    twoPath->next.push_back(incAndDec);
    twoPath->next.push_back(consAndCons);
    twoPath->next.push_back(unsupport);
    twoPath->chooseNext=twoPathSplit;
    incAndCons->chooseNext=incAndConsSplit;
    incAndCons->next.push_back(terminate);
    incAndCons->next.push_back(unTerminate);

    multiPaths->next.push_back(singlePath);
    multiPaths->next.push_back(unsupport);
    multiPaths->chooseNext=multiPathsSplit;

    incAndDec->chooseNext=incAndDecSplit;
    incAndDec->next.push_back(terminate);
    incAndDec->next.push_back(unTerminate);

    consAndCons->chooseNext=consAndConsSplit;
    consAndCons->next.push_back(terminate);
    consAndCons->next.push_back(unTerminate);





    singleAbove->next.push_back(terminate);
    singleAbove->next.push_back(unTerminate);
    singleAbove->next.push_back(unknown);
    singleBelow->next.push_back(unTerminate);
    singleBelow->next.push_back(terminate);
    singleAbove->chooseNext=singleAboveBelowSplit;
    singleBelow->chooseNext=singleAboveBelowSplit;

    return root;
}