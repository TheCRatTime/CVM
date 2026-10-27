/*
 * Copyright (C) 2026 TheCRatTime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CVM_FROM_CVM_OPCODES_H_
#define CVM_FROM_CVM_OPCODES_H_

#include "types.h"

#include <stddef.h>

/* FromCVMOpcodes context struct */
typedef struct FCVMOCtx {
  const Opcode* opcodes;
  size_t opcodes_len;
  i8* out;
  size_t max_out;
} FCVMOCtx;

/* Initialize FCVMOCtx */
FCVMOCtx* FCVMOCtxInit(const Opcode*, size_t, i8*, size_t);

/* Free FCVMOCtx */
void FCVMOCtxFree(FCVMOCtx*);

/* Convert CVM opcodes to string by CTX. */
void FromCVMOpcodes(FCVMOCtx*);

#endif /* CVM_FROM_CVM_OPCODES_H_ */
