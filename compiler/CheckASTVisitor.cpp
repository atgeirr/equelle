/*
  Copyright 2014 SINTEF ICT, Applied Mathematics.
*/

#include "CheckASTVisitor.hpp"
#include "ASTNodes.hpp"
#include "SymbolTable.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>


CheckASTVisitor::CheckASTVisitor()
{
}

CheckASTVisitor::~CheckASTVisitor()
{
}


void CheckASTVisitor::visit(SequenceNode&)
{
}

void CheckASTVisitor::midVisit(SequenceNode&)
{
}

void CheckASTVisitor::postVisit(SequenceNode&)
{
}

void CheckASTVisitor::visit(NumberNode&)
{
}

void CheckASTVisitor::visit(QuantityNode&)
{
}

void CheckASTVisitor::postVisit(QuantityNode&)
{
}

void CheckASTVisitor::visit(UnitNode&)
{
}

void CheckASTVisitor::visit(StringNode&)
{
}

void CheckASTVisitor::visit(TypeNode&)
{
}

void CheckASTVisitor::visit(FuncTypeNode&)
{
}

void CheckASTVisitor::visit(BinaryOpNode&)
{
}

void CheckASTVisitor::midVisit(BinaryOpNode&)
{
}

void CheckASTVisitor::postVisit(BinaryOpNode& node)
{
    const EquelleType lt = node.left()->type();
    const EquelleType rt = node.right()->type();
    const BinaryOp op = node.op();
    if (!isNumericType(lt.basicType()) || !(isNumericType(rt.basicType()))) {
        error("arithmetic binary operators only apply to numeric types");
    }
    if (lt.isArray() || rt.isArray()) {
        error("arithmetic binary operators cannot be applied to Array types");
    }
    if (lt.isCollection() && rt.isCollection()) {
        if (lt.gridMapping() != rt.gridMapping()) {
            error("arithmetic binary operators on Collections only acceptable "
                    "if both sides are On the same set.");
        }
    }
    switch (op) {
    case Add:
        // Intentional fall-through.
    case Subtract:
        if (lt != rt) {
            if ((lt.basicType() == StencilI || lt.basicType() == StencilJ || lt.basicType() == StencilK)
                && rt.basicType() == Scalar) {
                //i,j,k OP n is OK
            }
            else if (lt.basicType() == Scalar &&
                     (rt.basicType() == StencilI || rt.basicType() == StencilJ || rt.basicType() == StencilK)) {
                //n OP i,j,k is OK
            }
            else if ((lt.isStencil() && rt.basicType() == Scalar)
                     || (rt.isStencil() && lt.basicType() == Scalar)) {
                //n OP u(i, j)  and  u(i, j) OP n is OK
            }
            else {
                error("addition and subtraction only allowed between identical types.");
            }
        }
        if (node.left()->dimension() != node.right()->dimension()) {
            error("addition and subtraction only allowed when both sides have same dimension.");
        }
        break;
    case Multiply:
        if (lt.basicType() == Vector && rt.basicType() == Vector) {
            error("cannot multiply two 'Vector' types.");
        }
        break;
    case Divide:
        if (rt.basicType() != Scalar) {
            error("can only divide by 'Scalar' types");
        }
        break;
    default:
        error("internal compiler error in CheckASTVisitor::postVisit(BinaryOpNode&).");
    }
}

void CheckASTVisitor::visit(ComparisonOpNode&)
{
}

void CheckASTVisitor::midVisit(ComparisonOpNode&)
{
}

void CheckASTVisitor::postVisit(ComparisonOpNode&)
{
}

void CheckASTVisitor::visit(NormNode&)
{
}

void CheckASTVisitor::postVisit(NormNode&)
{
}

void CheckASTVisitor::visit(UnaryNegationNode&)
{
}

void CheckASTVisitor::postVisit(UnaryNegationNode&)
{
}

void CheckASTVisitor::visit(OnNode&)
{
}

