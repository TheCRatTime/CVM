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

#ifndef CVM_CVM_H_
#define CVM_CVM_H_

#include "types.h"

#include <stddef.h>

/* === Defines === */
#define STACK_SIZE 4096
#define STACK_END  4095

/* === Opcodes === */
#define OPCODE_NOP   0x00
#define OPCODE_ADD   0x01
#define OPCODE_SUB   0x02
#define OPCODE_PRINT 0x03
#define OPCODE_HALT  0x04
#define OPCODE_MOV   0x05
#define OPCODE_EOF   0x06
#define OPCODE_NUM   0x07
#define OPCODE_INC   0x08
#define OPCODE_DEC   0x09

#define OPCODE_EAX   0x0A
#define OPCODE_EBX   0x0B

/* C Virtual Machine struct */
typedef struct CVM {
  Opcode cur_op;
  Register eax;
  Register ebx;
} CVM;

/* Initialize CVM */
CVM* InitCVM(void);

/* Free CVM */
void FreeCVM(CVM*);

/* Run OPCODES by VM */
void CVMRun(CVM*, const u8*, size_t);

#endif /* CVM_CVM_H_ */
