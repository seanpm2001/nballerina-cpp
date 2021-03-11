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

#include "ErrorTypeInsn.h"
#include "Function.h"
#include "Operand.h"
#include "Package.h"

using namespace std;
using namespace llvm;

namespace nballerina {

ErrorTypeInsn::ErrorTypeInsn(Operand *lOp, BasicBlock *currentBB, Operand *mOp, Operand *cOp, Operand *dOp, Type *tDecl)
    : NonTerminatorInsn(lOp, currentBB), msgOp(mOp), causeOp(cOp), detailOp(dOp), typeDecl(tDecl) {}

Operand *ErrorTypeInsn::getMsgOp() { return msgOp; }
Operand *ErrorTypeInsn::getCauseOp() { return causeOp; }
Operand *ErrorTypeInsn::getDetailOp() { return detailOp; }
Type *ErrorTypeInsn::getTypeDecl() { return typeDecl; }

void ErrorTypeInsn::setMsgOp(Operand *op) { msgOp = op; }
void ErrorTypeInsn::setCauseOp(Operand *op) { causeOp = op; }
void ErrorTypeInsn::setDetailOp(Operand *op) { detailOp = op; }
void ErrorTypeInsn::setTypeDecl(Type *tDecl) { typeDecl = tDecl; }

void ErrorTypeInsn::translate(LLVMModuleRef &modRef) {
    // Error Type Translation logic
}

} // namespace nballerina
