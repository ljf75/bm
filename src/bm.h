#ifndef _BM_H_
#define _BM_H_

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
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

typedef enum {
  TRAP_OK = 0,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
  TRAP_ILLEGAL_INST,
  TRAP_ILLEGAL_INST_ACCESS,
  TRAP_ILLEGAL_OPERAND,
  TRAP_DIV_BY_ZERO,
} Trap;

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
    INST_PRINT_DEBUG,
    NUMBER_OF_INSTS,
} INST_Type;

const char *inst_name(INST_Type type);
int inst_has_operand(INST_Type type);

typedef uint64_t Inst_Addr;

typedef union {
  uint64_t as_u64;
  int64_t as_i64;
  double as_f64;
  void *as_ptr;
} Word;

static_assert(sizeof(Word) == 8, "The BM's Word is expected to be 64 bits");

typedef struct {
    INST_Type type;
    Word operand;
} Inst;

typedef struct {
   Word stack[BM_STACK_CAPACITY];
   Inst_Addr stack_size;

   Inst program[BM_STACK_CAPACITY];
   Inst_Addr program_size;
   Inst_Addr ip;

   int halt;
} Bm;

Bm bm = {0};

typedef struct 
{
  size_t count;
  const char *data;
} String_View;

int sv_eq(String_View a, String_View b);
int sv_to_int(String_View sv);

const char *trap_as_cstr(Trap trap);
const char *inst_type_as_cstr(INST_Type type);

Trap bm_execute_inst(Bm *bm);
Trap bm_execute_program(Bm *bm, int limit);

void bm_dump_stack(FILE *stream, const Bm *bm);
void bm_load_program_from_memory(Bm *bm, Inst *program, size_t program_size);
void bm_load_program_from_file(Bm *bm, const char *file_path);
void bm_save_program_to_file(const Bm *bm, const char *file_path);

String_View cstr_as_sv(const char *cstr);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_chop_by_delim(String_View *sv, char delim);
String_View sv_slurp_file(const char *file_path);

typedef struct {
  String_View name;
  Inst_Addr addr;
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

Inst_Addr basm_find_label_addr(const Basm *llt, String_View name);
void basm_push_label(Basm *lt, String_View name, Inst_Addr addr);
void basm_push_defered_operand(Basm *lt, Inst_Addr addr, String_View label);

void bm_translate_source(String_View source, Bm *bm, Basm *lt);

Word number_literal_as_word(String_View sv);

#endif // __BM_H_

#ifdef BM_IMPLEMENTATION

int inst_has_operand(INST_Type type)
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
       case INST_PRINT_DEBUG: return 0;
       case INST_SWAP:   return 1;
       case INST_NOT:  return 0;
       case INST_GEF:  return 0;
       case INST_RET: return 0;
       case INST_CALL: return 1;
       case NUMBER_OF_INSTS: 
       default: assert(0 && "inst_has_operand: unreachable");
  }
}

const char *inst_name(INST_Type type)
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
       case INST_PRINT_DEBUG: return "print_debug";
       case INST_SWAP:  return "swap";
       case INST_NOT: return "not";
       case INST_GEF: return "gef";
       case INST_DROP: return "drop";
       case INST_RET: return "ret";
       case INST_CALL: return "call";
       case NUMBER_OF_INSTS: 
       default: assert(0 && "inst_name: unreachable");
  }
}

const char *trap_as_cstr(Trap trap)
{
  switch (trap) {
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

const char *inst_type_as_cstr(INST_Type type)
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
     case INST_PRINT_DEBUG: return " INST_PRINT_DEBUG";
     case INST_DUP: return "INST_DUP";
     case INST_SWAP: return "INST_SWAP";
     case INST_NOT: return "INST_NOT";
     case INST_GEF: return "INST_GEF";
     case INST_DROP: return "INST_DROP";
     case INST_RET: return "INST_RET";
     case INST_CALL: return "INST_CALL";
     case NUMBER_OF_INSTS: 
     default: assert(0 && "trap_as_cstr: Unreachable");
   }
}

Trap bm_execute_program(Bm *bm, int limit)
{
  while (limit != 0 && !bm->halt) {
    Trap trap = bm_execute_inst(bm);
    if (trap != TRAP_OK) {
      return trap;
    }
    if (limit > 0) {
      --limit;
    }
  }
  return TRAP_OK;
}

