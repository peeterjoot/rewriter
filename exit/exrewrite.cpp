#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

// Define a category for the command line options
static llvm::cl::OptionCategory ToolCategory("function-replacer-tool");

// Define command-line options for `from` and `to` function names
static llvm::cl::opt<std::string> FromFunctionName("from", llvm::cl::desc("Specify the function name to replace"), llvm::cl::value_desc("function name"), llvm::cl::Required, llvm::cl::cat(ToolCategory));
static llvm::cl::opt<std::string> ToFunctionName("to", llvm::cl::desc("Specify the function name to replace with"), llvm::cl::value_desc("function name"), llvm::cl::Required, llvm::cl::cat(ToolCategory));

class FunctionCallReplacer : public MatchFinder::MatchCallback {
public:
    FunctionCallReplacer(Rewriter &R) : Rewrite(R) {}

    void run(const MatchFinder::MatchResult &Result) override;
private:
    Rewriter &Rewrite;
};

void FunctionCallReplacer::run(const MatchFinder::MatchResult &Result) {
    // Check if we are looking at a function call
    if (const CallExpr *call = Result.Nodes.getNodeAs<CallExpr>("funcCall")) {
        const FunctionDecl *FD = call->getDirectCallee();
        // Only replace if the function name matches `from`
        if (FD && FD->getNameAsString() == FromFunctionName) {
            clang::SourceLocation loc = call->getBeginLoc();
            if (loc.isValid() && loc.isFileID()) {
#if 0
                std::cout << "FromFunctionName: " << FromFunctionName << '\n';
                std::cout << "ToFunctionName: " << ToFunctionName << '\n';
                std::cout << "FD: " << FD->getNameAsString() << '\n';
#endif

                // Replace the "from" function with the "to" function
                Rewrite.ReplaceText(loc, FD->getNameAsString().length(), ToFunctionName);
            } else {
                llvm::errs() << "Invalid source location!\n";
            }
        }
    }
}

// Custom ASTConsumer class
class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(MatchFinder *finder, Rewriter &rewriter)
        : Finder(finder), Rewriter(rewriter) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        // Execute the MatchFinder
        Finder->matchAST(Context);
    }

private:
    MatchFinder *Finder; // Pointer to the MatchFinder
    Rewriter &Rewriter;  // Reference to the Rewriter
};

// Frontend action class
class MyFrontendAction : public ASTFrontendAction {
public:
    MyFrontendAction() : TheRewriter() {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        // Initialize the Rewriter with the correct SourceManager and LangOptions
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());

        // Create the MatchFinder and pass the Rewriter
        auto *Replacer = new FunctionCallReplacer(TheRewriter);
        auto *Finder = new MatchFinder();
        Finder->addMatcher(callExpr(callee(functionDecl(hasName(FromFunctionName)))).bind("funcCall"), Replacer);

        // Return the custom ASTConsumer
        return std::make_unique<MyASTConsumer>(Finder, TheRewriter);
    }

    void EndSourceFileAction() override {
        // After processing all files, overwrite the changed files
        TheRewriter.overwriteChangedFiles();
    }

private:
    Rewriter TheRewriter; // Member to hold the Rewriter
};

int main(int argc, const char **argv) {
    argv[0] = "/usr/bin/clang";

    auto OptionsParser = CommonOptionsParser::create(argc, argv, ToolCategory);
    if (!OptionsParser) {
        llvm::errs() << OptionsParser.takeError();
        return 1;
    }

    ClangTool Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList());

    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}

// vim: et ts=4 sw=4
