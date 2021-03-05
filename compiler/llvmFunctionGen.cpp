#include "BIR.h"

BIRFunction::BIRFunction() {}

BIRFunction::BIRFunction(Location *pos, string namep, int flagsp, InvokableType *typep, string workerNamep)
    : BIRNode(pos), name(namep), flags(flagsp), type(typep), workerName(workerNamep) {}

BIRFunction::BIRFunction(const BIRFunction &) {}

// Search basic block based on the basic block ID
BIRBasicBlock *BIRFunction::searchBb(std::string name) {
    std::vector<BIRBasicBlock *>::iterator itr;
    for (itr = basicBlocks.begin(); itr != basicBlocks.end(); itr++) {
        if ((*itr)->getId() == name) {
            return (*itr);
        }
    }
    return NULL;
}

LLVMValueRef BIRFunction::getValueRefBasedOnName(string lhsName) {
    map<string, LLVMValueRef>::iterator it;
    it = branchComparisonList.find(lhsName);

    if (it == branchComparisonList.end()) {
        return NULL;
    } else
        return it->second;
}

LLVMValueRef BIRFunction::getLocalVarRefUsingId(string locVar) {
    for (std::map<string, LLVMValueRef>::iterator iter = localVarRefs.begin(); iter != localVarRefs.end(); iter++) {
        if (iter->first == locVar)
            return iter->second;
    }
    return NULL;
}

LLVMValueRef BIRFunction::getLocalToTempVar(Operand *operand) {
    string refOp = operand->getVarDecl()->getVarName();
    string tempName = refOp + "_temp";
    LLVMValueRef locVRef = getLocalVarRefUsingId(refOp);
    if (!locVRef)
        locVRef = getPkgAddress()->getGlobalVarRefUsingId(refOp);
    return LLVMBuildLoad(builder, locVRef, tempName.c_str());
}

static bool isParamter(VarDecl *locVar) {
    switch (locVar->getVarKind()) {
    case LOCAL_VAR_KIND:
    case TEMP_VAR_KIND:
    case RETURN_VAR_KIND:
    case GLOBAL_VAR_KIND:
    case SELF_VAR_KIND:
    case CONSTANT_VAR_KIND:
        return false;
    case ARG_VAR_KIND:
        return true;
    default:
        return false;
    }
}

LLVMTypeRef BIRFunction::getLLVMFuncRetTypeRefOfType(VarDecl *vDecl) {
    int typeTag = 0;
    if (vDecl->getTypeDecl())
        typeTag = vDecl->getTypeDecl()->getTypeTag();
    // if main function return type is void, but user wants to return some
    // value using _bal_result (global variable from BIR), change main function
    // return type from void to global variable (_bal_result) type.
    if (typeTag == TYPE_TAG_NIL || typeTag == TYPE_TAG_VOID) {
        vector<VarDecl *> globVars = getPkgAddress()->getGlobalVars();
        for (unsigned int i = 0; i < globVars.size(); i++) {
            VarDecl *globVar = globVars[i];
            if (globVar->getVarName() == "_bal_result") {
                typeTag = globVar->getTypeDecl()->getTypeTag();
                break;
            }
        }
    }

    switch (typeTag) {
    case TYPE_TAG_INT:
        return LLVMInt32Type();
    case TYPE_TAG_BYTE:
    case TYPE_TAG_FLOAT:
        return LLVMFloatType();
    case TYPE_TAG_BOOLEAN:
        return LLVMInt8Type();
    case TYPE_TAG_CHAR_STRING:
    case TYPE_TAG_STRING:
        return LLVMPointerType(LLVMInt8Type(), 0);
    default:
        return LLVMVoidType();
    }
}

LLVMValueRef BIRFunction::generateAbortInsn(LLVMModuleRef &modRef) {
    BIRPackage *pkgObj = getPkgAddress();
    // calling the C stdlib.h abort() function.
    const char *abortFuncName = "abort";
    LLVMValueRef abortFuncRef = pkgObj->getFunctionRefBasedOnName(abortFuncName);
    if (!abortFuncRef) {
        LLVMTypeRef *paramTypes = new LLVMTypeRef[0];
        LLVMTypeRef funcType = LLVMFunctionType(LLVMVoidType(), paramTypes, 0, 0);
        abortFuncRef = LLVMAddFunction(modRef, abortFuncName, funcType);
        pkgObj->addArrayFunctionRef(abortFuncName, abortFuncRef);
    }
    return abortFuncRef;
}

