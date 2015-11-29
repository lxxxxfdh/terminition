#include <iostream>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "loopAnalysis/termLoopPass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/PassAnalysisSupport.h"

using namespace std;
using namespace llvm;
using namespace termloop;


int main() {
    //cout << "Hello, World!" << endl;
    LLVMContext &Context = getGlobalContext();
    SMDiagnostic Err;
    char* p1="/home/xie/terminition_test/testreg.ll";
    p1="/home/xie/ClionProjects/terminition/test/aboveIncreg.ll"; //aboveDecSinglereg.ll  belowDecSinglereg.ll   aboveIncSinglereg.ll  belowIncSinglereg.ll
    std::unique_ptr<Module> Mod = parseIRFile(p1, Err, Context);

    //traverse(root);

    if (Mod) {
        //std::cout << "Mod is not null" << std::endl;
        legacy::PassManager pm;
        pm.add(createLoopPass());
        pm.run(*Mod);
       // errs()<<*Mod;
    }
    else {
        std::cout << "Mod is null" << std::endl;
    }
    return 0;
}