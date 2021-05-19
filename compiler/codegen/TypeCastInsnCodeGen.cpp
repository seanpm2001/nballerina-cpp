/*
 * Copyright (c) 2021, WSO2 Inc. (http://www.wso2.org) All Rights Reserved.
 *
 * WSO2 Inc. licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "bir/Function.h"
#include "bir/Operand.h"
#include "bir/TypeCastInsn.h"
#include "bir/Types.h"
#include "bir/Variable.h"
#include "codegen/CodeGenUtils.h"
#include "codegen/NonTerminatorInsnCodeGen.h"

namespace nballerina {

void NonTerminatorInsnCodeGen::visit(class TypeCastInsn &obj, llvm::IRBuilder<> &builder) {
    const auto &funcObj = obj.getFunctionRef();
    auto *rhsOpRef = functionGenerator.getLocalOrGlobalVal(obj.rhsOp);
    auto *lhsOpRef = functionGenerator.getLocalOrGlobalVal(obj.lhsOp);
    auto *lhsTypeRef = lhsOpRef->getType();

    const auto &lhsVar = funcObj.getLocalOrGlobalVariable(obj.lhsOp);
    const auto &rhsVar = funcObj.getLocalOrGlobalVariable(obj.rhsOp);
    const auto &lhsType = lhsVar.getType();
    auto lhsTypeTag = lhsType.getTypeTag();
    const auto &rhsType = rhsVar.getType();
    auto rhsTypeTag = rhsType.getTypeTag();
    auto &module = moduleGenerator.getModule();

    if (Type::isSmartStructType(rhsTypeTag)) {
        if (Type::isSmartStructType(lhsTypeTag)) {
            auto *rhsVarOpRef = functionGenerator.createTempVal(obj.rhsOp, builder);
            builder.CreateStore(rhsVarOpRef, lhsOpRef);
            return;
        }
        if (lhsTypeTag == TYPE_TAG_BOOLEAN) {
            auto *rhsValueRef = llvm::dyn_cast<llvm::Instruction>(builder.CreateLoad(rhsOpRef, ""));
            auto *namedFuncRef = CodeGenUtils::getAnyToBoolFunction(module);
            auto *callResult = builder.CreateCall(namedFuncRef, llvm::ArrayRef<llvm::Value *>({rhsValueRef}));
            builder.CreateStore(callResult, lhsOpRef);
        } else {
            llvm_unreachable("unsupported type");
        }

    } else if (Type::isSmartStructType(lhsTypeTag)) {
        if (rhsTypeTag == TYPE_TAG_BOOLEAN) {
            auto *rhsValueRef = llvm::dyn_cast<llvm::Instruction>(builder.CreateLoad(rhsOpRef, ""));
            auto *namedFuncRef = CodeGenUtils::getBoolToAnyFunction(module);
            auto *callResult = builder.CreateCall(namedFuncRef, llvm::ArrayRef<llvm::Value *>({rhsValueRef}));
            builder.CreateStore(callResult, lhsOpRef);
        } else {
            llvm_unreachable("unsupported type");
        }

    } else if (lhsTypeTag == TYPE_TAG_INT && rhsTypeTag == TYPE_TAG_FLOAT) {
        auto *rhsLoad = builder.CreateLoad(rhsOpRef);
        auto *lhsLoad = builder.CreateLoad(lhsOpRef);
        auto *castResult = builder.CreateFPToSI(rhsLoad, lhsLoad->getType(), "");
        builder.CreateStore(castResult, lhsOpRef);
    } else {
        builder.CreateBitCast(rhsOpRef, lhsTypeRef, "data_cast");
    }
}

} // namespace nballerina
