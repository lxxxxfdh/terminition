//
// Created by xie on 11/27/15.
//

#include "Node.h"
using namespace termloop;

vector<Path*>* termloop::Node::paths=new vector<Path*>();
vector<BasicBlock*>* termloop::Node::outBlocks=new vector<BasicBlock*>();
Value* termloop::Node::con_var=nullptr;
Value* termloop::Node::c= nullptr;
Value* Node::convar_value=nullptr;
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
Node* singleAboveBelowSplit(Node* pa){
  //  assert(Node::paths->size()==1);
    Path* pt=*Node::paths->begin();
    if(pt->increase==increasing)
        return pa->next[0];
    else if(pt->increase==decreasing)
        return pa->next[1];
    else
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
int incStep(int x, int step, int c1){
    return c1+step-(c1-x)%step;
}
int decStep(int x, int step, int c1){
    return c1-step+(x-c1)%step;
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
                    if(incStep(convar,p1->step,cc1)<=cc){
                        if(bb<=cc){
                            if(bb>cc1||incStep(bb,p1->step,cc1)<=cc)
                                return pa->next[1];
                            else
                                return pa->next[0];
                        }else
                            return pa->next[0];

                    } else
                        return pa->next[0];

                }else{
                    if(bb<=cc){
                        if(bb>cc1||incStep(bb,p1->step,cc1)<=cc)
                            return pa->next[1];

                        else
                            return pa->next[0];


                    }else
                        return pa->next[0];

                }

            }else{ //x<c, x<c1 f1 <-
                assert(b!= nullptr&&c!= nullptr&&cons_var!= nullptr);
                if(p1->initialSat==satisfied||b->getZExtValue()<=c->getZExtValue())
                    return pa->next[1];
                else
                    return pa->next[0];



            }
        }else{
            if(p1->increase==increasing){  //x<c, x>c1 f1->
                assert(b!= nullptr&&c!= nullptr&&c1!=nullptr);
                if(p1->initialSat==unsatisfied&&b<=c&&b<c1)
                    return pa->next[0];
                else
                    return pa->next[1];

            }else{ //x<c, x>c1 f1 <-
                assert(b!= nullptr&&c!= nullptr);
                if(b->getZExtValue()<=c->getZExtValue())
                    return pa->next[1];
                else
                    return pa->next[0];

            }

        }

    }else{
        if(p1->pathAbove){
            if(p1->increase==increasing){ //x>c, x<c1 f1 ->
                assert(b!= nullptr&&c!= nullptr);
                if(b->getZExtValue()>=c->getZExtValue())
                    return pa->next[1];
                else
                    return pa->next[0];

            }else{ //x>c, x<c1 f1 <-
                assert(b!= nullptr&&c!= nullptr&&c1!=nullptr);
                if(p1->initialSat==unsatisfied&&b>=c&&b>c1)
                    return pa->next[0];
                else
                    return pa->next[1];


            }
        }else{
            if(p1->increase==increasing){ //x>c, x>c1 f1 ->
                assert(b!= nullptr&&c!= nullptr&&cons_var!= nullptr);
                if(p1->initialSat==satisfied||b->getZExtValue()>=c->getZExtValue())
                    return pa->next[1];
                else
                    return pa->next[0];

            }else{  //x>c, x>c1 f1 <-
                assert(b!= nullptr&&c!= nullptr&&c1!=nullptr&&cons_var!= nullptr);
                int bb=b->getZExtValue(), cc=c->getZExtValue(),cc1=c1->getZExtValue(),convar=cons_var->getZExtValue();
                if(p1->initialSat==satisfied){//x|=B
                    if(decStep(convar,p1->step,cc1)>=cc){
                        if(bb>=cc){
                            if(bb<cc1||decStep(bb,p1->step,cc1)>=cc)
                                return pa->next[1];
                            else
                                return pa->next[0];
                        }else
                            return pa->next[0];

                    } else
                        return pa->next[0];

                }else{
                    if(bb<=cc){
                        if(bb<cc1||decStep(bb,p1->step,cc1)>=cc)
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

Node* incAndDecSplit(Node* pa){
    vector<Path*>::iterator it=Node::paths->begin();
    it++;
    Path* p1=*Node::paths->begin();
    Path* p2=*it;
    //errs()<<*Node::c<<"\r\n"<<*p1->c1<<"\r\n"<<*Node::con_var<<"\r\n"<<*p1->b;
    ConstantInt* c=dyn_cast<ConstantInt>(Node::c);
    ConstantInt* c1=dyn_cast<ConstantInt>(p1->c1);
    ConstantInt* cons_var=dyn_cast<ConstantInt>(Node::convar_value);
    if(Node::controlAbove){
        if(p1->pathAbove){
            assert(c!= nullptr&&c1!=nullptr&&cons_var!= nullptr);
            int cc=c->getZExtValue();
            int cc1=c->getZExtValue();
            if(cc1<=cc&&((cc-cc1+1)>=p1->step))
                return pa->next[1];
            else
                return pa->next[0];

        }else{
            if(p1->initialSat!=satisfied)
                return pa->next[1];
            else
                return pa->next[0];


        }

    }else{
        if(p1->pathAbove){
            if(p1->initialSat==satisfied)
                return pa->next[1];
            else
                return pa->next[0];
        }else{
            assert(c!= nullptr&&c1!=nullptr&&cons_var!= nullptr);
            int cc=c->getZExtValue();
            int cc1=c->getZExtValue();
            if(cc1>=cc&&((cc1-cc+1)>=p2->step))
                return pa->next[1];
            else
                return pa->next[0];


        }
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
    if(Node::controlAbove){
        if(p1->pathAbove){
            if(p1->initialSat==satisfied){
                if(bb1<=cc&&(bb1<=cc1||bb2<=cc))
                    return pa->next[1];
                else
                    return pa->next[0];
            }else{
                if(bb2<=cc&&(bb2>cc1||bb1<=cc))
                    return pa->next[1];
                return pa->next[0];

            }

        }else{
            if(p1->initialSat==satisfied){
                if(bb1<=cc&&(bb1>=cc1||bb2<=cc))
                    return pa->next[1];
                else
                    return pa->next[0];
            }else{
                if(bb2<=cc&&(bb2<cc1||bb1<=cc))
                    return pa->next[1];
                return pa->next[0];

            }


        }

    }else{
        if(p1->pathAbove){
            if(p1->initialSat==satisfied){
                if(bb1>=cc&&(bb1<=cc1||bb2>=cc))
                    return pa->next[1];
                else
                    return pa->next[0];
            }else{
                if(bb2>=cc&&(bb2>cc1||bb1>=cc))
                    return pa->next[1];
                return pa->next[0];

            }
        }else{
            if(p1->initialSat==satisfied){
                if(bb1>=cc&&(bb1>=cc1||bb2>=cc))
                    return pa->next[1];
                else
                    return pa->next[0];
            }else{
                if(bb2>=cc&&(bb2<cc1||bb1>=cc))
                    return pa->next[1];
                return pa->next[0];

            }


        }
    }

}
Node* termloop::constructTree(){

    Node*root= new Node(nodeType::pathnum);
    Node*singlePath= new Node(nodeType::controlabove);
    Node*twoPath=new Node(nodeType::monotonic);
    Node*unknownPath=new Node(nodeType::unsupport);
    Node* terminate=new Node(nodeType::result);
    Node* unTerminate=new Node(nodeType::result);
    terminate->chooseNext=termOutput;
    unTerminate->chooseNext=nonTermOutput;
    root->next.push_back(singlePath);
    root->next.push_back(twoPath);
    root->next.push_back(unknownPath);
    root->chooseNext=pathNumSplit;



    Node *singleAbove=new Node(nodeType::monotonic);
    Node *singleBelow=new Node(nodeType::monotonic);
    singlePath->next.push_back(singleAbove);
    singlePath->next.push_back(singleBelow);
    singlePath->chooseNext=singlePathSplit;

    Node* incAndCons=new Node(nodeType::empty);
    Node* incAndDec=new Node(nodeType::empty);
    Node* consAndCons=new Node(nodeType::empty);
    twoPath->next.push_back(singlePath);
    twoPath->next.push_back(incAndCons);
    twoPath->next.push_back(incAndDec);
    twoPath->next.push_back(consAndCons);
    twoPath->chooseNext=twoPathSplit;
    incAndCons->chooseNext=incAndConsSplit;
    incAndCons->next.push_back(terminate);
    incAndCons->next.push_back(unTerminate);


    incAndDec->chooseNext=incAndDecSplit;
    incAndDec->next.push_back(terminate);
    incAndDec->next.push_back(unTerminate);

    consAndCons->chooseNext=consAndConsSplit;
    consAndCons->next.push_back(terminate);
    consAndCons->next.push_back(unTerminate);





    singleAbove->next.push_back(terminate);
    singleAbove->next.push_back(unTerminate);
    singleBelow->next.push_back(unTerminate);
    singleBelow->next.push_back(terminate);
    singleAbove->chooseNext=singleAboveBelowSplit;
    singleBelow->chooseNext=singleAboveBelowSplit;

    return root;
}