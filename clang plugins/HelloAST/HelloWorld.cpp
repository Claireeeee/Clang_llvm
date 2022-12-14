//==============================================================================
// FILE:
//    HelloWorld.cpp
//
// DESCRIPTION:
//    Counts the number of C++ record declarations in the input translation
//    unit. The results are printed on a file-by-file basis (i.e. for each
//    included header file separately).
//
//    Internally, this implementation leverages llvm::StringMap to map file
//    names to the corresponding #count of declarations.
//
// USAGE:
//   clang -cc1 -load <BUILD_DIR>/lib/libHelloWorld.dylib '\'
//    -plugin hello-world test/HelloWorld-basic.cpp
//
// License: The Unlicense
//==============================================================================
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

//-----------------------------------------------------------------------------
// RecursiveASTVisitor
//-----------------------------------------------------------------------------
class HelloWorld : public RecursiveASTVisitor<HelloWorld> {
public:
  explicit HelloWorld(ASTContext *Context) : Context(Context) {}
  bool VisitCXXRecordDecl(CXXRecordDecl *Decl);
  bool VisitFunctionDecl(FunctionDecl *Decl);
  llvm::StringMap<unsigned> getDeclMap() { return DeclMap; }

private:
  ASTContext *Context;
  // Map that contains the count of declaration in every input file
  llvm::StringMap<unsigned> DeclMap;
};

bool HelloWorld::VisitCXXRecordDecl(CXXRecordDecl *Declaration) {
  FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());

  // Basic sanity checking
  if (!FullLocation.isValid())
    return true;

  // There are 2 types of source locations: in a file or a macro expansion. The
  // latter contains the spelling location and the expansion location (both are
  // file locations), but only the latter is needed here (i.e. where the macro
  // is expanded). File locations are just that - file locations.
  if (FullLocation.isMacroID())
    FullLocation = FullLocation.getExpansionLoc();

  SourceManager &SrcMgr = Context->getSourceManager();
  const FileEntry *Entry =
      SrcMgr.getFileEntryForID(SrcMgr.getFileID(FullLocation));
  DeclMap[Entry->getName()]++;

  return true;
}

bool HelloWorld::VisitFunctionDecl(FunctionDecl *Declaration) {
  FullSourceLoc FullLocation = Context->getFullLoc(Declaration->getBeginLoc());

  // Basic sanity checking
  if (!FullLocation.isValid())
    return true;

  // There are 2 types of source locations: in a file or a macro expansion. The
  // latter contains the spelling location and the expansion location (both are
  // file locations), but only the latter is needed here (i.e. where the macro
  // is expanded). File locations are just that - file locations.
  if (FullLocation.isMacroID())
    FullLocation = FullLocation.getExpansionLoc();

  SourceManager &SrcMgr = Context->getSourceManager();
  const FileEntry *Entry =
      SrcMgr.getFileEntryForID(SrcMgr.getFileID(FullLocation));
  DeclMap[Entry->getName()]++;

  return true;
}

class FuncCallback : public ast_matchers::MatchFinder::MatchCallback {
public:
  virtual void
  run(const clang::ast_matchers::MatchFinder::MatchResult &Result) final {
    llvm::outs() << "(found function) ";
    if (const auto *F =
            Result.Nodes.getNodeAs<clang::FunctionDecl>("func")) {
      const auto& SM = *Result.SourceManager;
      const auto& Loc = F->getLocation();
      auto name = F->getName();
      llvm::outs() << "Name : " << name << "  "
                   << "Loc : " << SM.getSpellingLineNumber(Loc) << ":"
                   << SM.getSpellingColumnNumber(Loc) << "\n";
    }
  }
};


//-----------------------------------------------------------------------------
// ASTConsumer
//-----------------------------------------------------------------------------
class HelloWorldASTConsumer : public clang::ASTConsumer {
public:
  explicit HelloWorldASTConsumer(ASTContext *Ctx) : Visitor(Ctx) {
    // ast_matchers::DeclarationMatcher funcMatcher = ast_matchers::functionDecl(ast_matchers::hasName("main")).bind("func");
    ast_matchers::DeclarationMatcher funcMatcher = ast_matchers::functionDecl().bind("func");
    Matcher.addMatcher(funcMatcher, &funcback);
  }

  void HandleTranslationUnit(clang::ASTContext &Ctx) override {
    Matcher.matchAST(Ctx);
    Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
    if (Visitor.getDeclMap().empty()) {
      llvm::outs() << "(clang-tutor)  no declarations found "
                   << "\n";
      return;
    }

    for (auto &Element : Visitor.getDeclMap()) {
      llvm::outs() << "(clang-tutor)  file: " << Element.first() << "\n";
      llvm::outs() << "(clang-tutor)  count: " << Element.second << "\n";
    }

    /*MatchFinder finder;
    //body ??????
    finder.addMatcher(ifStmt(isExpansionInMainFile(),hasThen(compoundStmt(statementCountIs(0)))).bind("ifStmt_empty_then_body"), &handlerForMatchResult);
    //condition_always_true
    finder.addMatcher(ifStmt(isExpansionInMainFile(),hasCondition(integerLiteral(unless(equals(false))))).bind("condition_always_true"), &handlerForMatchResult);
    finder.addMatcher(ifStmt(isExpansionInMainFile(),hasCondition(floatLiteral(unless(equals(false))))).bind("condition_always_true"), &handlerForMatchResult);
    //condition_always_false
    finder.addMatcher(ifStmt(isExpansionInMainFile(),hasCondition(characterLiteral(equals(false)))).bind("condition_always_false"), &handlerForMatchResult);
    finder.addMatcher(ifStmt(isExpansionInMainFile(),hasCondition(unaryOperator(hasOperatorName("~"),hasUnaryOperand(integerLiteral(unless(equals(false))))))).bind("condition_always_false"), &handlerForMatchResult); */

  }

private:
  ast_matchers::MatchFinder Matcher;
  HelloWorld Visitor;
  FuncCallback funcback;
};

//-----------------------------------------------------------------------------
// FrontendAction for HelloWorld
//-----------------------------------------------------------------------------
class FindNamedClassAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &Compiler,
                    llvm::StringRef InFile) override {
    return std::unique_ptr<clang::ASTConsumer>(
        std::make_unique<HelloWorldASTConsumer>(&Compiler.getASTContext()));
  }
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    return true;
  }
};

//-----------------------------------------------------------------------------
// Registration
//-----------------------------------------------------------------------------
static FrontendPluginRegistry::Add<FindNamedClassAction>
    X(/*Name=*/"hello-world", /*Description=*/"The HelloWorld plugin");