void CheckASTVisitor::midVisit(OnNode&)
{
}

void CheckASTVisitor::postVisit(OnNode&)
{
}

void CheckASTVisitor::visit(TrinaryIfNode&)
{
}

void CheckASTVisitor::questionMarkVisit(TrinaryIfNode&)
{
}

void CheckASTVisitor::colonVisit(TrinaryIfNode&)
{
}

void CheckASTVisitor::postVisit(TrinaryIfNode&)
{
}

void CheckASTVisitor::visit(VarDeclNode& node)
{
    if (SymbolTable::isVariableDeclared(node.name())
        || SymbolTable::isFunctionDeclared(node.name())) {
        std::string err = "cannot redeclare ";
        err += node.name();
        err += ": already declared.";
        error(err);
    }
    SymbolTable::declareVariable(node.name(), node.type());
}

void CheckASTVisitor::postVisit(VarDeclNode&)
{
}

void CheckASTVisitor::visit(VarAssignNode&)
{
}

void CheckASTVisitor::postVisit(VarAssignNode& node)
{
    const std::string& name = node.name();
    const ExpressionNode* expr = node.rhs();

    // Check if already declared as function.
    if (SymbolTable::isFunctionDeclared(name)) {
        std::string err_msg = "cannot declare variable ";
        err_msg += name;
        err_msg += ": already declared as function.";
        error(err_msg);
        return;
    }
    // If already declared...
    if (SymbolTable::isVariableDeclared(name)) {
        // Check if already assigned.
        if (SymbolTable::isVariableAssigned(name) && !SymbolTable::variableType(name).isMutable()) {
            std::string err_msg = "variable already assigned, cannot re-assign ";
            err_msg += name;
            error(err_msg);
            return;
        }
        // Check that declared type matches right hand side.
        EquelleType lhs_type = SymbolTable::variableType(name);
        EquelleType rhs_type = expr->type();
        if (lhs_type != rhs_type) {
            // Check for special case: variable declared to have type
            // 'Collection Of <Entity> Subset Of <Some entityset>',
            // actual setting its grid mapping is postponed until the entityset
            // has been created by a function call.
            // That means we have to set the actual grid mapping here.
            if (lhs_type.gridMapping() == PostponedDefinition
                && lhs_type.basicType() == rhs_type.basicType()
                && lhs_type.isCollection() && rhs_type.isCollection()
                && lhs_type.isStencil() == rhs_type.isStencil()
                && rhs_type.isDomain()
                && SymbolTable::isSubset(rhs_type.gridMapping(), lhs_type.subsetOf())) {
                // OK, should make postponed definition of the variable.
                SymbolTable::setVariableType(name, rhs_type);
                SymbolTable::setVariableDimension(name, expr->dimension());
                SymbolTable::setEntitySetName(rhs_type.gridMapping(), name);
            } else {
                std::string err_msg = "mismatch between type in assignment and declaration for ";
                err_msg += name;
                error(err_msg);
                return;
            }
        }
    } else {
        // Setting the gridmapping, as in the block above. Only this is
        // the direct (no previous declaration) assignment scenario.
        const int gm = expr->type().gridMapping();
        if (gm != NotApplicable && SymbolTable::entitySetName(gm) == "AnonymousEntitySet") {
            SymbolTable::setEntitySetName(gm, name);
        }
        SymbolTable::declareVariable(name, expr->type());
        SymbolTable::setVariableDimension(name, expr->dimension());
    }

    // Set variable to assigned (unless mutable) and return.
    if (!SymbolTable::variableType(name).isMutable()) {
        SymbolTable::setVariableAssigned(name, true);
    }
}

void CheckASTVisitor::visit(VarNode& node)
{
    if (!SymbolTable::isVariableDeclared(node.name())) {
        std::string err_msg = "using undeclared variable ";
        err_msg += node.name();
        error(err_msg);
    } else if (!SymbolTable::isVariableAssigned(node.name())) {
        std::string err_msg = "using unassigned variable ";
        err_msg += node.name();
        error(err_msg);
    }
}