void BIRFunction::translateFunctionBody(LLVMModuleRef &modRef) {
    LLVMBasicBlockRef BbRef;
    int paramIndex = 0;
    BbRef = LLVMAppendBasicBlock(newFunction, "entry");
    LLVMPositionBuilderAtEnd(builder, BbRef);

    // iterate through all local vars.
    for (unsigned int i = 0; i < localVars.size(); i++) {
        VarDecl *locVar = localVars[i];
        const char *varName = (locVar->getVarName()).c_str();
        LLVMTypeRef varType = getLLVMTypeRefOfType(locVar->getTypeDecl());
        LLVMValueRef localVarRef;
        if (locVar->getTypeDecl()->getTypeTag() == TYPE_TAG_ANY) {
            varType = wrap(getPkgAddress()->getStructType());
        }
        localVarRef = LLVMBuildAlloca(builder, varType, varName);
        localVarRefs.insert({locVar->getVarName(), localVarRef});
        if (isParamter(locVar)) {
            LLVMValueRef parmRef = LLVMGetParam(newFunction, paramIndex);
            string paramName = getParam(paramIndex)->getVarDecl()->getVarName();
            LLVMSetValueName2(parmRef, paramName.c_str(), paramName.length());
            if (parmRef)
                LLVMBuildStore(builder, parmRef, localVarRef);
            paramIndex++;
        }
    }

    // iterate through with each basic block in the function and create them
    // first.
    for (unsigned int i = 0; i < basicBlocks.size(); i++) {
        BIRBasicBlock *bb = basicBlocks[i];
        LLVMBasicBlockRef bbRef = LLVMAppendBasicBlock(this->getNewFunctionRef(), bb->getId().c_str());
        bb->setLLVMBBRef(bbRef);
        bb->setBIRFunction(this);
        bb->setLLVMBuilderRef(builder);
        bb->setPkgAddress(getPkgAddress());
    }

    // creating branch to next basic block.
    if (basicBlocks.size() != 0 && basicBlocks[0] && basicBlocks[0]->getLLVMBBRef())
        LLVMBuildBr(builder, basicBlocks[0]->getLLVMBBRef());

    // Now translate the basic blocks (essentially add the instructions in them)
    for (unsigned int i = 0; i < basicBlocks.size(); i++) {
        BIRBasicBlock *bb = basicBlocks[i];
        LLVMPositionBuilderAtEnd(builder, bb->getLLVMBBRef());
        bb->translate(modRef);
    }
}

