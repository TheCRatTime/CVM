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

#define ATEXIT_SOURCE
#include <atexit.h>
#include <cvm_src.h>
#include <to_cvm_opcodes.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define nil NULL

#define MAIN_ARGV
#include <main_wrapper.h>

static void Help(void) {
  fputs("\
NAME\n\
    cvmc - compile code to CVM op-codes.\n\
\n\
SYNOPSIS\n\
    cvmc FILE\n\
    cvmc -\n\
\n\
DESCRIPTION\n\
    cvmc is a CVM Compiler. Util compiles file\n\
    to CVM op-codes.\n", stdout);
}

static char* RealOpenFileAndInitBuf(AtExitHeader* atexit,
                                    const char* filename, long* out_len) {
  char* buf = NULL;
  FILE* file = NULL;
  long file_size = 0;
  size_t read = 0;
  
  if (filename == NULL) {
    fputs("cvmc: null filename\n", stdout);
    return nil;
  }

  file = AtExitFopen(atexit, filename, "rb");
  if (!file) {
    printf("cvmc: can't open %s\n", filename);
    return nil;
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (file_size <= 0) {
    printf("cvmc: invalid file size: %ld\n", file_size);
    return nil;
  }

  buf = (char*)malloc((size_t)file_size+1);
  if (!buf) {
    fputs("cvmc: out of memory\n", stdout);
    return nil;
  }

  read = fread(buf, 1, (size_t)file_size, file);
  if (read != (size_t)file_size) {
    printf("cvmc: readed %lu expected %ld\n", read, file_size);
    free(buf);
    return nil;
  }

  buf[file_size] = '\0';
  *out_len = file_size;

  return buf;
}

/* Internal bug fix */
typedef char* charptr;

#define GEN_STATIC
#define GEN_NAME RealOpenFileAndInitBuf
#define GEN_OUT  OpenFileAndInitBuf
#define GEN_TYPE charptr
#define GEN_ARGS GENARG(const char*, filename) GENARG(long*, out_len)
#include <gen_wrapper.h>

static void FreeAtExit(void* ptr) {
  if (ptr) {
    AtExitFree((AtExitHeader*)ptr);
  }
}

static Opcode* InitOpcodes(AtExitHeader* main_atexit) {
  Opcode* ret = (Opcode*)AtExitMalloc(main_atexit, sizeof(Opcode) * 512);
  if (ret) {
    memset(ret, 0, sizeof(Opcode) * 512);
  }

  return ret;
}

static int RealTryWriteToFile(AtExitHeader* atexit, Opcode* opcodes) {
  size_t wrote = 0;
  const char magic[5] = {'?', 'C', 'V', 'M', 'C'};
  FILE* out = NULL;

  out = AtExitFopen(atexit, "out.cvm", "wb");
  if (!out) {
    fputs("cvmc: can't open out.cvm\n", stdout);
    return 1;
  }

  wrote = fwrite(magic, 1, 5, out);
  if (wrote != 5) {
    fputs("cvmc: can't write magic to file\n", stdout);
    return 1;
  }

  wrote = fwrite(opcodes, sizeof(Opcode), 512, out);
  if (wrote != (sizeof(Opcode) * 512)) {
    fputs("cvmc: file to file error\n", stdout);
    return 1;
  }

  return 0;
}

#define GEN_NAME RealTryWriteToFile
#define GEN_OUT  TryWriteToFile
#define GEN_TYPE int
#define GEN_ARGS GENARG(Opcode*, opcodes)
#include <gen_wrapper.h>

int Main(AtExitHeader* main_atexit, int argc, char** argv) {
  char* buffer = NULL;
  long out_len = 0;
  TCVMOCtx* ctx = NULL;
  Opcode* opcodes = NULL;
  AtExitHeader* temp_atexit = NULL;
  
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

  buffer = OpenFileAndInitBuf(temp_atexit, argv[1], &out_len);
  if (!buffer) {
    return 1;
  }

  defer (main_atexit, free, buffer);

  opcodes = InitOpcodes(main_atexit);
  if (!opcodes) {
    fputs("cvmc: out of memory\n", stdout);
    return 1;
  }

  ctx = TCVMOCtxInit((i8*)buffer, (size_t)out_len, opcodes, 512);
  if (!ctx) {
    fputs("cvmc: out of memory\n", stdout);
    return 1;
  }

  ToCVMOpcodes(ctx);

  TCVMOCtxFree(ctx);

  if (TryWriteToFile(temp_atexit, opcodes)) {
    return 1;
  }
  
  return 0;
}