void CheckASTVisitor::visit(FuncRefNode&)
{
}

void CheckASTVisitor::visit(JustAnIdentifierNode&)
{
}

void CheckASTVisitor::visit(FuncArgsDeclNode&)
{
}

void CheckASTVisitor::midVisit(FuncArgsDeclNode&)
{
}

void CheckASTVisitor::postVisit(FuncArgsDeclNode&)
{
}

void CheckASTVisitor::visit(FuncDeclNode&)
{
}

void CheckASTVisitor::postVisit(FuncDeclNode&)
{
}

void CheckASTVisitor::visit(FuncStartNode&)
{
}

void CheckASTVisitor::postVisit(FuncStartNode&)
{
}

void CheckASTVisitor::visit(FuncAssignNode&)
{
}

void CheckASTVisitor::postVisit(FuncAssignNode&)
{
}

void CheckASTVisitor::visit(FuncArgsNode&)
{
}

void CheckASTVisitor::midVisit(FuncArgsNode&)
{
}

void CheckASTVisitor::postVisit(FuncArgsNode&)
{
}

void CheckASTVisitor::visit(ReturnStatementNode&)
{
}

void CheckASTVisitor::postVisit(ReturnStatementNode&)
{
}

void CheckASTVisitor::visit(FuncCallNode&)
{
}

void CheckASTVisitor::postVisit(FuncCallNode& node)
{
    const Function& f = SymbolTable::getFunction(node.name());
    // Check function call arguments.
    const auto& argtypes = node.args()->argumentTypes();
    const auto& fargs = f.functionType().arguments();
    if (argtypes.size() != fargs.size()) {
        std::string err_msg = "wrong number of arguments when calling function ";
        err_msg += node.name();
        error(err_msg);
    }
    for (int arg = 0; arg < argtypes.size(); ++arg) {
        if (!argtypes[arg].canSubstituteFor(fargs[arg].type())) {
            std::ostringstream err;
            err << "wrong argument type for argument " << arg << " named '"
                << fargs[arg].name() << "' when calling function " << node.name()
                << ", expected " << SymbolTable::equelleString(fargs[arg].type())
                << " but got " << SymbolTable::equelleString(argtypes[arg]);
            error(err.str());
        }
    }
    // If the function returns a new dynamically created domain,
    // we must declare it (anonymously for now).
    const EquelleType rtype = f.returnType(argtypes);
    if (rtype.isDomain()) {
        const int dynsubret = f.functionType().dynamicSubsetReturn(argtypes);
        if (dynsubret != NotApplicable) {
            // Create a new domain.
            const int gm = SymbolTable::declareNewEntitySet("AnonymousEntitySet", dynsubret);
            (void) gm; // TODO use this, but where?
        }
    }
}

void CheckASTVisitor::visit(FuncCallStatementNode&)
{
}

void CheckASTVisitor::postVisit(FuncCallStatementNode&)
{
}

void CheckASTVisitor::visit(LoopNode&)
{
}

void CheckASTVisitor::postVisit(LoopNode&)
{
}

void CheckASTVisitor::visit(ArrayNode&)
{
}

void CheckASTVisitor::postVisit(ArrayNode&)
{
}

void CheckASTVisitor::visit(RandomAccessNode&)
{
}

void CheckASTVisitor::postVisit(RandomAccessNode&)
{
}

void CheckASTVisitor::visit(StencilAssignmentNode&)
{
}

void CheckASTVisitor::midVisit(StencilAssignmentNode&)
{
}

void CheckASTVisitor::postVisit(StencilAssignmentNode&)
{
}

void CheckASTVisitor::visit(StencilNode&)
{
}

void CheckASTVisitor::postVisit(StencilNode&)
{
}

void CheckASTVisitor::error(const std::string& err, const int line)
{
    std::cerr << "Parser error near line " << line << ": " << err << std::endl;
}
