#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"


using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace
{
    class ToIRVisitor : public ASTVisitor
    {
        Module* M;
        IRBuilder<> Builder;
        Type* VoidTy;
        Type* Int32Ty;
        Type* Int8PtrTy;
        Type* Int8PtrPtrTy;
        Constant* Int32Zero;
        

        Value* V;
        StringMap<AllocaInst*> nameMap;

        FunctionType* MainFty;
        Function* MainFn;
        FunctionType *CalcWriteFnTy;
        Function *CalcWriteFn;
    public:
        // Constructor for the visitor class.
        ToIRVisitor(Module* M) : M(M), Builder(M->getContext())
        {
            // Initialize LLVM types and constants.
            VoidTy = Type::getVoidTy(M->getContext());
            Int32Ty = Type::getInt32Ty(M->getContext());
            Int8PtrTy = Type::getInt8PtrTy(M->getContext());
            Int8PtrPtrTy = Int8PtrTy->getPointerTo();
            Int32Zero = ConstantInt::get(Int32Ty, 0, true);
            CalcWriteFn = Function::Create(CalcWriteFnTy, GlobalValue::ExternalLinkage, "print", M);
        }
        void run(AST* Tree)
        {
            // Create the main function with the appropriate function type.
            MainFty = FunctionType::get(Int32Ty, { Int32Ty, Int8PtrPtrTy }, false);
            MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

            // Create a basic block for the entry point of the main function.
            BasicBlock* BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
            Builder.SetInsertPoint(BB);

            // Visit the root node of the AST to generate IR.
            Tree->accept(*this);

            // Create a return instruction at the end of the main function.
            Builder.CreateRet(Int32Zero);
        }
        virtual void visit(Base& Node) override
        {
            for (auto I = Node.begin(), E = Node.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        }
        virtual void visit(Statement& Node) override
        {
             if (Node.getKind() == Statement::StateMentType::DeclarationInt) 
            {
                DefInt* declaration = (DefInt*)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StateMentType::DeclarationBool)
            {
                DefBool* declaration = (DefBool*)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StateMentType::Assignment)
            {
                AssignStatement* declaration = (AssignStatement*)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StateMentType::If)
            {
                IfStatement* declaration = (IfStatement*)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StateMentType::ElseIf)
            {
                ElseIfStatement* declaration = (ElseIfStatement*)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StateMentType::Else)
            {
                ElseStatement* declaration = (ElseStatement*)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StateMentType::While)
            {
                WhileStatement* declaration = (WhileStatement*)&Node;
                declaration->accept(*this);
            }
            else if (Node.getKind() == Statement::StateMentType::For)
            {
                ForStatement* declaration = (ForStatement*)&Node;
                declaration->accept(*this);
            }
            
            
        }

    };
}; // namespace

