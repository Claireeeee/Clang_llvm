helloworld for clang plugin, an editd version of https://github.com/banach-space/clang-tutor

#### to build and run 
  in this dir:
  ```
  mkdir build && cd build
  # before cmake : set local LLVM dir in CMakeLists.txt:line8
  cmake ..
  make
  <dir to clang14>/clang -cc1 -load ./libHelloWorld.so -plugin hello-world ../test.cpp
  ```
  
  the output should be : 
  ```
  (found function) Name : a  Loc : 14:5
  (found function) Name : b  Loc : 18:5
  (clang-tutor)  file: ../test.cpp
  (clang-tutor)  count: 5
  ```
  
#### some notes :
  to use RecursiveASTVisitor:
  ```
  #include "clang/AST/ASTConsumer.h"
  #include "clang/AST/RecursiveASTVisitor.h"
  #include "clang/Frontend/CompilerInstance.h"
  #include "clang/Frontend/FrontendPluginRegistry.h"

  1. customize ASTConsumer to build your own program analyzer : 
      rewrite HandleTranslationUnit() in it to call your own vistor; 
       （and other hooks, like HandleTopLevelDecl）
      calls TraverseDecl() in your vistor to start recursively travel a target AST 
        (by calling VisitStmt (), VisitFunctionDecl() etc.)
        (it visits each Stmt in a depth-first search order defaultly. you can change this.)

  2. customize RecursiveASTVisitor to build your own vistor: 
	  	rewrite VisitStmt (), VisitFunctionDecl() in it.
      
  3. customize PluginASTAction to creat your ASTConsumer
      check the code

  4. PluginASTAction
  ```
  
  to use ASTMatcher:
  ```
  // plus include
  #include "clang/ASTMatchers/ASTMatchers.h"
  #include "clang/ASTMatchers/ASTMatchFinder.h"
  
  1. customize ast_matchers::MatchFinder::MatchCallback() to build your own match action
      rewrite the run() in it, get the result by id, and deal with it
  2. add matcher in your consumer
      define your matcher(), with all the functions and rules and bind it with an id;
      add it through MatchFinder();
      call it in HandleTranslationUnit()
  ```
