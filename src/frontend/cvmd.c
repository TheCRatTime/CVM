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

#include "types.h"
#define ATEXIT_SOURCE
#include <atexit.h>
#include <cvm_src.h>
#include <from_cvm_opcodes.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAIN_ARGV
#include <main_wrapper.h>

static void Help(void) {
  fputs("\
NAME\n\
    cvmd - decompile CVM Opcodes.\n\
\n\
SYNOPSIS\n\
    cvmd FILE\n\
\n\
DESCRIPTION\n\
    cvmd is a CVM Decompiler. Util converts CVM\n\
    Opcodes to human-readable string.\n", stdout);
}

static void FreeAtExit(void* ptr) {
  if (ptr) {
    AtExitFree((AtExitHeader*)ptr);
  }
}

static Opcode* RealGetOpcodes(AtExitHeader* atexit, const char* filename) {
  Opcode* ret = NULL;
  FILE* file = NULL;
  long file_len = 0;
  char* buf = NULL;
  size_t read = 0;

  file = AtExitFopen(atexit, filename, "rb");
  if (!file) {
    printf("cvmd: can't open %s\n", filename);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  file_len = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (file_len != (5 + (sizeof(Opcode) * 512)) ||
      file_len <= 0) {
    fputs("cvmd: invalid file size\n", stdout);
    return NULL;
  }

  buf = (char*)AtExitMalloc(atexit, (size_t)file_len+1);
  if (!buf) {
    fputs("cvmd: out of memory\n", stdout);
    return NULL;
  }

  buf[file_len] = '\0';

  read = fread(buf, 1, (size_t)file_len, file);
  if (read != (size_t)file_len) {
    fputs("cvmd: cannot read from file\n", stdout);
    return NULL;
  }

  if (strncmp(buf, "?CVMC", 5)) {
    fputs("cvmd: magic failure\n", stdout);
    return NULL;
  }

  ret = (Opcode*)malloc(sizeof(Opcode) * 512);
  if (!ret) {
    fputs("cvmd: out of memory\n", stdout);
    return NULL;
  }

  memcpy(ret, buf+5, sizeof(Opcode) * 512);
  
  return ret;
}

/* Internal bug fix */
typedef Opcode* opcodeptr;

#define GEN_NAME RealGetOpcodes
#define GEN_OUT  GetOpcodes
#define GEN_TYPE opcodeptr
#define GEN_ARGS GENARG(const char*, filename)
#include <gen_wrapper.h>

static int RealTryWriteToFile(AtExitHeader* atexit, const char* str,
                              size_t str_len) {
  FILE* out = NULL;
  char comment[28] = "DUMPED BY CVMD,BEGIN DUMP: ";
  size_t wrote = 0;

  out = AtExitFopen(atexit, "cvmd.dump", "wb");
  if (!out) {
    fputs("cvmd: can't open cvmd.dump\n", stdout);
    return 1;
  }

  wrote = fwrite(comment, 1, 28, out);
  if (wrote != 28) {
    fputs("cvmd: file write error\n", stdout);
    return 1;
  }

  wrote = fwrite(str, 1, str_len, out);
  if (wrote != str_len) {
    fputs("cvmd: file write error\n", stdout);
    return 1;
  }
  
  return 0;
}

#define GEN_NAME RealTryWriteToFile
#define GEN_OUT  TryWriteToFile
#define GEN_TYPE int
#define GEN_ARGS GENARG(const char*, str) GENARG(size_t, str_len)
#include <gen_wrapper.h>

int Main(AtExitHeader* main_atexit, int argc, char** argv) {
  AtExitHeader* temp_atexit = NULL;
  FCVMOCtx* ctx = NULL;
  Opcode* opcodes = NULL;
  char output[4096] = {0};
  
  if (argc < 2) {
    Help();
    return 1;
  }

  temp_atexit = InitAtExit(ATEXIT_DEFVAL);
  if (!temp_atexit) {
    fputs("cvmc: out of memory\n", stdout);
    return 1;
  }

  defer (main_atexit, FreeAtExit, temp_atexit);

  opcodes = GetOpcodes(temp_atexit, argv[1]);
  if (!opcodes) {
    return 1;
  }

  defer (main_atexit, free, opcodes);

  ctx = FCVMOCtxInit(opcodes, 512, (i8*)output, 4095);
  if (!ctx) {
    fputs("cvmc: out of memory\n", stdout);
    return 1;
  }

  FromCVMOpcodes(ctx);

  FCVMOCtxFree(ctx);

  if (TryWriteToFile(temp_atexit, output, 4096)) {
    return 1;
  }
  
  return 0;
}
