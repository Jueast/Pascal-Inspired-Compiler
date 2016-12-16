#include "llvmlib.h"
#include "llvm/IR/TypeBuilder.h"
#include <vector>
using namespace llvm;
extern Module* module;
extern IRBuilder<> builder;
Constant* printFunc() {
    std::vector<Type *> args;
    args.push_back(Type::getInt8PtrTy(getGlobalContext()));
    FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), args, true);
    return module->getOrInsertFunction("printf", printfType);
}

