#ifndef _BM_H_
#define _BM_H_

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>
#include<inttypes.h>

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_NATIVE_CAPACITY 1024
#define LABEL_CAPACITY 1024
#define DEFERED_OPERANDS_CAPACITY 1024

#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = (value)}
#define MAKE_INST_PLUS {.type = INST_PLUSI}
#define MAKE_INST_MINUS {.type = INST_MINUSI}
#define MAKE_INST_MULT {.type = INST_MULTI}
#define MAKE_INST_DIV {.type = INST_DIVI}
#define MAKE_INST_JMP(addr) {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_DUP(addr) {.type = INST_DUP, .operand = (addr)}
#define MAKE_INST_HALT {.type = INST_JMP, .operand = (addr)}

#define BASM_COMMENT_SYMBOL ';'
#define BASM_PP_SYMBOL '%'
#define BASM_MAX_INCLUDDE_LEVEL 69

typedef enum {
  TRAP_OK = 0,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
  TRAP_ILLEGAL_INST,
  TRAP_ILLEGAL_INST_ACCESS,
  TRAP_ILLEGAL_OPERAND,
  TRAP_DIV_BY_ZERO,
} Err;

typedef struct 
{
  size_t count;
  const char *data;
} String_View;

#define SV_FORMAT(sv) (int) sv.count, sv.data

// TODO: comparsion instruction is not set
// TODOl there is no operation for converting integer->float/float->integer

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_DUP,
    INST_DROP,
    INST_SWAP,
    INST_PLUSI,
    INST_MINUSI,
    INST_MULTI,
    INST_DIVI,
    INST_PLUSF,
    INST_MINUSF,
    INST_MULTF,
    INST_DIVF,
    INST_JMP,
    INST_JMP_IF,
    INST_EQ,
    INST_HALT,
    INST_NOT,
    INST_GEF,
    INST_RET,
    INST_CALL,
    INST_NATIVE,
    INST_URMOM,
    NUMBER_OF_INSTS,
} Inst_Type;

const char *inst_name(Inst_Type type);
int inst_has_operand(Inst_Type type);
int inst_by_name(String_View name, Inst_Type *output);

typedef uint64_t Inst_Addr;

typedef union {
  uint64_t as_u64;
  int64_t as_i64;
  double as_f64;
  void *as_ptr;
} Word;

static_assert(sizeof(Word) == 8, "The BM's Word is expected to be 64 bits");

typedef struct {
    Inst_Type type;
    Word operand;
} Inst;

typedef struct Bm Bm;

typedef Err (*Bm_Native)(Bm*);

struct Bm {
   Word stack[BM_STACK_CAPACITY];
   Inst_Addr stack_size;

   Inst program[BM_STACK_CAPACITY];
   Inst_Addr program_size;
   Inst_Addr ip;

   Bm_Native natives[BM_NATIVE_CAPACITY];
   size_t natives_size;

   int halt;
};

Bm bm = {0};

int sv_eq(String_View a, String_View b);
int sv_to_int(String_View sv);

const char *trap_as_cstr(Err err);
const char *inst_type_as_cstr(Inst_Type type);

Err bm_execute_inst(Bm *bm);
Err bm_execute_program(Bm *bm, int limit);
void bm_push_native(Bm *bm, Bm_Native native);
void bm_dump_stack(FILE *stream, const Bm *bm);
void bm_load_program_from_memory(Bm *bm, Inst *program, size_t program_size);
void bm_load_program_from_file(Bm *bm, const char *file_path);
void bm_save_program_to_file(const Bm *bm, const char *file_path);

String_View cstr_as_sv(const char *cstr);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_chop_by_delim(String_View *sv, char delim);
String_View sv_slurp_file(String_View file_path);

typedef struct {
  String_View name;
  Word word;
} Label;

typedef struct {
  Inst_Addr addr; 
  String_View label;
} Defered_Operand;

typedef struct {
  Label labels[LABEL_CAPACITY];
  size_t labels_size;
  Defered_Operand defered_operands[DEFERED_OPERANDS_CAPACITY];
  size_t defered_operands_size;
} Basm;