Trap bm_execute_inst(Bm *bm) 
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
        return TRAP_STACK_UNDERFLOW;
      }
      bm->stack[bm->stack_size + 1].as_u64 = bm->ip + 1;
      bm->ip = inst.operand.as_u64;
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
    
    case INST_PRINT_DEBUG:
      if (bm->stack_size < 2) {
         return TRAP_STACK_UNDERFLOW;
       }
      fprintf(stdout, " u64: %lu i64: %ld f64: %lf ptr: %p\n",     
            bm->stack[bm->stack_size - 1].as_u64,
            bm->stack[bm->stack_size - 1].as_i64,
            bm->stack[bm->stack_size - 1].as_f64,
            bm->stack[bm->stack_size - 1].as_ptr
      );
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

    case INST_NOT:
      if (bm->stack_size <= 1) {
        return TRAP_STACK_UNDERFLOW;
      }

      bm->stack[bm->stack_size -1].as_u64 =  !bm->stack[bm->stack_size -1].as_u64;
      bm->ip += 1;
      break;
    
    case NUMBER_OF_INSTS:
    
    default:
      return TRAP_ILLEGAL_INST;    
  }
  return TRAP_OK;
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

Inst_Addr basm_find_label_addr(const Basm *lt, String_View name)
{
    for (size_t i = 0; i < lt->labels_size; ++i) {
      if (sv_eq(lt->labels[i].name, name)) {
         return lt->labels[i].addr;
      }
    }
    fprintf(stderr, "ERROR: label `%.*s` does not exist\n", (int) name.count, name.data);
    exit(1);
}

void basm_push_label(Basm *lt, String_View name, Inst_Addr addr)
{
  assert(lt->labels_size < LABEL_CAPACITY);
  lt->labels[lt->labels_size++] = (Label) {.name = name, .addr = addr};
}

void basm_push_defered_operand(Basm *lt, Inst_Addr addr, String_View label)
{
  assert(lt->defered_operands_size < DEFERED_OPERANDS_CAPACITY);
  lt->defered_operands[lt->defered_operands_size++] =    
      (Defered_Operand) {.addr = addr, .label = label};
}

Word number_literal_as_word(String_View sv)
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
       fprintf(stderr, "ERROR: `%s` is not a member literal\n", cstr);
       exit(1);
     } 
  } 
  
  return result;
}

