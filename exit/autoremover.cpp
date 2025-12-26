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
            TypeSourceInfo *TSI = VD->getTypeSourceInfo();
            if (!TSI) return;

            QualType QT = TSI->getType();
            if (QT.isNull()) return;

            const clang::Type *TP = QT.getTypePtr();
            if (!TP) return;

            const AutoType *AT = TP->getAs<AutoType>();
            if (!AT) return;

            // Skip if not deduced yet (e.g., dependent or no initializer)
            if (!AT->isDeduced()) return;

            QualType Deduced = AT->getDeducedType();
            if (Deduced.isNull()) return;

            ASTContext *Ctx = Result.Context;
            if (!Ctx) return;

            SourceLocation TypeStart = TSI->getTypeLoc().getBeginLoc();
            if (!TypeStart.isValid() || !TypeStart.isFileID()) return;

            // Find the end of the type specifier (after 'auto' and any cv/ref)
            TypeLoc TL = TSI->getTypeLoc();
            AutoTypeLoc ATL = TL.getAs<AutoTypeLoc>();
            if (ATL.isNull()) return;

            SourceLocation TypeEnd = ATL.getRParenLoc();
            if (!TypeEnd.isValid() || TypeEnd.isMacroID()) {
                // Fallback: use location just before the name
                SourceLocation NameLoc = VD->getLocation();
                if (NameLoc.isValid()) {
                    TypeEnd = NameLoc.getLocWithOffset(-1);
                }
            }

            if (!TypeEnd.isValid() || !TypeEnd.isFileID()) {
                errs() << "Invalid type end location\n";
                return;
            }

            PrintingPolicy Policy = Ctx->getPrintingPolicy();
            Policy.SuppressScope = false;  // Print full qualified names
            Policy.FullyQualifiedName = true;

            std::string Replacement = Deduced.getAsString(Policy);

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

        DeclarationMatcher AutoMatcher = varDecl(
            hasType(autoType()),
            hasInitializer(anything()),
            unless(isExpansionInSystemHeader()),
            unless(parmVarDecl())  // Exclude function parameters
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