int basm_resolve_label(const Basm *basm, String_View name, Word *output);
int basm_bind_label(Basm *basm, String_View name, Word word);
void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View label);

void bm_translate_source( Bm *bm, Basm *basm, String_View input_file_path, size_t level);

int number_literal_as_word(String_View sv, Word *output);

#endif // __BM_H_

#ifdef BM_IMPLEMENTATION

int inst_has_operand(Inst_Type type)
{
  switch (type) {
       case INST_NOP: return 0;
       case INST_PUSH: return 1;
       case INST_DUP: return 1;
       case INST_DROP: return 0;
       case INST_PLUSI: return  0;
       case INST_MINUSI: return 0;
       case INST_MULTI: return 0;
       case INST_DIVI: return  0;
       case INST_PLUSF: return 0;
       case INST_MINUSF: return 0;
       case INST_MULTF: return 0;
       case INST_DIVF: return 0;
       case INST_JMP: return 1;
       case INST_JMP_IF: return 1;
       case INST_EQ: return 0;
       case INST_HALT: return 0;
       case INST_SWAP:   return 1;
       case INST_NOT:  return 0;
       case INST_GEF:  return 0;
       case INST_RET: return 0;
       case INST_CALL: return 1;
       case INST_NATIVE: return 1;
       case INST_URMOM: return 1;
       case NUMBER_OF_INSTS: 
       default: assert(0 && "inst_has_operand: unreachable");
  }
}

int inst_by_name(String_View name, Inst_Type *output)
{
  for (Inst_Type type = (Inst_Type) 0; type < NUMBER_OF_INSTS; type += 1) {
    if (sv_eq(cstr_as_sv(inst_name(type)), name)) {
       *output = type;
       return 1;
    }
  }
  
  return 0;
}

const char *inst_name(Inst_Type type)
{
  switch (type) {
       case INST_NOP: return "nop";
       case INST_PUSH: return "push";
       case INST_DUP: return "dup";
       case INST_PLUSI: return  "plusi";
       case INST_MINUSI: return "minusi";
       case INST_MULTI: return "multi";
       case INST_DIVI: return  "divi";
       case INST_PLUSF: return "plusf";
       case INST_MINUSF: return "minusf";
       case INST_MULTF: return "multf";
       case INST_DIVF: return "divf";
       case INST_JMP:  return "jmp";
       case INST_JMP_IF: return "jmp_if";
       case INST_EQ: return "eq";
       case INST_HALT: return "halt";
       case INST_SWAP:  return "swap";
       case INST_NOT: return "not";
       case INST_GEF: return "gef";
       case INST_DROP: return "drop";
       case INST_RET: return "ret";
       case INST_CALL: return "call";
       case INST_NATIVE: return "native";
       case INST_URMOM: return "urmom";
       case NUMBER_OF_INSTS: 
       default: assert(0 && "inst_name: unreachable");
  }
}

const char *trap_as_cstr(Err err)
{
  switch (err) {
      case TRAP_OK:
          return "TRAP_OK";
      case TRAP_STACK_OVERFLOW:
          return "TRAP_STACK_OVERFLOW";
      case TRAP_STACK_UNDERFLOW:
          return "TRAP_STACK_UNDERFLOW";
      case TRAP_ILLEGAL_INST:
          return "TRAP_ILLEGAL_INST";
      case TRAP_ILLEGAL_INST_ACCESS:
          return "TRAP_ILLEGAL_INST_ACCESS";
      case TRAP_ILLEGAL_OPERAND:
          return "TRAP_ILLEGAL_OPERAND";
      case TRAP_DIV_BY_ZERO:
          return "TRAP_DIV_BY_ZERO";
      default:
          assert(0 && "trap_as_cstr: Unreachable");
  }
}

