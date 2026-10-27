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
#include <to_cvm_opcodes.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static u32 StrToU32(const i8* string) {
#ifdef UINT_AS_U32
  return (u32)atoi((const char*)string);
#else
  return (u32)atol((const char*)string);
#endif
}

#define TOTAL_KEYWORDS 11

static u8 LikeKeyword(const i8* str) {
  u8 i = 0;
  const struct {
    char* name;
    u8 len;
  } Keywords[TOTAL_KEYWORDS] = {
    {"eax", 3},
    {"ebx", 3},
    {"print", 5},
    {"halt", 4},
    {"eof", 3},
    {"add", 3},
    {"sub", 3},
    {"mov", 3},
    {"nop", 3},
    {"dec", 3},
    {"inc", 3}
  };

  for (; i < TOTAL_KEYWORDS; i++) {
    if (!strncmp((const char*)str, Keywords[i].name, Keywords[i].len)) {
      return i+1;
    }
  }

  return 0;
}

static void KeywordParse(const i8* cursor, Opcode* cur_op, u8* offset) {
  u8 out_num = 0;
  const struct {
    Opcode code;
    u8 len;
  } Keywords[TOTAL_KEYWORDS] = {
    {OPCODE_EAX, 3},
    {OPCODE_EBX, 3},
    {OPCODE_PRINT, 5},
    {OPCODE_HALT, 4},
    {OPCODE_EOF, 3},
    {OPCODE_ADD, 3},
    {OPCODE_SUB, 3},
    {OPCODE_MOV, 3},
    {OPCODE_NOP, 3},
    {OPCODE_DEC, 3},
    {OPCODE_INC, 3}
  };

  out_num = LikeKeyword(cursor)-1;

  *cur_op = Keywords[out_num].code;
  *offset += Keywords[out_num].len;
}

#define SIZE_OF_U32 4

static void NumParse(const i8* cursor, Opcode* cur_op, u8* offset) {
  u8 i = 0;
  u32 num = 0;
  *cur_op = OPCODE_NUM;
  cur_op++;
  num = StrToU32(cursor);

  for (; i < SIZE_OF_U32; i++, cur_op++) {
    *cur_op = (num << (i << 3)) & 0xFF;
  }

  while (isdigit(*cursor)) {
    (*offset)++;
    cursor++;
  }
}

TCVMOCtx* TCVMOCtxInit(const i8* str, size_t string_len,
                       Opcode* opcodes, size_t opcodes_len) {
  TCVMOCtx* ctx = (TCVMOCtx*)malloc(sizeof(TCVMOCtx));
  if (ctx == NULL) {
    return NULL;
  }

  ctx->string = str;
  ctx->string_len = string_len;
  ctx->out = opcodes;
  ctx->max_out = opcodes_len;
  
  return ctx;
}

void TCVMOCtxFree(TCVMOCtx* ctx) {
  free(ctx);
}

void ToCVMOpcodes(TCVMOCtx* ctx) {
  const i8* end = NULL;
  const Opcode* op_end = NULL;
  const i8* cursor = NULL;
  Opcode* cur_op = NULL;
  u8 offset = 0;
  
  if (ctx == NULL || ctx->string == NULL || ctx->string_len == 0 ||
      ctx->out == NULL || ctx->max_out == 0) {
    return;
  }

  end = &ctx->string[ctx->string_len-1];
  op_end = &ctx->out[ctx->max_out-1];
  cursor = ctx->string;
  cur_op = ctx->out;

  while (cursor < end && cur_op < op_end) {
    if (LikeKeyword(cursor)) {
      KeywordParse(cursor, cur_op, &offset);
      cur_op++;
    } else if (isspace(*cursor) || *cursor == '\0') {
      offset++;
    } else if (isdigit(*cursor)) {
      NumParse(cursor, cur_op, &offset);
      cur_op += 5;
    } else {
      offset++;
    }
    
    cursor += offset;
    offset = 0;
  }

  if (cur_op < op_end) {
    *cur_op = OPCODE_HALT;
    cur_op++;
  }
  
  if (cur_op < op_end) {
    *cur_op = OPCODE_EOF;
    cur_op++;
  }
}
