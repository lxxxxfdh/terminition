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
//0 : both inc or dec  1: increasing and constant
int adjustPostion(){
    vector<Path*>::iterator it=Node::paths->begin();
    it++;
    Path* p1=*Node::paths->begin();
    Path* p2=*it;
    if((p1->increase==increasing&&p2->increase==increasing)||(p1->increase==decreasing&&p2->increase==decreasing))
        return 0;
    if(p1->increase==increasing&&p2->increase==constant)
        return 1;
    if(p1->increase==constant&&p2->increase==increasing) {
        iter_swap(Node::paths->begin(), Node::paths->begin() + 1);
        return 1;
    }

}
Node* notImplement(Node* pa){
    errs()<<"Not implemented\r\n";
    return nullptr;
}
Node* twoPathSplit(Node* pa){
    int tag=adjustPostion();

    switch (tag){
        case 0:
            return pa->next[0];
        case 1:
            return pa->next[1];
        default:
            assert(false);
    }


}



Node* termloop::constructTree(){
    Node*root= new Node(nodeType::pathnum);
    Node*singlePath= new Node(nodeType::controlabove);
    Node*twoPath=new Node(nodeType::monotonic);
    Node*unknownPath=new Node(nodeType::unsupport);
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
    twoPath->next.push_back(singlePath);
    twoPath->next.push_back(incAndCons);
    twoPath->chooseNext=twoPathSplit;
    incAndCons->chooseNext=notImplement;   //not




    Node *aboveIncrease=new Node(nodeType::result);
    Node *aboveDecrease=new Node(nodeType::result);
    Node *belowIncrease=new Node(nodeType::result);
    Node *belowDecrease=new Node(nodeType::result);
    singleAbove->next.push_back(aboveIncrease);
    singleAbove->next.push_back(aboveDecrease);
    singleBelow->next.push_back(belowIncrease);
    singleBelow->next.push_back(belowDecrease);
    singleAbove->chooseNext=singleAboveBelowSplit;
    singleBelow->chooseNext=singleAboveBelowSplit;

    aboveIncrease->chooseNext=termOutput;
    aboveDecrease->chooseNext=nonTermOutput;
    belowDecrease->chooseNext=termOutput;
    belowIncrease->chooseNext=nonTermOutput;


    return root;
}