#include <iostream>
#include <memory>
#include <vector>

#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "parser.h"
#include "ast.h"
#include "tabsym.h"

using namespace llvm;
using namespace llvm::legacy;
using namespace std;
Constant* printFunc(); 
int createObjectFile(Module *mod, const char *targetName)
{
  // Initialize targets first, so that --version shows registered targets.
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  // Initialize codegen and IR passes used by llc so that the -print-after,
  PassRegistry *Registry = PassRegistry::getPassRegistry();
  initializeCore(*Registry);
  initializeCodeGen(*Registry);
  initializeLoopStrengthReducePass(*Registry);
  initializeLowerIntrinsicsPass(*Registry);
  initializeUnreachableBlockElimPass(*Registry);

  // Load the module to be compiled...
  SMDiagnostic Err;
  Triple TheTriple;
  TheTriple.setTriple(sys::getDefaultTargetTriple());

  // Get the target specific parser.
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(MArch, TheTriple, Error);
  if (!TheTarget)
  {
    errs() << "Error: " << Error;
    return 1;
  }

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (MAttrs.size())
  {
    SubtargetFeatures Features;
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  CodeGenOpt::Level OLvl = CodeGenOpt::None;
  TargetOptions Options = InitTargetOptionsFromCodeGenFlags();

  std::unique_ptr<TargetMachine> Target(TheTarget->createTargetMachine(TheTriple.getTriple(), MCPU, FeaturesStr, Options, RelocModel, CMModel, OLvl));

  assert(Target && "Could not allocate target machine!");

  // Open the file.
  std::error_code EC;
  sys::fs::OpenFlags OpenFlags = sys::fs::F_None;
  std::unique_ptr<tool_output_file> Out = llvm::make_unique<tool_output_file>(targetName, EC, OpenFlags);
  if (EC)
  {
    errs() << EC.message() << '\n';
    return 1;
  }

  // Build up all of the passes that we want to do to the module.
  PassManager PM;

  // Add an appropriate TargetLibraryInfo pass for the module's triple.
  TargetLibraryInfoImpl TLII(Triple(mod->getTargetTriple()));

  // The -disable-simplify-libcalls flag actually disables all builtin optzns.
  PM.add(new TargetLibraryInfoWrapperPass(TLII));

  // Add the target data from the target machine, if it exists, or the module.
  if (const DataLayout *DL = Target->getDataLayout())
    mod->setDataLayout(DL);
  PM.add(new DataLayoutPass());

  if (RelaxAll.getNumOccurrences() > 0 && FileType != TargetMachine::CGFT_ObjectFile)
    errs() << ": warning: ignoring -mc-relax-all because filetype != obj";
  {
    formatted_raw_ostream FOS(Out->os());

    AnalysisID StartAfterID = nullptr;
    AnalysisID StopAfterID = nullptr;
    const PassRegistry *PR = PassRegistry::getPassRegistry();
    if (!StartAfter.empty()) {
      const PassInfo *PI = PR->getPassInfo(StartAfter);
      if (!PI) {
        errs() << ": start-after pass is not registered.\n";
        return 1;
      }
      StartAfterID = PI->getTypeInfo();
    }
    if (!StopAfter.empty()) {
      const PassInfo *PI = PR->getPassInfo(StopAfter);
      if (!PI) {
        errs() << ": stop-after pass is not registered.\n";
        return 1;
      }
      StopAfterID = PI->getTypeInfo();
    }

    // Ask the target to add backend passes as necessary.
    if (Target->addPassesToEmitFile(PM, FOS, TargetMachine::CodeGenFileType::CGFT_ObjectFile, false,
      StartAfterID, StopAfterID)) {
      errs() << ": target does not support generation of this file type!\n";
      return 1;
    }

    // Before executing passes, print the final values of the LLVM options.
    cl::PrintOptionValues();

    PM.run(*mod);
  }

  // Declare success.
  Out->keep();

  return 0;
}

IRBuilder<> builder(getGlobalContext());
Module * module = new Module("Sfe", getGlobalContext());
Value* writeFormatStr;
Value* scanfFormatStr;
BasicBlock* breakTarget;
static std::map<std::string, AllocaInst*> NamedValues;
static  AllocaInst* CreateEntryBlockAlloca(Function *TheFunction,
                                        std::string VarName) 
{
  IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                   TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(Type::getInt32Ty(getGlobalContext()), 0,
                            VarName.c_str());
}

void GetVariables(){
    std::vector<Variable> vars = VarNames();
    Function *TheFunction = builder.GetInsertBlock()->getParent();
    for(auto it = vars.begin(); it != vars.end(); it++){
        AllocaInst* alloc = CreateEntryBlockAlloca(TheFunction, it->name);
        builder.CreateStore(builder.getInt32(0), alloc);
        NamedValues[it->name] = alloc;
    }

}

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " programName [-d]" << endl;
    exit(1);
  }

  bool debug = false;
  for (int i = 2; i < argc; i++)
  {
    if (strcmp(argv[i], "-d") == 0)
      debug = true;
  }
  Type * returnType = Type::getInt32Ty(getGlobalContext());
  std::vector<Type*> argTypes;
  FunctionType *functionType = FunctionType::get(returnType, argTypes, false);
  Function * function = Function::Create(functionType, Function::ExternalLinkage, "main", module);
  
  BasicBlock *BB = BasicBlock::Create(getGlobalContext(), "entry", function);
  builder.SetInsertPoint(BB);

  
  char * fileName = argv[1];
  if(!initParser(fileName)) {
         printf("Error creating the syntax analyzer.\n");
         return 0;
  }
  writeFormatStr = builder.CreateGlobalStringPtr("value = %d\n");
  scanfFormatStr = builder.CreateGlobalStringPtr("%d");
  Node* res = Program();
  GetVariables();
  res->codegen();
  Value * returnValue = builder.getInt32(0);
  builder.CreateRet(returnValue);

  if (debug)
  {
    printf("== dump start ==\n");
    module->dump();
    printf("== dump end ==\n");
  }
  
  createObjectFile(module, "a.o");
  
  delete module;
  llvm_shutdown();
  
  system("gcc a.o -o a.out");
  
  return 0;
}

