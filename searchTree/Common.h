//
// Created by xie on 11/27/15.
//

#ifndef TERMINITION_COMMON_H
#define TERMINITION_COMMON_H
#include <vector>
#include <algorithm>
using namespace std;
namespace termloop {
    const int UNOWN=-99999;

    enum nodeType {
        pathnum, monotonic, controlabove, result, unsupport
    };
    enum monotonicity {
        increasing, decreasing, constant, unknown
    };
    enum cmpSymbol {
        get=0, gt,let,lt,other
    };
    enum Result{
        termination, nontermination, nosupport
    };

    template<class T>
    bool isInVector(vector<T*>*vb,T* ins){

        if(std::find(vb->begin(),vb->end(),ins)!=vb->end())
            return true;
        return false;

    };

}
#endif //TERMINITION_COMMON_H
