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
#include <from_cvm_opcodes.h>

#include <stdlib.h>
#include <string.h>

FCVMOCtx* FCVMOCtxInit(const Opcode* opcodes, size_t opcodes_len,
                       i8* out, size_t max_out) {
  FCVMOCtx* ctx = (FCVMOCtx*)malloc(sizeof(FCVMOCtx));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->opcodes = opcodes;
  ctx->opcodes_len = opcodes_len;
  ctx->out = out;
  ctx->max_out = max_out;

  return ctx;
}

void FCVMOCtxFree(FCVMOCtx* ctx) {
  free(ctx);
}

/* This itoa function stolen from C_JSON, see:
   https://github.com/TheCRatTime/C_JSON/blob/main/include/c_json.h */

#define WriteToBuf(chr) \
  if (wrote < len) { \
    buf[wrote] = chr; \
  } else { \
    return wrote; \
  } \
  wrote++ 

static size_t U32ToA(u32 num, i8* buf, size_t len) {
  size_t wrote = 0;
  size_t start = 0;
  size_t end = 0;

  if (num == 0 || num <= 9) {
    WriteToBuf((i8)num + '0');
    WriteToBuf('\0');
    return wrote;
  }
  
  for(; num != 0; num /= 10) {
    WriteToBuf((char)(num % 10) + '0');
  }

  WriteToBuf('\0');

  start = 0;
  end = wrote - 2;

  while(end >= start) {
    i8 temp = buf[start];
    buf[start] = buf[end];
    buf[end] = temp;
    start++;
    end--;
  }

  return wrote;
}

#undef WriteToBuf

static void WriteOpcode(i8* cursor, i32* left, const Opcode* opcode) {
  u32 num = 0;

  switch (*opcode) {
    case OPCODE_NOP:
      strncat((char*)cursor, "nop ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_ADD:
      strncat((char*)cursor, "add ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_SUB:
      strncat((char*)cursor, "sub ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_PRINT:
      strncat((char*)cursor, "print ", (size_t)*left);
      *left -= 6;
      break;
    case OPCODE_HALT:
      strncat((char*)cursor, "halt ", (size_t)*left);
      *left -= 5;
      break;
    case OPCODE_MOV:
      strncat((char*)cursor, "mov ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_EOF:
      strncat((char*)cursor, "eof ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_NUM:
      memcpy(&num, opcode+1, 4);
      *left -= U32ToA(num, cursor, (size_t)*left);
      break;
    case OPCODE_EAX:
      strncat((char*)cursor, "eax ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_EBX:
      strncat((char*)cursor, "ebx ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_DEC:
      strncat((char*)cursor, "dec ", (size_t)*left);
      *left -= 4;
      break;
    case OPCODE_INC:
      strncat((char*)cursor, "inc ", (size_t)*left);
      *left -= 4;
      break;
    default:
      break;
  }
}

void FromCVMOpcodes(FCVMOCtx* ctx) {
  const i8* end = NULL;
  const Opcode* op_end = NULL;
  const Opcode* cur_op = NULL;
  i8* cursor = NULL;
  i32 left = 0;
  
  if (ctx == NULL || ctx->opcodes == NULL || ctx->opcodes_len == 0 ||
      ctx->out == NULL || ctx->max_out == 0) {
    return;
  }

  end = &ctx->out[ctx->max_out-1];
  op_end = &ctx->opcodes[ctx->opcodes_len-1];
  cur_op = ctx->opcodes;
  cursor = ctx->out;
  left = (i32)ctx->max_out-1;

  while (cur_op < op_end && cursor < end && left > 0) {
    i32 prev_left = left;
    WriteOpcode(cursor, &left, cur_op);
    if (prev_left < left) {
      break;
    }

    cursor += prev_left - left;
    if (*cur_op == OPCODE_NUM) {
      cur_op += 4;
    }
    cur_op++;
  }

  for (cursor = ctx->out; cursor < end; cursor++) {
    if (*cursor == '\0') {
      *cursor = ' ';
    }
  }
}
