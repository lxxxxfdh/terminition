//
// Created by xie on 11/27/15.
//

#ifndef TERMINITION_TERMLOOPPASS_H
#define TERMINITION_TERMLOOPPASS_H
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopInfoImpl.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Support/Valgrind.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
using namespace std;
namespace termloop{
    llvm::FunctionPass *createLoopPass();
}


#endif //TERMINITION_TERMLOOPPASS_H