// Splitting Basicblock after 'is_same_type()' function call and
// based on is_same_type() function result, crating branch condition using
// splitBB Basicblock (ifBB) and abortBB(elseBB).
// In IfBB we are doing casing and from ElseBB Aborting.
void BIRFunction::splitBBIfPossible(LLVMModuleRef &modRef) {
    unsigned totalBB = LLVMCountBasicBlocks(getNewFunctionRef());
    LLVMBasicBlockRef currentBB = LLVMGetFirstBasicBlock(getNewFunctionRef());
    for (unsigned i = 0; i < totalBB; i++) {
        BasicBlock *bBlock = unwrap(currentBB);
        for (BasicBlock::iterator I = bBlock->begin(); I != bBlock->end(); ++I) {
            CallInst *callInst = dyn_cast<CallInst>(&*I);
            if (!callInst) {
                continue;
            } else {
                string insnName;
                if (callInst->getNumOperands() == 2)
                    insnName = callInst->getOperand(1)->getName().data();
                else if (callInst->getNumOperands() == 3)
                    insnName = callInst->getOperand(2)->getName().data();
                if (insnName == "is_same_type") {
                    advance(I, 1);
                    Instruction *compInsn = &*I;
                    // Splitting BasicBlock.
                    BasicBlock *splitBB = bBlock->splitBasicBlock(++I, bBlock->getName() + ".split");
                    LLVMValueRef lastInsn = LLVMGetLastInstruction(wrap(bBlock));
                    // branch intruction to the split BB is creating in BB2 (last BB)
                    // basicblock, removing from BB2 and insert this branch instruction
                    // into BB0(split original BB).
                    LLVMInstructionRemoveFromParent(lastInsn);
                    // Creating abortBB (elseBB).
                    BasicBlock *elseBB = BasicBlock::Create(*unwrap(LLVMGetGlobalContext()), "abortBB");

                    // Creating Branch condition using if and else BB's.
                    Instruction *compInsnRef = unwrap(builder)->CreateCondBr(compInsn, splitBB, elseBB);

                    // branch to abortBB instruction is generating in last(e.g bb2 BB)
                    // basicblock. here, moving from bb2 to bb0.split basicblock.
                    compInsnRef->removeFromParent();
                    bBlock->getInstList().push_back(compInsnRef);

                    // get the last instruction from splitBB.
                    Instruction *newBBLastInsn;
                    BasicBlock::iterator I = splitBB->end();
                    if (I == splitBB->begin())
                        newBBLastInsn = NULL;
                    newBBLastInsn = &*--I;
                    BasicBlock *elseBBSucc = newBBLastInsn->getSuccessor(0);
                    // creating branch to else basicblock.
                    Instruction *brInsn = unwrap(builder)->CreateBr(elseBBSucc);
                    brInsn->removeFromParent();

                    // generate LLVMFunction call to Abort from elseLLVMBB(abortBB).
                    LLVMValueRef abortInsn = generateAbortInsn(modRef);
                    LLVMValueRef abortFuncCallInsn = LLVMBuildCall(builder, abortInsn, NULL, 0, "");
                    LLVMInstructionRemoveFromParent(abortFuncCallInsn);
                    // Inserting Abort Functioncall instruction into elseLLVMBB(abortBB).
                    elseBB->getInstList().push_back(unwrap<Instruction>(abortFuncCallInsn));
                    elseBB->getInstList().push_back(brInsn);
                    // Inserting elseLLVMBB (abort BB) after splitBB (bb0.split)
                    // basicblock.
                    splitBB->getParent()->getBasicBlockList().insertAfter(splitBB->getIterator(), elseBB);
                    currentBB = LLVMGetNextBasicBlock(currentBB);
                    break;
                }
            }
        }
        currentBB = LLVMGetNextBasicBlock(currentBB);
    }
}

void BIRFunction::translate(LLVMModuleRef &modRef) {
    translateFunctionBody(modRef);

    splitBBIfPossible(modRef);
}

BIRFunction::~BIRFunction() {}

LLVMTypeRef BIRFunction::getLLVMTypeRefOfType(TypeDecl *typeD) {
    int typeTag = typeD->getTypeTag();
    switch (typeTag) {
    case TYPE_TAG_INT:
        return LLVMInt32Type();
    case TYPE_TAG_BYTE:
    case TYPE_TAG_FLOAT:
        return LLVMFloatType();
    case TYPE_TAG_BOOLEAN:
        return LLVMInt8Type();
    case TYPE_TAG_CHAR_STRING:
    case TYPE_TAG_STRING:
    case TYPE_TAG_ARRAY:
        return LLVMPointerType(LLVMInt8Type(), 0);
    case TYPE_TAG_ANY:
        return LLVMPointerType(LLVMInt64Type(), 0);
    default:
        return LLVMInt32Type();
    }
}

VarDecl *BIRFunction::getNameVarDecl(string opName) {
    for (unsigned int i = 0; i < localVars.size(); i++) {
        VarDecl *variableDecl = localVars[i];
        string varName = variableDecl->getVarName();
        if (varName == opName) {
            return variableDecl;
        }
    }
    return NULL;
}

const char *BIRFunction::getTypeNameOfTypeTag(TypeTagEnum typeTag) {
    switch (typeTag) {
    case TYPE_TAG_INT:
        return "int";
    case TYPE_TAG_FLOAT:
        return "float";
    case TYPE_TAG_CHAR_STRING:
    case TYPE_TAG_STRING:
        return "string";
    case TYPE_TAG_BOOLEAN:
        return "bool";
    case TYPE_TAG_ANY:
        return "any";
    default:
        return "";
    }
}