const char *inst_type_as_cstr(Inst_Type type)
{
   switch(type) {
     case  INST_NOP:  return "INST_NOP";
     case  INST_PUSH: return "INST_PUSH";
     case  INST_PLUSI: return "INST_PLUSI";
     case INST_MINUSI: return "INST_MINUSI";
     case  INST_MULTI: return "INST_MULT";
     case   INST_DIVI: return "INST_DIVI";
     case  INST_PLUSF: return "INST_PLUSF";
     case INST_MINUSF: return "INST_MINUSF";
     case  INST_MULTF: return "INST_MULTF";
     case   INST_DIVF: return "INST_DIVF";
     case   INST_JMP: return "INST_JMP";
     case  INST_HALT: return "INST_HALT";
     case  INST_JMP_IF: return "INST_JMP_IF";
     case   INST_EQ: return "INST_EQ";
     case INST_DUP: return "INST_DUP";
     case INST_SWAP: return "INST_SWAP";
     case INST_NOT: return "INST_NOT";
     case INST_GEF: return "INST_GEF";
     case INST_DROP: return "INST_DROP";
     case INST_RET: return "INST_RET";
     case INST_CALL: return "INST_CALL";
     case INST_NATIVE: return "INST_NATIVE";
     case INST_URMOM: return "INST_URMOM";
     case NUMBER_OF_INSTS: 
     default: assert(0 && "trap_as_cstr: Unreachable");
   }
}

Err bm_execute_program(Bm *bm, int limit)
{
  while (limit != 0 && !bm->halt) {
    Err err = bm_execute_inst(bm);
    if (err != TRAP_OK) {
      return err;
    }
    if (limit > 0) {
      --limit;
    }
  }
  return TRAP_OK;
}