Constant* printFunc() {
    std::vector<Type *> args;
    args.push_back(Type::getInt8PtrTy(getGlobalContext()));
    FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), args, true);
    return module->getOrInsertFunction("printf", printfType);
}

Constant* scanfFunc() {
    std::vector<Type *> args;
    args.push_back(Type::getInt8PtrTy(getGlobalContext()));
    FunctionType *scanfType = FunctionType::get(builder.getInt32Ty(), args, true);
    return module->getOrInsertFunction("scanf", scanfType);

}

//--------------CodeGEN-----------------

void LogError(const char *Str) {
    std::cout << Str;
    exit(1);
}
Value* Var::codegen() {

   if(rvalue){
        Value *V = NamedValues[name];
        if(!V){
            LogError("Unknown variable name");
            return nullptr;
        }
        return builder.CreateLoad(V, name.c_str());
   }
   else{
       LogError("Bad use of variable");
       return nullptr;
    
   }
}


Value* IntConst::codegen() {
    return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), val);
}

Value* BinOp::codegen() {
    Value* L = left->codegen();
    Value* R = right->codegen();
    switch (op) {
        case Add:
            return builder.CreateAdd(L, R, ".addtmp");
        case Sub:
            return builder.CreateSub(L, R, ".subtmp");
        case Mult:
            return builder.CreateMul(L, R, ".multmp");
        case Div:
            return builder.CreateSDiv(L, R, ".divtmp");
        case Mod:
            return builder.CreateSRem(L, R, ".modtmp");
        case Eq:
            return builder.CreateICmpEQ(L, R, ".eqtmp");
        case Neq:
            return builder.CreateICmpNE(L, R, ".neqtmp");
        case Lt:
            return builder.CreateICmpSLT(L, R, ".lttmp");
        case Gt:
            return builder.CreateICmpSGT(L, R, ".gttmp");
        case Lte:
            return builder.CreateICmpSLE(L, R, ".ltetmp");
        case Gte:
            return builder.CreateICmpSGE(L, R, ".gtetmp");
                    
        default:
            LogError("Errors in BinOp code genearting! Wrong OP!\n ");
            return nullptr;
    }
}

