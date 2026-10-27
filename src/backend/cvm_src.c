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

#include <cvm_src.h>

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CVM* InitCVM(void) {
  CVM* vm = (CVM*)malloc(sizeof(CVM));

  if (!vm) {
    return NULL;
  }

  vm->cur_op = OPCODE_NOP;
  vm->eax = 0;
  vm->ebx = 0;

  return vm;
}

void FreeCVM(CVM* vm) {
  free(vm);
}

static u8 IsRegister(const u8 opcode) {
  return (opcode == OPCODE_EAX || opcode == OPCODE_EBX);
}

static const u8* Peek(const u8* opcodes) {
  return (*opcodes == OPCODE_EOF ? opcodes : (opcodes+1));
}

static u8 NopHandle(u8* offset) {
  (*offset)++;
  return 0;
}

static u8 AddHandle(CVM* vm, const u8* opcodes, u8* offset) {
  const u8* next = NULL;
  const u8* next2 = NULL;
  u32 num = 0;
  
  next = Peek(opcodes);
  if (*next == OPCODE_EOF) {
    goto expected;
  }
  
  next2 = Peek(next);

  if (!IsRegister(*next) || (*next2 != OPCODE_NUM &&
      !IsRegister(*next2))) {
  expected:
    fputs("Expected register name and number/register name"
          " after OPCODE_ADD\n", stdout);
    return 1;
  }

  if (IsRegister(*next2)) {
    if (*next == OPCODE_EAX) {
      vm->eax += vm->ebx;
    } else {
      vm->ebx += vm->eax;
    }

    (*offset) += 3;
    return 0;
  } else {
    if (*Peek(next2) == OPCODE_EOF) {
      fputs("Expected number after OPCODE_NUM\n", stdout);
      return 1;
    }
    
    memcpy(&num, Peek(next2), sizeof(u32));
    if (*next == OPCODE_EAX) {
      vm->eax += num;
    } else {
      vm->ebx += num;
    }

    (*offset) += 2 + sizeof(u32);
    return 0;
  }
}

static u8 SubHandle(CVM* vm, const u8* opcodes, u8* offset) {
  const u8* next = NULL;
  const u8* next2 = NULL;
  u32 num = 0;
  
  next = Peek(opcodes);
  if (*next == OPCODE_EOF) {
    goto expected;
  }
  
  next2 = Peek(next);

  if (!IsRegister(*next) || (*next2 != OPCODE_NUM &&
      !IsRegister(*next2))) {
  expected:
    fputs("Expected register name and number/register name"
          " after OPCODE_SUB\n", stdout);
    return 1;
  }

  if (IsRegister(*next2)) {
    if (*next == OPCODE_EAX) {
      vm->eax -= vm->ebx;
    } else {
      vm->ebx -= vm->eax;
    }

    (*offset) += 3;
    return 0;
  } else {
    if (*Peek(next2) == OPCODE_EOF) {
      fputs("Expected number after OPCODE_NUM\n", stdout);
      return 1;
    }
    
    memcpy(&num, Peek(next2), sizeof(u32));
    if (*next == OPCODE_EAX) {
      vm->eax -= num;
    } else {
      vm->ebx -= num;
    }

    (*offset) += 2 + sizeof(u32);
    return 0;
  }
}

static u8 PrintHandle(CVM* vm, const u8* opcodes, u8* offset) {
  const u8* next = Peek(opcodes);
  
  if (!IsRegister(*next)) {
    fputs("Expected register name after OPCODE_PRINT\n", stdout);
    return 1;
  }

  printf(U32PRINTF"\n", (*next == OPCODE_EAX ? vm->eax : vm->ebx));
  (*offset) += 2;
  return 0;
}

static u8 MovHandle(CVM* vm, const u8* opcodes, u8* offset) {
  const u8* next = NULL;
  const u8* next2 = NULL;
  u32 num = 0;
  
  next = Peek(opcodes);
  if (*next == OPCODE_EOF) {
    goto expected;
  }
  
  next2 = Peek(next);

  if (!IsRegister(*next) || (*next2 != OPCODE_NUM &&
      !IsRegister(*next2))) {
  expected:
    fputs("Expected register name and number/register name"
          " after OPCODE_MOV\n", stdout);
    return 1;
  }

  if (IsRegister(*next2)) {
    if (*next == OPCODE_EAX) {
      vm->eax = vm->ebx;
    } else {
      vm->ebx = vm->eax;
    }

    (*offset) += 2;
    return 0;
  } else {
    if (*Peek(next2) == OPCODE_EOF) {
      fputs("Expected number after OPCODE_NUM\n", stdout);
      return 1;
    }
    
    memcpy(&num, Peek(next2), sizeof(u32));
    if (*next == OPCODE_EAX) {
      vm->eax = num;
    } else {
      vm->ebx = num;
    }

    (*offset) += 2 + sizeof(u32);
    return 0;
  }
}

static u8 InterruptHandle(void) {
  printf("Interrupt...\n");
  return 10;
}

static u8 InvalidHandle(CVM* vm) {
  printf("Invalid opcode: 0x%02X\n", vm->cur_op);
  return 1;
}

static u8 IncHandle(CVM* vm, const u8* opcodes, u8* offset) {
  const u8* next = Peek(opcodes);
  
  if (!IsRegister(*next)) {
    fputs("Expected register name after OPCODE_INC\n", stdout);
    return 1;
  }

  if (*next == OPCODE_EAX) {
    vm->eax++;
  } else {
    vm->ebx++;
  }
  
  (*offset) += 2;
  return 0;
}

static u8 DecHandle(CVM* vm, const u8* opcodes, u8* offset) {
  const u8* next = Peek(opcodes);
  
  if (!IsRegister(*next)) {
    fputs("Expected register name after OPCODE_INC\n", stdout);
    return 1;
  }

  if (*next == OPCODE_EAX) {
    vm->eax++;
  } else {
    vm->ebx++;
  }
  
  (*offset) += 2;
  return 0;
}

static u8 EvalOpcode(CVM* vm, const u8* opcodes, u8* offset) {
  vm->cur_op = opcodes[0];

  switch (vm->cur_op) {
    case OPCODE_NOP:
      return NopHandle(offset);
    case OPCODE_ADD:
      return AddHandle(vm, opcodes, offset);
    case OPCODE_SUB:
      return SubHandle(vm, opcodes, offset);
    case OPCODE_PRINT:
      return PrintHandle(vm, opcodes, offset);
    case OPCODE_MOV:
      return MovHandle(vm, opcodes, offset);
    case OPCODE_INC:
      return IncHandle(vm, opcodes, offset);
    case OPCODE_DEC:
      return DecHandle(vm, opcodes, offset);
    case OPCODE_HALT:
    case OPCODE_EOF:
      return InterruptHandle();
    case OPCODE_NUM:
    default:
      return InvalidHandle(vm);
  }
}

void CVMRun(CVM* vm, const u8* opcodes, size_t len) {
  u8 offset = 0;
  const u8* const end = &opcodes[len-1];
  
  if (vm == NULL || opcodes == NULL || len == 0) {
    return;
  }

  for (;;) {
    u8 code = EvalOpcode(vm, opcodes, &offset);
    if (code) {
      printf("VM exited with code %u\n", code);
      return;
    }

    opcodes += offset;
    if (opcodes > end) {
      fputs("VM exited with code 0\n", stdout);
      return;
    }
    
    offset = 0;
  }
}
