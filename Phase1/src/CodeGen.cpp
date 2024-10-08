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
        Type* Int1Ty; //boolean
        Type* Int32Ty;
        Type* Int8PtrTy;
        Type* Int8PtrPtrTy;
        Constant* Int32Zero;
        Constant* Int32One;
        Constant* Int1False;
        Constant* Int1True;
        

        Value* V;
        StringMap<AllocaInst*> nameMapInt;
        StringMap<AllocaInst*> nameMapBool;


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

        // Entry point for generating LLVM IR from the AST.
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

        virtual void visit(Expression& Node) override
        {
            if (Node.getKind() == Expression::ExpressionType::Identifier)
            {
                V = Builder.CreateLoad(Int32Ty, nameMapInt[Node.getValue()]); ///???????
            }
            else if (Node.getKind() == Expression::ExpressionType::Number)
            {
               
                int intval = Node.getNumber();
                V = ConstantInt::get(Int32Ty, intval, true);
            }
            else if (Node.getKind() == Expression::ExpressionType::Boolean)
            {
               
                bool boolVal = Node.getBoolean();
                V = ConstantInt::get(Int32Ty, boolVal, true);                              //boolean
            }
            else if (Node.getKind() == Expression::ExpressionType::BooleanOpType) {
                BooleanOp* temp = Node.getBooleanOp();
                if (temp->getOperator() == BooleanOp::Operator::And)
                {
                    (temp->getLeft())->accept(*this);
                    Value* Left = V;

                    (temp->getRight())->accept(*this);
                    Value* Right = V;

                    V = Builder.CreateAnd(Left, Right);
                }
                else if (temp->getOperator() == BooleanOp::Operator::Or)
                {
                    (temp->getLeft())->accept(*this);
                    Value* Left = V;

                    (temp->getRight())->accept(*this);
                    Value* Right = V;

                    V = Builder.CreateOr(Left, Right);
                }
            }
        }


        virtual void visit(BooleanOp& Node) override
        {
            // Visit the left-hand side of the binary operation and get its value.
            Node.getLeft()->accept(*this);
            Value* Left = V;

            // Visit the right-hand side of the binary operation and get its value.
            Node.getRight()->accept(*this);
            Value* Right = V;

            // Perform the boolean operation based on the operator type and create the corresponding instruction.
            switch (Node.getOperator())
            {
            case BooleanOp::Equal:
                V = Builder.CreateICmpEQ(Left, Right);
                break;
            case BooleanOp::NotEqual:
                V = Builder.CreateICmpNE(Left, Right);
                break;
            case BooleanOp::Less:
                V = Builder.CreateICmpSLT(Left, Right);
                break;
            case BooleanOp::LessEqual:
                V = Builder.CreateICmpSLE(Left, Right);
                break;
            case BooleanOp::Greater:
                V = Builder.CreateICmpSGT(Left, Right);
                break;
            case BooleanOp::GreaterEqual:
                V = Builder.CreateICmpSGE(Left, Right);
                break;
            case BooleanOp::And:
                V = Builder.CreateAnd(Left, Right);
                break;
            case BooleanOp::Or:
                V = Builder.CreateOr(Left, Right);
                break;
            }
        }


        virtual void visit(BinaryOp& Node) override
        {
            // Visit the left-hand side of the binary operation and get its value.
            Node.getLeft()->accept(*this);
            Value* Left = V;

            // Visit the right-hand side of the binary operation and get its value.
            Node.getRight()->accept(*this);
            Value* Right = V;

            // Perform the binary operation based on the operator type and create the corresponding instruction.
            switch (Node.getOperator())
            {
            case BinaryOp::Plus:
                V = Builder.CreateNSWAdd(Left, Right);
                break;
            case BinaryOp::Minus:
                V = Builder.CreateNSWSub(Left, Right);
                break;
            case BinaryOp::Mul:
                V = Builder.CreateNSWMul(Left, Right);
                break;
            case BinaryOp::Div:
                V = Builder.CreateSDiv(Left, Right);
                break;
            case BinaryOp::Pow:
                if ((Node.getRight())->isNumber())
                {
                    int power = (Node.getRight())->getNumber();
                    Value* result = Left;
                    for (int i=1; i<power; i++)
                    {
                        result = Builder.CreateNSWMul(result, Left);
                    }
                    V = result;
                }
                break;
            case BinaryOp::Mod:
                Value* division = Builder.CreateSDiv(Left, Right);
                Value* multiplication = Builder.CreateNSWMul(division, Right);
                V = Builder.CreateNSWSub(Left, multiplication);
            }
        }
        virtual void visit(UneryOp& Node) override
        {
            // Visit the left-hand side of the unary operation and get its value.
            Node.getLeft()->accept(*this);
            Value* Left = V;


            // Perform the unary operation based on the operator type and create the corresponding instruction.
            switch (Node.getOperator())
            {
            case UneryOp::Plus:
                V = Builder.CreateNSWAdd(Left, 1);
                break;
            case UneryOp::Minus:
                V = Builder.CreateNSWSub(Left, 1);
                break;

            }
        }

        virtual void visit(DefInt& Node) override  
        {
            Value* val = nullptr;                        // defint

            if (Node.getRValue()->getKind() == Expression::ExpressionType::BinaryOpType || Node.getRValue()->isNumber())
            {
                // If there is an expression provided, visit it and get its value.
                Node.getRValue()->accept(*this);
                val = V;
            }

            // Iterate over the variables declared in the declaration statement.

            auto I = Node.getLValue()->getValue();
            StringRef Var = I;

            // Create an alloca instruction to allocate memory for the variable.
            nameMapInt[Var] = Builder.CreateAlloca(Int32Ty);

            // Store the initial value (if any) in the variable's memory location.
            if (val != nullptr)
            {
                Builder.CreateStore(val, nameMapInt[Var]);
            }
            else
            {
                Value* Zero = ConstantInt::get(Type::getInt32Ty(M->getContext()), 0);
                Builder.CreateStore(Zero, nameMapInt[Var]);
            }
            
        }
        virtual void visit(DefBool& Node) override  
        {
            Value* val = nullptr;                                 // defbool

            if (Node.getRValue()->getKind() == Expression::ExpressionType::BooleanOpType || Node.getRValue()->isBoolean())
            {
                // If there is an expression provided, visit it and get its value.
                Node.getRValue()->accept(*this);
                val = V;
            }

            // Iterate over the variables declared in the declaration statement.

            auto I = Node.getLValue()->getValue();
            StringRef Var = I;

            // Create an alloca instruction to allocate memory for the variable.
            nameMapBool[Var] = Builder.CreateAlloca(Int1Ty);                               //befahmim

            // Store the initial value (if any) in the variable's memory location.
            if (val != nullptr)
            {
                Builder.CreateStore(val, nameMapBool[Var]);
            }
            else
            {
                Value* Zero = ConstantInt::get(Type::getInt32Ty(M->getContext()), 0);
                Builder.CreateStore(Zero, nameMapBool[Var]);
            }
            
        }

        virtual void visit(AssignStatement& Node) override
        {
            // Visit the right-hand side of the assignment and get its value.
            Node.getRValue()->accept(*this);
            Value* val = V;

            // Get the name of the variable being assigned.
            auto varName = Node.getLValue()->getValue();

            // Create a store instruction to assign the value to the variable.
            Builder.CreateStore(val, nameMapInt[varName]);    ////???????
            

        }

        virtual void visit(IfStatement& Node) override
        {
            llvm::BasicBlock* IfCondBB = llvm::BasicBlock::Create(M->getContext(), "if.cond", MainFn);

            llvm::BasicBlock* IfBodyBB = llvm::BasicBlock::Create(M->getContext(), "if.body", MainFn);

            llvm::BasicBlock* AfterIfBB = llvm::BasicBlock::Create(M->getContext(), "after.if", MainFn);

            Builder.CreateBr(IfCondBB);
            Builder.SetInsertPoint(IfCondBB);

            Node.getCondition()->accept(*this);
            Value* Cond = V;

            Builder.SetInsertPoint(IfBodyBB);

            llvm::SmallVector<Statement*> stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }

            Builder.CreateBr(AfterIfBB);

            llvm::BasicBlock* BeforeCondBB = IfCondBB;

            llvm::BasicBlock* BeforeBodyBB = IfBodyBB;

            llvm::Value* BeforeCondVal = Cond;

            if(Node.hasElseIf())  
            {
                for (auto& ElseIf : Node.getElseIfsStatements()) {
                    llvm::BasicBlock* ElifCondBB = llvm::BasicBlock::Create(MainFn->getContext(), "elif.cond", MainFn); //

                    llvm::BasicBlock* ElifBodyBB = llvm::BasicBlock::Create(MainFn->getContext(), "elif.body", MainFn); //

                    Builder.SetInsertPoint(BeforeCondBB);

                    Builder.CreateCondBr(BeforeCondVal, BeforeBodyBB, ElifCondBB);

                    Builder.SetInsertPoint(ElifCondBB);
                    ElseIf->getCondition()->accept(*this);
                    llvm::Value* ElifCondVal = V;

                    Builder.SetInsertPoint(ElifBodyBB);
                    ElseIf->accept(*this);
                    Builder.CreateBr(AfterIfBB);

                    BeforeCondBB = ElifCondBB;
                    BeforeCondVal = ElifCondVal;
                    BeforeBodyBB = ElifBodyBB;
                }
            }

            llvm::BasicBlock* ElseBB = nullptr;
            if (Node.hasElse()) 
            {
                ElseStatement* elseS = Node.getElseStatement();
                ElseBB = llvm::BasicBlock::Create(MainFn->getContext(), "else.body", MainFn);
                Builder.SetInsertPoint(ElseBB);
                Node.getElseStatement()->accept(*this);
                Builder.CreateBr(AfterIfBB);

                Builder.SetInsertPoint(BeforeCondBB);
                Builder.CreateCondBr(Cond, IfBodyBB, ElseBB);

            } else {

                Builder.SetInsertPoint(BeforeCondBB);
                Builder.CreateCondBr(Cond, IfBodyBB, AfterIfBB);

            }

            Builder.SetInsertPoint(AfterIfBB);
        }

        virtual void visit(ElseIfStatement& Node) override   
        {
            llvm::SmallVector<Statement* > stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        }

        virtual void visit(ElseStatement& Node) override
        {
            llvm::SmallVector<Statement* > stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }
        }

        virtual void visit(WhileStatement& Node) override       
        {

            llvm::BasicBlock* WhileCondBB = llvm::BasicBlock::Create(M->getContext(), "while.cond", MainFn);
            // The basic block for the while body.
            llvm::BasicBlock* WhileBodyBB = llvm::BasicBlock::Create(M->getContext(), "while.body", MainFn);
            // The basic block after the while statement.
            llvm::BasicBlock* AfterWhileBB = llvm::BasicBlock::Create(M->getContext(), "after.loop", MainFn);

            // Branch to the condition block.
            Builder.CreateBr(WhileCondBB);

            // Set the insertion point to the condition block.
            Builder.SetInsertPoint(WhileCondBB);

            // Visit the condition expression and create the conditional branch.
            Node.getCondition()->accept(*this);
            Value* Cond = V;
            Builder.CreateCondBr(Cond, WhileBodyBB, AfterWhileBB);

            // Set the insertion point to the body block.
            Builder.SetInsertPoint(WhileBodyBB);

            llvm::SmallVector<Statement* > stmts = Node.getStatements();
            for (auto I = stmts.begin(), E = stmts.end(); I != E; ++I)
            {
                (*I)->accept(*this);
            }

            // Branch back to the condition block.
            Builder.CreateBr(WhileCondBB);

            // Set the insertion point to the block after the while loop.
            Builder.SetInsertPoint(AfterWhileBB);


        }

         virtual void visit(ForStatement& Node) override   
         {
            
             llvm::BasicBlock* ForCondBB = llvm::BasicBlock::Create(M->getContext(), "for.cond", Builder.GetInsertBlock()->getParent());
            // The basic block for the while body.
            llvm::BasicBlock* ForBodyBB = llvm::BasicBlock::Create(M->getContext(), "for.body", Builder.GetInsertBlock()->getParent());
            // The basic block after the while statement.
            llvm::BasicBlock* AfterForBB = llvm::BasicBlock::Create(M->getContext(), "after.loop", Builder.GetInsertBlock()->getParent());

            Node.getFirst()->accept(*this);

            Builder.CreateBr(ForCondBB); //?

            Builder.SetInsertPoint(ForCondBB);
            Node.getSecond()->accept(*this);
            Value* val=V;
            Builder.CreateCondBr(val,ForBodyBB,AfterForBB);

            Builder.SetInsertPoint(ForBodyBB);
            for(llvm::SmallVector<AST*>::const_iterator I = Node.begin(),E = Node.end();I !=E; ++I)
            {
                (*I)->accept(*this);
            }
            if(Node.getThirdAssign()==nullptr)
                Node.getThirdUnary()->accept(*this);
            else
                Node.getThirdAssign()->accept(*this);

            Builder.CreateBr(ForCondBB);

            Builder.SetInsertPoint(AfterForBB);

         }
         virtual void visit(Print &Node) override
        {
         // Visit the right-hand side of the assignment and get its value.
             Node.getExpr()->accept(*this);
             Value *val = V;

         // Create a call instruction to invoke the "print" function with the value.
            CallInst *Call = Builder.CreateCall(CalcWriteFnTy, CalcWriteFn, {val});
         }

         



    };
}; // namespace

void CodeGen::compile(AST* Tree)
{
    // Create an LLVM context and a module.
    LLVMContext Ctx;
    Module* M = new Module("mas.expr", Ctx);

    // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR.
    ToIRVisitor ToIRn(M);

    ToIRn.run(Tree);

    // Print the generated module to the standard output.
    M->print(outs(), nullptr);
}
