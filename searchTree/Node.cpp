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
    assert(Node::paths->size()==1);
    Path* pt=*Node::paths->begin();
    if(pt->increase==increasing)
        return pa->next[0];
    else if(pt->increase==decreasing)
        return pa->next[1];
    else
        assert(false);
}



Node* termloop::constructTree(){
    Node*root= new Node(nodeType::pathnum);
    Node*singlePath= new Node(nodeType::controlabove);
    Node*twicePath=new Node(nodeType::monotonic);
    Node*unknownPath=new Node(nodeType::unsupport);
    root->next.push_back(singlePath);
    root->next.push_back(twicePath);
    root->next.push_back(unknownPath);
    root->chooseNext=pathNumSplit;
    singlePath->chooseNext=singlePathSplit;




    Node *singleAbove=new Node(nodeType::monotonic);
    Node *singleBelow=new Node(nodeType::monotonic);
    singlePath->next.push_back(singleAbove);
    singlePath->next.push_back(singleBelow);
    singleAbove->chooseNext=singleAboveBelowSplit;
    singleBelow->chooseNext=singleAboveBelowSplit;



    Node *aboveIncrease=new Node(nodeType::result);
    Node *aboveDecrease=new Node(nodeType::result);
    Node *belowIncrease=new Node(nodeType::result);
    Node *belowDecrease=new Node(nodeType::result);
    singleAbove->next.push_back(aboveIncrease);
    singleAbove->next.push_back(aboveDecrease);
    singleBelow->next.push_back(belowIncrease);
    singleBelow->next.push_back(belowDecrease);

    aboveIncrease->chooseNext=termOutput;
    aboveDecrease->chooseNext=nonTermOutput;
    belowDecrease->chooseNext=termOutput;
    belowIncrease->chooseNext=nonTermOutput;


    return root;
}