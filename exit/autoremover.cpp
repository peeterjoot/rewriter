#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

static cl::OptionCategory ToolCategory("auto-replacer-tool");

class AutoReplacer : public MatchFinder::MatchCallback {
public:
    AutoReplacer(Rewriter &R) : Rewrite(R) {}

    void run(const MatchFinder::MatchResult &Result) override {
        if (const auto *VD = Result.Nodes.getNodeAs<VarDecl>("autoVar")) {
            // Only process variables that use 'auto' as type specifier
            if (!VD->getTypeSourceInfo() || !VD->getTypeSourceInfo()->getType()->isAutoType()) {
                return;
            }

            QualType Deduced = VD->getType();
            if (Deduced->isDeducedType()) {
                Deduced = Deduced->getDeducedType();
            }

            if (Deduced.isNull()) {
                errs() << "Failed to deduce type\n";
                return;
            }

            ASTContext *Ctx = Result.Context;
            if (!Ctx) return;

            SourceLocation TypeStart = VD->getTypeSpecStartLoc();
            if (!TypeStart.isValid() || !TypeStart.isFileID()) {
                return;
            }

            SourceLocation TypeEnd = VD->getTypeSpecEndLoc();
            if (!TypeEnd.isValid()) {
                // Fallback: compute end from the name location
                SourceLocation NameLoc = VD->getLocation();
                if (NameLoc.isValid()) {
                    TypeEnd = NameLoc.getLocWithOffset(-1);
                }
            }

            if (!TypeEnd.isValid() || !TypeEnd.isFileID()) {
                errs() << "Invalid type end location\n";
                return;
            }

            // Ensure we are replacing the correct range (skip leading cv-qualifiers if any)
            std::string Replacement = Deduced.getAsString(Ctx->getPrintingPolicy());

            // Replace the 'auto' (and any trailing cv/ref) with the deduced type
            SourceRange ReplaceRange(TypeStart, TypeEnd);
            Rewrite.ReplaceText(ReplaceRange, Replacement);
        }
    }

private:
    Rewriter &Rewrite;
};

class MyASTConsumer : public ASTConsumer {
public:
    MyASTConsumer(MatchFinder *Finder, Rewriter &R) : Finder(Finder), Rewriter(R) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Finder->matchAST(Context);
    }

private:
    MatchFinder *Finder;
    Rewriter &Rewriter;
};

class MyFrontendAction : public ASTFrontendAction {
public:
    MyFrontendAction() {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef InFile) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());

        auto *Replacer = new AutoReplacer(TheRewriter);
        Finder = std::make_unique<MatchFinder>();

        // Match local and global variables (including statics) that use 'auto' and have an initializer
        // (auto without initializer is invalid in most contexts)
        DeclarationMatcher AutoMatcher = varDecl(
            hasType(autoType()),
            hasInitializer(anything()),
            unless(parmVarDecl())  // exclude function parameters for simplicity
        ).bind("autoVar");

        Finder->addMatcher(AutoMatcher, Replacer);

        return std::make_unique<MyASTConsumer>(Finder.get(), TheRewriter);
    }

    void EndSourceFileAction() override {
        TheRewriter.overwriteChangedFiles();
    }

private:
    Rewriter TheRewriter;
    std::unique_ptr<MatchFinder> Finder;
};

int main(int argc, const char **argv) {
    auto OptionsParser = CommonOptionsParser::create(argc, argv, ToolCategory, cl::OneOrMore);
    if (!OptionsParser) {
        errs() << OptionsParser.takeError();
        return 1;
    }

    ClangTool Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList());
    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
