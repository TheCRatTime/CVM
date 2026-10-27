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

#ifndef CVM_TO_CVM_OPCODES_H_
#define CVM_TO_CVM_OPCODES_H_

#include "types.h"

#include <stddef.h>

/* ToCVMOpcodes context struct */
typedef struct TCVMOCtx {
  const i8* string;
  size_t string_len;
  Opcode* out;
  size_t max_out;
} TCVMOCtx;

/* Initialize TCVMOCtx */
TCVMOCtx* TCVMOCtxInit(const i8*, size_t, Opcode*, size_t);

/* Free TCVMOCtx */
void TCVMOCtxFree(TCVMOCtx*);

/* Convert string to CVM opcodes by CTX. */
void ToCVMOpcodes(TCVMOCtx*);

#endif /* CVM_TO_CVM_OPCODES_H_ */