void bm_translate_source(String_View source, Bm *bm, Basm *lt)
{
   bm->program_size = 0;

   // First pass
   while (source.count > 0) {
     assert(bm->program_size < BM_PROGRAM_CAPACITY);
     String_View line = sv_trim(sv_chop_by_delim(&source, '\n'));
     if (line.count > 0 && *line.data != '#') {
        String_View token = sv_chop_by_delim(&line, ' ');
        
        if (token.count > 0 && token.data[token.count - 1] == ':') {
          String_View label = {
             .count = token.count - 1,
             .data = token.data,
          };

          basm_push_label(lt, label, bm->program_size);
          
          token = sv_trim(sv_chop_by_delim(&line, ' '));
        } 

       if (token.count > 0) {
          String_View operand = sv_trim(sv_chop_by_delim(&line, '#'));

           if (sv_eq(token, cstr_as_sv(inst_name(INST_NOP)))) {
               bm->program[bm->program_size++] = (Inst) {
                 .type = INST_NOP,
               };
            } else if (sv_eq(token, cstr_as_sv(inst_name(INST_PUSH)))) {
               bm->program[bm->program_size++] = (Inst) {
                 .type = INST_PUSH, 
                 .operand = number_literal_as_word(operand),
               };
            } else if (sv_eq(token, cstr_as_sv(inst_name(INST_DUP)))) {
              bm->program[bm->program_size++] = (Inst) { 
                .type = INST_DUP, 
                .operand = {.as_i64 = sv_to_int(operand)}
              };
            } else if (sv_eq(token, cstr_as_sv(inst_name(INST_PLUSI)))) {
              bm->program[bm->program_size++] = (Inst) { 
                .type = INST_PLUSI
              };
            }  else if (sv_eq(token, cstr_as_sv(inst_name(INST_MINUSI)))) {
              bm->program[bm->program_size++] = (Inst) { 
                .type = INST_MINUSI
              };
            } else if (sv_eq(token, cstr_as_sv(inst_name(INST_JMP)))) {
              if (operand.count > 0 && isdigit(*operand.data)) {
                bm->program[bm->program_size++] =(Inst) {  
                  .type = INST_JMP,
                  .operand = {.as_i64 = sv_to_int(operand)}
                };
              } else {
                basm_push_defered_operand(
                  lt, bm->program_size, operand);

                bm->program[bm->program_size++] =(Inst) {  
                  .type = INST_JMP
                };
              }
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_JMP_IF)))) {
               if (operand.count > 0 && isdigit(*operand.data)) {
                 bm->program[bm->program_size++] =(Inst) {  
                   .type = INST_JMP_IF,
                   .operand = {.as_i64 = sv_to_int(operand)}
                 };
               } else {
                 basm_push_defered_operand(
                   lt, bm->program_size, operand);

                 bm->program[bm->program_size++] =(Inst) {  
                   .type = INST_JMP_IF,
                 };
               }
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_CALL)))) {
              if (operand.count > 0 && isdigit(*operand.data)) {
                bm->program[bm->program_size++] =(Inst) {  
                  .type = INST_CALL,
                  .operand = {.as_i64 = sv_to_int(operand)}
                };
              } else {
                basm_push_defered_operand(
                  lt, bm->program_size, operand);

                bm->program[bm->program_size++] =(Inst) {  
                  .type = INST_CALL,
                };
              }
            } else if (sv_eq(token, cstr_as_sv(inst_name(INST_HALT)))) {
             bm->program[bm->program_size++] =(Inst) {  
               .type = INST_HALT
               }; 
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_PLUSF)))) {
             bm->program[bm->program_size++] = (Inst) { 
                    .type = INST_PLUSF
                  };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_MINUSF)))) {
                bm->program[bm->program_size++] = (Inst) { 
                       .type = INST_MINUSF
                  };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_DIVF)))) {
          bm->program[bm->program_size++] = (Inst) { 
                 .type = INST_DIVF
               };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_MULTF)))) {
               bm->program[bm->program_size++] = (Inst) { 
                      .type = INST_MULTF
              };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_SWAP)))) {
              bm->program[bm->program_size++] = (Inst) { 
                     .type = INST_SWAP,
                     .operand = {.as_u64 = sv_to_int(operand)},
              };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_EQ)))) {
             bm->program[bm->program_size++] = (Inst) { 
                    .type = INST_EQ,
               };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_DROP)))) {
                bm->program[bm->program_size++] = (Inst) { 
                       .type = INST_DROP,
               };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_RET)))) {
               bm->program[bm->program_size++] = (Inst) { 
                      .type = INST_RET,
              };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_GEF)))) {
                bm->program[bm->program_size++] = (Inst) { 
                       .type = INST_GEF,
               };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_NOT)))) {
                bm->program[bm->program_size++] = (Inst) { 
                       .type = INST_NOT,
                  };
             } else if (sv_eq(token, cstr_as_sv(inst_name(INST_PRINT_DEBUG)))) {
               bm->program[bm->program_size++] = (Inst) { 
                      .type = INST_PRINT_DEBUG,
                };
             } else {
              fprintf(stderr, "ERROR: unknown instruction `%.*s`", (int) token.count, token.data);
              exit(1);
          }
       }
     }
   }
    // Second pass
    for (size_t i = 0; i < lt->defered_operands_size; ++i) {

        Inst_Addr addr = basm_find_label_addr(lt,lt->defered_operands[i].label);
        bm->program[lt->defered_operands[i].addr].operand.as_u64 = addr;

    }
}

String_View sv_slurp_file(const char *file_path)
{
  FILE *f = fopen(file_path, "r");
  if (f == NULL) {
     fprintf(stderr, "ERROR: could not read file %s : %s\n", file_path, strerror(errno));
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

  char *buffer = malloc(m);
  if (buffer == NULL) {
    fprintf(stderr, "ERROR: could not allocate memory for file: %s\n", strerror(errno));
     exit(1);
  }

  if (fseek(f, 0, SEEK_SET) < 0) {
     fprintf(stderr, "ERROR: could not read file %s : %s\n", file_path, strerror(errno));
     exit(1);
  }

  size_t n = fread(buffer, 1, m, f);
  if (ferror(f)) {
     fprintf(stderr, "ERROR: could not read file %s : %s\n", file_path, strerror(errno));
     exit(1);
  }

  fclose(f);

  return (String_View) {
    .count = n,
    .data = buffer,
  };
}

#endif// BM_IMPLEMENTATION