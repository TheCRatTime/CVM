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
    cvmr - run CVM Opcodes.\n\
\n\
SYNOPSIS\n\
    cvmr FILE\n\
\n\
DESCRIPTION\n\
    cvmr is a CVM Runner. Util runs CVM Opcodes.\n", stdout);
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
    printf("cvmr: can't open %s\n", filename);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  file_len = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (file_len != (5 + (sizeof(Opcode) * 512)) ||
      file_len <= 0) {
    fputs("cvmr: invalid file size\n", stdout);
    return NULL;
  }

  buf = (char*)AtExitMalloc(atexit, (size_t)file_len+1);
  if (!buf) {
    fputs("cvmr: out of memory\n", stdout);
    return NULL;
  }

  buf[file_len] = '\0';

  read = fread(buf, 1, (size_t)file_len, file);
  if (read != (size_t)file_len) {
    fputs("cvmr: cannot read from file\n", stdout);
    return NULL;
  }

  if (strncmp(buf, "?CVMC", 5)) {
    fputs("cvmr: magic failure\n", stdout);
    return NULL;
  }

  ret = (Opcode*)malloc(sizeof(Opcode) * 512);
  if (!ret) {
    fputs("cvmr: out of memory\n", stdout);
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

int Main(AtExitHeader* main_atexit, int argc, char** argv) {
  AtExitHeader* temp_atexit = NULL;
  CVM* vm = NULL;
  Opcode* opcodes = NULL;
  
  if (argc < 2) {
    Help();
    return 1;
  }

  temp_atexit = InitAtExit(ATEXIT_DEFVAL);
  if (!temp_atexit) {
    fputs("cvmr: out of memory\n", stdout);
    return 1;
  }

  defer (main_atexit, FreeAtExit, temp_atexit);

  opcodes = GetOpcodes(temp_atexit, argv[1]);
  if (!opcodes) {
    return 1;
  }

  defer (main_atexit, free, opcodes);

  vm = InitCVM();
  if (!vm) {
    fputs("cvmr: out of memory\n", stdout);
    return 1;
  }

  CVMRun(vm, (u8*)opcodes, 512);

  FreeCVM(vm);

  return 0;
}
