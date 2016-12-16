#include <iostream>
#include <memory>


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
  Node* res = Program();
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


//--------------CodeGEN-----------------

void LogError(const char *Str) {
    std::cout << Str;
    exit(1);
}

Value* IntConst::codegen() {
    return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), val);
}

Value* BinOp::codegen() {
    Value* L = left->codegen();
    Value* R = right->codegen();

    switch (op) {
        case '+':
            return builder.CreateAdd(L, R, ".addtmp");
        case '-':
            return builder.CreateSub(L, R, ".subtmp");
        case '*':
            return builder.CreateMul(L, R, ".multmp");
        case '/':
            return builder.CreateSDiv(L, R, ".divtmp");
        default:
            LogError("Errors in BinOp code genearting! Wrong OP!\n ");
            return nullptr;
    }
}

Value* UnMinus::codegen() {
    Value* E = expr->codegen();
    return builder.CreateNeg(E, ".negtmp");
}

Value* Write::codegen() {
    Value* E = expr->codegen();
    Value* formatStr = builder.CreateGlobalStringPtr("value = %d\n");
    std::vector<llvm::Value *> values;
    values.push_back(formatStr);
    values.push_back(E);
    return builder.CreateCall(printFunc(), values);
}

Value* StatmList::codegen() {
    for(auto iter = statm_list.begin(); iter < statm_list.end(); iter++){
        (*iter)->codegen();
    }
    return nullptr;
}