Value* UnMinus::codegen() {
    Value* E = expr->codegen();
    return builder.CreateNeg(E, ".negtmp");
}
Value* If::codegen() {
    Value* condV = cond->codegen();
    if(!condV)
        return nullptr;
    Function* theFunction = builder.GetInsertBlock()->getParent();
    BasicBlock* ThenBB = BasicBlock::Create(getGlobalContext(), "then", theFunction);
    BasicBlock* ElseBB = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock* MergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");

    builder.CreateCondBr(condV, ThenBB, ElseBB);
    builder.SetInsertPoint(ThenBB);
    thenstm->codegen();
    builder.CreateBr(MergeBB);
    theFunction->getBasicBlockList().push_back(ElseBB);
    builder.SetInsertPoint(ElseBB);
    if(elsestm){
        elsestm->codegen();
    }
    builder.CreateBr(MergeBB);
    theFunction->getBasicBlockList().push_back(MergeBB);
    builder.SetInsertPoint(MergeBB);
    return nullptr; 
}

Value* While::codegen() {
    Value* condV = cond->codegen();
    Function* theFunction = builder.GetInsertBlock()->getParent();
    BasicBlock* LoopBB = BasicBlock::Create(getGlobalContext(), "loop", theFunction);
    BasicBlock* aftLoopBB = BasicBlock::Create(getGlobalContext(), "aft");
    BasicBlock* previousTarget = breakTarget;
    breakTarget = aftLoopBB;
    builder.CreateCondBr(condV, LoopBB, aftLoopBB);
    builder.SetInsertPoint(LoopBB);
    body->codegen();
    condV = cond->codegen();
    builder.CreateCondBr(condV, LoopBB, aftLoopBB);
    theFunction->getBasicBlockList().push_back(aftLoopBB);
    breakTarget = previousTarget;
    builder.SetInsertPoint(aftLoopBB);
    return nullptr; 
}
Value* Break::codegen(){
    Function* theFunction = builder.GetInsertBlock()->getParent();
    BasicBlock* breakBB = BasicBlock::Create(getGlobalContext(), "break", theFunction);
     BasicBlock* aftBreakBB = BasicBlock::Create(getGlobalContext(), "aftbreak");
    builder.CreateBr(breakBB);
    builder.SetInsertPoint(breakBB);
    if(breakTarget){
        builder.CreateBr(breakTarget);
    }
    theFunction->getBasicBlockList().push_back(aftBreakBB);
    builder.SetInsertPoint(aftBreakBB);
    return nullptr;
}
Value* Assign::codegen(){
    Value* V = NamedValues[var->name];
    if(!V){
        LogError("Unknown variable name");
        return nullptr;
    }
    Value* E = expr->codegen();
    return builder.CreateStore(E, V);
}
Value* Write::codegen() { 
    Value* E = expr->codegen();
    std::vector<llvm::Value *> values;
    values.push_back(writeFormatStr);
    values.push_back(E);
    return builder.CreateCall(printFunc(), values);
}

Value* Read::codegen(){
    Value* V = NamedValues[var->name];
    if(!V){
        LogError("Unknown variable name");
        return nullptr;
    }
    std::vector<llvm::Value *> values;
    values.push_back(scanfFormatStr);
    values.push_back(V);
    return builder.CreateCall(scanfFunc(), values);
}

Value* StatmList::codegen() {
    for(auto iter = statm_list.begin(); iter < statm_list.end(); iter++){
        (*iter)->codegen();
    }
    return nullptr;
}