Err bm_execute_inst(Bm *bm) 
{
  if (bm->ip > bm->program_size) {
     return TRAP_ILLEGAL_INST_ACCESS;
  }

  Inst inst = bm->program[bm->ip];

  switch (inst.type) {
    case INST_NOP:
        bm->ip += 1;
        break;
    case INST_PUSH:
       if (bm->stack_size >= BM_STACK_CAPACITY) {
          return TRAP_STACK_OVERFLOW;
       }

       bm->stack[bm->stack_size++] = inst.operand;
       bm->ip += 1;
       break; 

    case INST_DROP:
       if (bm->stack_size >= BM_STACK_CAPACITY) {
          return TRAP_STACK_OVERFLOW;
       }

       bm->stack_size -= 1;
       bm->ip += 1;
       break; 
    
    case INST_PLUSI:
       if (bm->stack_size < 2) {
         return TRAP_STACK_UNDERFLOW;
       }

       bm->stack[bm->stack_size - 2].as_u64 +=  bm->stack[bm->stack_size - 1].as_u64;
       bm->stack_size -= 1;
       bm->ip += 1;
       break;

    case INST_MINUSI:
      if (bm->stack_size < 2) {
         return TRAP_STACK_UNDERFLOW;
       }

       bm->stack[bm->stack_size - 2].as_u64 -=  bm->stack[bm->stack_size - 1].as_u64;
       bm->stack_size -= 1;
       bm->ip += 1;
       break;

    case INST_MULTI:
       if (bm->stack_size < 2) {
          return TRAP_STACK_UNDERFLOW;
        }
        bm->stack[bm->stack_size - 2].as_u64 *=  bm->stack[bm->stack_size - 1].as_u64;
        bm->stack_size -= 1;
        bm->ip += 1;
        break;

    case INST_DIVI:
       if (bm->stack_size < 2) {
         return TRAP_STACK_UNDERFLOW;
       }

       if (bm->stack[bm->stack_size - 1].as_u64 == 0) {
         return TRAP_DIV_BY_ZERO;
       }

       bm->stack[bm->stack_size - 2].as_u64 /=  bm->stack[bm->stack_size - 1].as_u64;
       bm->stack_size -= 1;
       bm->ip += 1;
       break;

    case INST_PLUSF:
      if (bm->stack_size < 2) {
        return TRAP_STACK_UNDERFLOW;
      }
      
      bm->stack[bm->stack_size - 2].as_f64  +=  bm->stack[bm->stack_size - 1].as_f64;
      bm->stack_size -= 1;
      bm->ip += 1;
      break;

    case INST_MINUSF:
    if (bm->stack_size < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    bm->stack[bm->stack_size - 2].as_f64  -=  bm->stack[bm->stack_size - 1].as_f64;
    bm->stack_size -= 1;
    bm->ip += 1;
    break;

    case INST_MULTF:
    if (bm->stack_size < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    bm->stack[bm->stack_size - 2].as_f64  *=  bm->stack[bm->stack_size - 1].as_f64;
    bm->stack_size -= 1;
    bm->ip += 1;
    break;

    case INST_DIVF:
    if (bm->stack_size < 2) {
      return TRAP_STACK_UNDERFLOW;
    }

    bm->stack[bm->stack_size - 2].as_f64  /=  bm->stack[bm->stack_size - 1].as_f64;
    bm->stack_size -= 1;
    bm->ip += 1;
    break;

    case INST_JMP:
      bm->ip = inst.operand.as_u64;
      break;

    case INST_RET:
      if (bm->stack_size < 1) {
        return TRAP_STACK_UNDERFLOW;
      }

      bm->ip = bm->stack[bm->stack_size - 1].as_u64;
      bm->stack_size -= 1;
      break;

    case INST_CALL:
      if (bm->stack_size >= BM_STACK_CAPACITY) {
        return TRAP_STACK_OVERFLOW;
      }
      bm->stack[bm->stack_size++].as_u64 = bm->ip + 1;
      bm->ip = inst.operand.as_u64;
      break;

    case INST_NATIVE:
      if (inst.operand.as_u64 > bm->natives_size) {
        return TRAP_ILLEGAL_OPERAND;
      }
      bm->natives[inst.operand.as_u64](bm);
      bm->ip += 1;
      break;
    
    case INST_HALT:
      bm->halt = 1;
      break;

    case INST_JMP_IF:
      if (bm->stack_size < 1) {
         return TRAP_STACK_UNDERFLOW;
      }

      if (bm->stack[bm->stack_size - 1].as_u64) {
          bm->ip = inst.operand.as_u64;
      } else {
          bm->ip += 1;
      }

      bm->stack_size -= 1;
      break;

    case INST_EQ:
      if (bm->stack_size < 2) {
         return TRAP_STACK_UNDERFLOW;
       }

      bm->stack[bm->stack_size - 2].as_u64 =  bm->stack[bm->stack_size - 1].as_u64 ==  bm->stack[bm->stack_size - 2].as_u64;
       bm->stack_size -= 1;
       bm->ip += 1;
      break;
      
    case INST_GEF:
      if (bm->stack_size < 2) {
         return TRAP_STACK_UNDERFLOW;
       }
  
      bm->stack[bm->stack_size - 2].as_f64 =  bm->stack[bm->stack_size - 1].as_f64 >=  bm->stack[bm->stack_size - 2].as_f64;
       bm->stack_size -= 1;
       bm->ip += 1;
      break;

    case INST_DUP:
      if (bm->stack_size >= BM_STACK_CAPACITY) {
          return TRAP_STACK_OVERFLOW;
       }

      if (bm->stack_size - inst.operand.as_u64 <= 0) {
        return TRAP_STACK_UNDERFLOW;
      }

      bm->stack[bm->stack_size] = bm->stack[bm->stack_size - 1 - inst.operand.as_u64];

      bm->stack_size += 1;
      bm->ip += 1;
      break;
    
    case INST_SWAP:
        if (inst.operand.as_u64 >= bm->stack_size) {
          return TRAP_STACK_UNDERFLOW;
        }

        const uint64_t a = bm->stack_size - 1;
        const uint64_t b = bm->stack_size - 1 - inst.operand.as_u64; 
      
        Word t = bm->stack[a];
        bm->stack[a] =  bm->stack[b];
        bm->stack[b] = t;
        bm->ip += 1;
        break;

    // TODO: incosistency between gef and minus* instructions operand orders
    case INST_NOT:
      if (bm->stack_size <= 1) {
        return TRAP_STACK_UNDERFLOW;
      }

      bm->stack[bm->stack_size -1].as_u64 =  !bm->stack[bm->stack_size -1].as_u64;
      bm->ip += 1;
      break;

    case INST_URMOM:
      break;
    
    case NUMBER_OF_INSTS:    
    default:
      return TRAP_ILLEGAL_INST;    
  }
  return TRAP_OK;
}

void bm_push_native(Bm *bm, Bm_Native native)
{
   assert(bm->natives_size < BM_NATIVE_CAPACITY);
   bm->natives[bm->natives_size++] = native;
}

void bm_dump_stack(FILE *stream, const Bm *bm)
{
   fprintf(stream, "Stack:\n");
   if (bm->stack_size > 0) {
      for (Inst_Addr i = 0; i < bm->stack_size; ++i) {
          fprintf(stream, " u64: %lu i64: %ld f64: %lf ptr: %p\n",     
                  bm->stack[i].as_u64,
                  bm->stack[i].as_i64,
                  bm->stack[i].as_f64,
                  bm->stack[i].as_ptr
            );
       }
   } else {
     fprintf(stream, "  [empty]\n");
   }
}

void bm_load_program_from_memory(Bm *bm, Inst *program, size_t program_size)
{
  assert(program_size < BM_PROGRAM_CAPACITY);
  memcpy(bm->program, program, sizeof(program[0]) * program_size);
  bm->program_size = program_size;
}

void bm_load_program_from_file(Bm *bm, const char *file_path)
{
   FILE *f = fopen(file_path, "rb");
   if (f == NULL) {
     fprintf(stderr, "ERROR: could not open file %s : %s\n", file_path, strerror(errno));
     exit(1);
   }
  if (fseek(f, 0, SEEK_END) < 0) {
     fprintf(stderr, "ERROR: could not read file %s : %s\n", file_path, strerror(errno));
     exit(1);
  }

  long m = ftell(f);
  if (m < 0) {
     fprintf(stderr, "ERROR: could not read file %s : %s\n", file_path, strerror(errno));
     exit(1);
  }

  assert(m % sizeof(bm->program[0]) == 0);
  assert((size_t) m <= BM_PROGRAM_CAPACITY * sizeof(bm->program[0]));

  if (fseek(f, 0, SEEK_SET) < 0) {
     fprintf(stderr, "ERROR: could not read file %s : %s\n", file_path, strerror(errno));
     exit(1);
  }

  bm->program_size = fread(bm->program, sizeof(bm->program[0]), m / sizeof(bm->program[0]), f);
  if (ferror(f)) {
    fprintf(stderr, "ERROR: could not read file %s : %s\n", file_path, strerror(errno));
     exit(1);
  }

  fclose(f);
}

void bm_save_program_to_file(const Bm *bm, const char *file_path)
{
  FILE *f = fopen(file_path, "wb");
  if (f == NULL) {
    fprintf(stderr, "ERROR: could not open file %s : %s\n", file_path, strerror(errno));
    exit(1);
  }

  fwrite(bm->program, sizeof(bm->program[0]), bm->program_size, f);

  if (ferror(f)) {
    fprintf(stderr, "ERROR: could not write to file %s : %s\n", file_path, strerror(errno));
    exit(1);
  }

  fclose(f);
}

String_View cstr_as_sv(const char *cstr)
{
  return (String_View) {
    .count = strlen(cstr),
    .data = cstr,
  };
}

String_View sv_trim_left(String_View sv)
{
  size_t i = 0;
  while (i < sv.count && isspace(sv.data[i])) {
    i += 1;
  }

  return (String_View) {
    .count = sv.count - i,
    .data = sv.data + i,
  };
}

String_View sv_trim_right(String_View sv)
{
  size_t i = 0;
  while (i < sv.count && isspace(sv.data[sv.count - 1 - i])) {
     i += 1;
  }
 return (String_View) {
   .count = sv.count - i,
   .data = sv.data,
 };
}

String_View sv_trim(String_View sv)
{
    return sv_trim_right(sv_trim_left(sv));
}

String_View sv_chop_by_delim(String_View *sv, char delim)
{
  size_t i = 0;
  while (i < sv->count && sv->data[i] != delim) {
       i += 1;
  }

  String_View result = {
     .count = i,
     .data = sv->data,
  };

  if (i < sv->count) {
    sv->count -= i + 1;
    sv->data += i + 1;
  } else {
    sv->count -= i;
    sv->data += i;
  }

  return result;
}

int sv_eq(String_View a, String_View b)
{
  if (a.count != b.count) {
    return 0;
  } else {
    return memcmp(a.data, b.data, a.count) == 0;
  }
}

int sv_to_int(String_View sv)
{
   int result = 0;

   for (size_t i = 0; i < sv.count && isdigit(sv.data[i]); ++i) {
     result = result * 10 + sv.data[i] - '0';
   }

  return result;
}

int basm_resolve_label(const Basm *basm, String_View name, Word *output)
{
    for (size_t i = 0; i < basm->labels_size; ++i) {
      if (sv_eq(basm->labels[i].name, name)) {
         *output = basm->labels[i].word;
         return 1;
      }
    }
    return 0;
}

int basm_bind_label(Basm *basm, String_View name, Word word)
{
  assert(basm->labels_size < LABEL_CAPACITY);
 
  Word ignore = {0};
  if (basm_resolve_label(basm, name, &ignore)) {
    return 0;
  }
    
  basm->labels[basm->labels_size++] = (Label) {.name = name, .word = word};
  return 1;
}

void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View label)
{
  assert(basm->defered_operands_size < DEFERED_OPERANDS_CAPACITY);
  basm->defered_operands[basm->defered_operands_size++] =    
      (Defered_Operand) {.addr = addr, .label = label};
}

int number_literal_as_word(String_View sv, Word *output)
{
  assert(sv.count < 1024);
  char cstr[sv.count + 1];
  char *endptr = 0;

  memcpy(cstr, sv.data, sv.count);
  cstr[sv.count] = '\0';

  Word result = {0};

  result.as_u64 = strtoull(cstr, &endptr, 10);
  if ((size_t) (endptr - cstr) != sv.count) {
     result.as_f64 = strtod(cstr, &endptr);
     if ((size_t) (endptr - cstr) != sv.count) {
       return 0;
     } 
  } 

  *output = result;
  return 1;
}

void bm_translate_source( Bm *bm, Basm *basm, String_View input_file_path, size_t level)
{
   String_View original_source = sv_slurp_file(input_file_path);
   String_View source = original_source;
  
   bm->program_size = 0;
   int line_number = 0;

   // First pass
   while (source.count > 0) {
     assert(bm->program_size < BM_PROGRAM_CAPACITY);
     String_View line = sv_trim(sv_chop_by_delim(&source, '\n'));
     line_number += 1;
     if (line.count > 0 && *line.data != BASM_COMMENT_SYMBOL) {
        String_View token = sv_trim(sv_chop_by_delim(&line, ' '));

        // Pre-processor
        if (token.count > 0 && *token.data == BASM_PP_SYMBOL) {
           token.count -= 1;
           token.data += 1;
           if (sv_eq(token, cstr_as_sv("label"))) {
              line = sv_trim(line);
              String_View label = sv_chop_by_delim(&line, ' ');
              if (label.count > 0) {
                line = sv_trim(line);
                String_View value = sv_chop_by_delim(&line, ' ');
                Word word = {0};
                if (!number_literal_as_word(value, &word)) {
                  fprintf(stderr, "%.*s:%d: ERROR: `%.*s` is not a number\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(value));
                   exit(1);
                }
                
                if (!basm_bind_label(basm, label, word)) {
                       fprintf(stderr, "%.*s:%d: ERROR: label `%.*s` is already defined\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(label));
                        exit(1);
                }
              } else {
                      fprintf(stderr, "%.*s:%d: ERROR: label name is not provided\n", SV_FORMAT(input_file_path), line_number);
                       exit(1);
               }
             } else if(sv_eq(token, cstr_as_sv("include"))) {
                line = sv_trim(line);
                if (line.count > 0) {
                  if (*line.data == '"' && line.data[line.count - 1] == '"') {
                    line.data += 1;
                    line.count -= 2;

                    if (level + 1 >= BASM_MAX_INCLUDDE_LEVEL) {
                      fprintf(stderr, "%.*s:%d: ERROR: exceeded maximum include level\n", SV_FORMAT(input_file_path), line_number);
                       exit(1);
                    }
                    
                    bm_translate_source(bm, basm, line, level + 1);
                  } else {
                    fprintf(stderr, "%.*s:%d: ERROR: include file path has to be surrounded with quotation marks\n", SV_FORMAT(input_file_path), line_number);
                     exit(1);
                  }
                } 
                else {
                  fprintf(stderr, "%.*s:%d: ERROR: include file path is not provided\n", SV_FORMAT(input_file_path), line_number);
                   exit(1);
                }             
           } else {
             fprintf(stderr, "%.*s:%d: ERROR: unknown pre-processor directive `%.*s`\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(token));
             exit(1);
           }
        } else {
          // label
            if (token.count > 0 && token.data[token.count - 1] == ':') {
              String_View label = {
                 .count = token.count - 1,
                 .data = token.data,
              };

              if (!basm_bind_label(basm, label, (Word){.as_u64 = bm->program_size})) {
                // TODO: label redefinition does not tell where the label was already defined
                     fprintf(stderr, "%.*s:%d: ERROR: label `%.*s` is already defined\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(label));
                      exit(1);
              }

              token = sv_trim(sv_chop_by_delim(&line, ' '));
            } 

           // Instruction
           if (token.count > 0) {
               String_View operand = sv_trim(sv_chop_by_delim(&line, BASM_COMMENT_SYMBOL));

               Inst_Type inst_type = INST_NOP;
               if (inst_by_name(token, &inst_type)) {
                 bm->program[bm->program_size].type = inst_type;

                 if (inst_has_operand(inst_type)) {
                   if (operand.count == 0) {
                     fprintf(stderr, "%.*s:%d: ERROR: instruction `%.*s` requires an operand\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(token));
                     exit(1);
                   }
                   if (!number_literal_as_word(operand, &bm->program[bm->program_size].operand)) {                
                    basm_push_defered_operand(
                     basm, bm->program_size, operand);
                   }
                 }
                 bm->program_size += 1;
               } else {
                  fprintf(stderr, "%.*s:%d: ERROR: unknown instruction `%.*s`\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(token));
                  exit(1);
              }
           }
        }
     }
   }
    // Second pass
    for (size_t i = 0; i < basm->defered_operands_size; ++i) {
        String_View label =  basm->defered_operands[i].label;

        if (!basm_resolve_label(
          basm,
          label,
          &bm->program[basm->defered_operands[i].addr].operand)) {
                  // TODO: second pass resolutiion error
                  fprintf(stderr, "%.*s: ERROR: unknown label `%.*s`\n", SV_FORMAT(input_file_path), SV_FORMAT(label));
                  exit(1);
        }
    }

  free((void*) original_source.data);
}

String_View sv_slurp_file(String_View file_path)
{
  char *file_path_cstr = malloc(file_path.count + 1);

  if (file_path_cstr == NULL) {
    fprintf(stderr, "ERROR: could not allocate memory for file path: `%.*s` : %s\n", SV_FORMAT(file_path), strerror(errno));
     exit(1);
  }

  memcpy(file_path_cstr, file_path.data, file_path.count);
  file_path_cstr[file_path.count] = '\0';
  
  FILE *f = fopen(file_path_cstr, "r");
  if (f == NULL) {
     fprintf(stderr, "ERROR: could not read file `%s` : %s\n", file_path_cstr, strerror(errno));
     exit(1);
  }

  if (fseek(f, 0, SEEK_END) < 0) {
     fprintf(stderr, "ERROR: could not read file `%s` : %s\n", file_path_cstr, strerror(errno));
     exit(1);
  }

  long m = ftell(f);
  if (m < 0) {
     fprintf(stderr, "ERROR: could not read file `%s` : %s\n", file_path_cstr, strerror(errno));
     exit(1);
  }

  char *buffer = malloc(m);
  if (buffer == NULL) {
    fprintf(stderr, "ERROR: could not allocate memory for file: %s\n", strerror(errno));
     exit(1);
  }

  if (fseek(f, 0, SEEK_SET) < 0) {
     fprintf(stderr, "ERROR: could not read file `%s` : %s\n", file_path_cstr, strerror(errno));
     exit(1);
  }

  size_t n = fread(buffer, 1, m, f);
  if (ferror(f)) {
     fprintf(stderr, "ERROR: could not read file `%s` : %s\n", file_path_cstr, strerror(errno));
     exit(1);
  }

  fclose(f);
  free(file_path_cstr);

  return (String_View) {
    .count = n,
    .data = buffer,
  };
}

#endif// BM_IMPLEMENTATION