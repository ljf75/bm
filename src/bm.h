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
#include<stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
#  define PACKED __attribute__((packed))
#else
#  warning "Packed attributes for struct is not implemented for this compiler. This may result in a program working incorrectly. Feel free to fix that and submit a Pull Request to https://github.com/tsoding/bm"
#  define PACKED
#endif

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof(xs[0]))
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_NATIVE_CAPACITY 1024
#define BASM_BINDINGS_CAPACITY 1024
#define DEFERED_OPERANDS_CAPACITY 1024
#define BASM_ARENA_CAPACITY (640 * 1000)

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
#define BASM_MEMORY_CAPACITY (1000 * 1000 * 1000)

typedef enum {
  TRAP_OK = 0,
  TRAP_STACK_OVERFLOW,
  TRAP_STACK_UNDERFLOW,
  TRAP_ILLEGAL_INST,
  TRAP_ILLEGAL_INST_ACCESS,
  TRAP_ILLEGAL_OPERAND,
  TRAP_ILLEGAL_MEMORY_ACCESS,
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
    INST_ANDB,
    INST_ORB,
    INST_XOR,
    INST_SHR,
    INST_SHL,
    INST_NOTB,
    INST_READ8,
    INST_READ16,
    INST_READ32,
    INST_READ64,
    INST_WRITE8,
    INST_WRITE16,
    INST_WRITE32,
    INST_WRITE64,
    NUMBER_OF_INSTS,
} Inst_Type;

const char *inst_name(Inst_Type type);
int inst_has_operand(Inst_Type type);
int inst_by_name(String_View name, Inst_Type *output);

typedef uint64_t Inst_Addr;
typedef uint64_t Memory_Addr;


typedef union {
  uint64_t as_u64;
  int64_t as_i64;
  double as_f64;
  void *as_ptr;
} Word;

Word word_u64(uint64_t u64);
Word word_i64(int64_t i64);
Word word_f64(double f64);
Word word_ptr(void *ptr);

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

   uint8_t memory[BASM_ARENA_CAPACITY];

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
void bm_load_program_from_file(Bm *bm, const char *file_path);

String_View cstr_as_sv(const char *cstr);
String_View sv_trim_left(String_View sv);
String_View sv_trim_right(String_View sv);
String_View sv_trim(String_View sv);
String_View sv_chop_by_delim(String_View *sv, char delim);

#define BM_FILE_MAGIC 0X4D42
#define BM_FILE_VERSION 1

typedef struct {
  uint16_t magic;
  uint16_t version;
  uint64_t program_size; 
  uint64_t memory_size;
  uint64_t memory_capacity;
} PACKED Bm_File_Meta;

typedef struct {
  String_View name;
  Word value;
} Binding;

typedef struct {
  Inst_Addr addr; 
  String_View name;
} Defered_Operand;

typedef struct {
  Binding bindings[BASM_BINDINGS_CAPACITY];
  size_t bindings_size;

  Defered_Operand defered_operands[DEFERED_OPERANDS_CAPACITY];
  size_t defered_operands_size;

  Inst program[BM_STACK_CAPACITY];
  Inst_Addr program_size;
  
  uint8_t memory[BASM_ARENA_CAPACITY];
  size_t memory_size;
  size_t memory_capacity;

  char arena[BASM_ARENA_CAPACITY];
  size_t arena_size;
} Basm;

void *basm_alloc(Basm *basm, size_t size);
String_View basm_slurp_file(Basm *basm, String_View file_path);
int basm_resolve_binding(const Basm *basm, String_View name, Word *output);
int basm_bind_value(Basm *basm, String_View name, Word word);
void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View name);

void basm_save_to_file(Basm *basm, const char *output_file_path);
void basm_translate_source(Basm *basm, String_View input_file_path, size_t level);

Word basm_push_string_to_memory(Basm *basm, String_View sv);
bool basm_translate_literal(Basm *basm, String_View sv, Word *output);

#endif // __BM_H_

#ifdef BM_IMPLEMENTATION

Word word_u64(uint64_t u64)
{
    return (Word) {.as_u64 = u64};
}

Word word_i64(int64_t i64)
{
    return (Word) {.as_i64 = i64};
}

Word word_f64(double f64)
{
    return (Word) {.as_f64 = f64};
}

Word word_ptr(void *ptr)
{
    return (Word) {.as_ptr = ptr};
}


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
       case INST_ANDB: return 0;
       case INST_ORB: return 0;
       case INST_XOR: return 0;
       case INST_SHR: return 0;
       case INST_SHL: return 0;
       case INST_NOTB: return 0;
       case INST_READ8: return 0;
       case INST_READ16: return 0;
       case INST_READ32: return 0;
       case INST_READ64: return 0;
       case INST_WRITE8: return 0;
       case INST_WRITE16: return 0;
       case INST_WRITE32: return 0;
       case INST_WRITE64: return 0;
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
       case INST_ANDB: return "andb";
       case INST_ORB: return "orb";
       case INST_XOR: return "xor";
       case INST_SHR: return "shr";
       case INST_SHL: return "shl";
       case INST_NOTB: return "not";
       case INST_READ8: return "read8";
       case INST_READ16: return "read16";
       case INST_READ32: return "read32";
       case INST_READ64: return "read64";
       case INST_WRITE8: return "write8";
       case INST_WRITE16: return "write16";
       case INST_WRITE32: return "write32";
       case INST_WRITE64: return "write64";
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
      case TRAP_ILLEGAL_MEMORY_ACCESS:
          return "TRAP_ILLEGAL_MEMORY_ACCESS";
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
     case INST_ANDB: return "INST_ANDB";
     case INST_ORB: return "INST_ORB";
     case INST_XOR: return "INST_XOR";
     case INST_SHR: return "INST_SHR";
     case INST_SHL: return "INST_SHL";
     case INST_NOTB: return "INST_NOTB";
     case INST_READ8: return "INST_READ8";
     case INST_READ16: return "INST_READ16";
     case INST_READ32: return "INST_READ32";
     case INST_READ64: return "INST_READ64";
     case INST_WRITE8: return "INST_WRITE8";
     case INST_WRITE16: return "INST_WRITE16";
     case INST_WRITE32: return "INST_WRITE32";
     case INST_WRITE64: return "INST_WRITE64";
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

    case INST_ANDB:
        if (bm->stack_size < 2) {
        return TRAP_STACK_UNDERFLOW;
      } 

        bm->stack[bm->stack_size - 2].as_u64 =  bm->stack[bm->stack_size - 2].as_u64 & bm->stack[bm->stack_size - 1].as_u64;
        bm->stack_size -= 1;
        bm->ip += 1;
        break;
    
    case INST_ORB:
        if (bm->stack_size < 2) {
          return TRAP_STACK_UNDERFLOW;
        } 

        bm->stack[bm->stack_size - 2].as_u64 =  bm->stack[bm->stack_size - 2].as_u64 | bm->stack[bm->stack_size - 1].as_u64;
        bm->stack_size -= 1;
        bm->ip += 1;
        break;
    
    case INST_XOR: 
        if (bm->stack_size < 2) {
          return TRAP_STACK_UNDERFLOW;
        } 
    
        bm->stack[bm->stack_size - 2].as_u64 =  bm->stack[bm->stack_size - 2].as_u64 ^ bm->stack[bm->stack_size - 1].as_u64;
        bm->stack_size -= 1;
        bm->ip += 1;
        break;
    
    case INST_SHR:
        if (bm->stack_size < 2) {
          return TRAP_STACK_UNDERFLOW;
        } 

        bm->stack[bm->stack_size - 2].as_u64 =  bm->stack[bm->stack_size - 2].as_u64 >>  bm->stack[bm->stack_size - 1].as_u64;
        bm->stack_size -= 1;
        bm->ip += 1;
        break;
    
    case INST_SHL:
        if (bm->stack_size < 2) {
          return TRAP_STACK_UNDERFLOW;
        } 
    
        bm->stack[bm->stack_size - 2].as_u64 =  bm->stack[bm->stack_size - 2].as_u64 <<  bm->stack[bm->stack_size - 1].as_u64;
        bm->stack_size -= 1;
        bm->ip += 1;
        break;
    
    case INST_NOTB: 
        if (bm->stack_size < 1) {
          return TRAP_STACK_UNDERFLOW;
        } 
    
        bm->stack[bm->stack_size - 1].as_u64 =  ~bm->stack[bm->stack_size - 1].as_u64;
        bm->ip += 1;
        break;

    case INST_READ8: {
        if (bm->stack_size < 1) {
           return TRAP_STACK_UNDERFLOW;
        }
        const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
        if (addr >= BASM_ARENA_CAPACITY) {
           return TRAP_ILLEGAL_MEMORY_ACCESS;
        }
        bm->stack[bm->stack_size - 1].as_u64 = *(uint16_t*)&bm->memory[addr];
        bm->ip += 1;
    } break;
    
    case INST_READ16: {
        if (bm->stack_size < 1) {
           return TRAP_STACK_UNDERFLOW;
        }
        const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
        if (addr >= BASM_ARENA_CAPACITY - 1) {
           return TRAP_ILLEGAL_MEMORY_ACCESS;
        }
        bm->stack[bm->stack_size - 1].as_u64 = *(uint16_t*)&bm->memory[addr];
        bm->ip += 1;
    } break;
    
    case INST_READ32: {
        if (bm->stack_size < 1) {
           return TRAP_STACK_UNDERFLOW;
        }
        const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
        if (addr >= BASM_ARENA_CAPACITY - 3) {
          return TRAP_ILLEGAL_MEMORY_ACCESS;
        }
        bm->stack[bm->stack_size - 1].as_u64 = *(uint32_t*)&bm->memory[addr];
        bm->ip += 1;
    } break;
    
    case INST_READ64: {
        if (bm->stack_size < 1) {
           return TRAP_STACK_UNDERFLOW;
        }
        const Memory_Addr addr = bm->stack[bm->stack_size - 1].as_u64;
        if (addr >= BASM_ARENA_CAPACITY - 7) {
          return TRAP_ILLEGAL_MEMORY_ACCESS;
        }
        bm->stack[bm->stack_size - 1].as_u64 = *(uint64_t*)&bm->memory[addr];
        bm->ip += 1;
    }  break;
    
    case INST_WRITE8: {
         if (bm->stack_size < 2) {
            return TRAP_STACK_UNDERFLOW;
          }
          const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
          if (addr >= BASM_ARENA_CAPACITY) {
             return TRAP_ILLEGAL_MEMORY_ACCESS;
          }
          bm->memory[addr] = (uint8_t) bm->stack[bm->stack_size - 1].as_u64;
          bm->stack_size -= 2;
          bm->ip += 1;
    } break;
    
    case INST_WRITE16: {
         if (bm->stack_size < 2) {
            return TRAP_STACK_UNDERFLOW;
          }
          const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
          if (addr >= BASM_ARENA_CAPACITY - 1) {
             return TRAP_ILLEGAL_MEMORY_ACCESS;
          }
          *(uint16_t*)&bm->memory[addr] = (uint16_t) bm->stack[bm->stack_size - 1].as_u64;
          bm->stack_size -= 2;
          bm->ip += 1;
    } break;
    
    case INST_WRITE32: {
        if (bm->stack_size < 2) {
          return TRAP_STACK_UNDERFLOW;
        }
        const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
        if (addr >= BASM_ARENA_CAPACITY - 3) {
           return TRAP_ILLEGAL_MEMORY_ACCESS;
        }
        *(uint32_t*)&bm->memory[addr] = (uint32_t) bm->stack[bm->stack_size - 1].as_u64;
        bm->stack_size -= 2;
        bm->ip += 1;
    } break;
    
    case INST_WRITE64: {
          if (bm->stack_size < 2) {
            return TRAP_STACK_UNDERFLOW;
          }
          const Memory_Addr addr = bm->stack[bm->stack_size - 2].as_u64;
          if (addr >= BASM_ARENA_CAPACITY - 7) {
             return TRAP_ILLEGAL_MEMORY_ACCESS;
          }
          *(uint64_t*)&bm->memory[addr] = (uint64_t) bm->stack[bm->stack_size - 1].as_u64;
          bm->stack_size -= 2;
          bm->ip += 1;
      } break;
    
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

void bm_load_program_from_file(Bm *bm, const char *file_path)
{
   FILE *f = fopen(file_path, "rb");
   if (f == NULL) {
     fprintf(stderr, "ERROR: could not open file %s : %s\n", file_path, strerror(errno));
     exit(1);
   }
  
  Bm_File_Meta meta = {0};

  size_t n = fread(&meta, sizeof(meta), 1, f);
  if (n < 1) {
    fprintf(stderr, "ERROR: Could not read meta data from file `%s`: %s\n", 
            file_path, strerror(errno));
    exit(1);
  }

  if(meta.magic != BM_FILE_MAGIC) {
    fprintf(stderr, "ERROR: %s does not appear to be a valid BM file. "
      "Unexpected magic %04X. Expected %04X.\n",
      file_path,
      meta.magic, BM_FILE_MAGIC);
    exit(1);
  }

  if (meta.version != BM_FILE_VERSION) {
    fprintf(stderr, "ERROR: %s: unsupported version of BM file %d. Expected version %d.\n", 
      file_path,
      meta.version, BM_FILE_VERSION);
    exit(1);
  }

  if (meta.program_size > BM_PROGRAM_CAPACITY) {
    fprintf(stderr, "ERROR: %s: program section is too big. The file contains %" PRIu64 "program instruction. But the capacity is %" PRIu64 "\n", file_path, meta.program_size, (uint64_t) BM_PROGRAM_CAPACITY);
    exit(1);
  }


  if (meta.memory_capacity > BASM_MEMORY_CAPACITY) {
    fprintf(stderr, "ERROR: %s: memory section is too big. The file wants %" PRIu64 " bytes. But the capacity is %" PRIu64 "\n", file_path, meta.memory_capacity, (uint64_t) BASM_MEMORY_CAPACITY);
    exit(1);
  }

  if (meta.memory_size < meta.memory_capacity) {
    fprintf(stderr, "ERROR: %s: memory size %" PRIu64 "is greater than delcared memory capacity %" PRIu64 "\n", file_path, meta.memory_size, meta.memory_capacity);
    exit(1);
  }

  bm->program_size = fread(bm->program, sizeof(bm->program[0]), meta.program_size, f);

  if (bm->program_size != meta.program_size) {
    fprintf(stderr, "ERROR: %s: read %zd program instructions, but expetced %" PRIu64 "\n",
      file_path,
      bm->program_size,
      meta.program_size);
    exit(1);
  }
  
  n = fread(bm->memory, sizeof(bm->memory[0]), meta.memory_size, f);

  if (n != meta.memory_size) {
    fprintf(stderr, "ERROR: %s: read %zd bytes of memory section, but expetced %" PRIu64 " bytes\n",
      file_path,
      n,
      meta.memory_size);
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

void *basm_alloc(Basm *basm, size_t size)
{
  assert(basm->arena_size + size <= BASM_MEMORY_CAPACITY);

  void *result = basm->arena + basm->arena_size;
  basm->arena_size += size;
  return result;
}

int basm_resolve_binding(const Basm *basm, String_View name, Word *output)
{
    for (size_t i = 0; i < basm->bindings_size; ++i) {
      if (sv_eq(basm->bindings[i].name, name)) {
         *output = basm->bindings[i].value;
         return 1;
      }
    }
    return 0;
}

int basm_bind_value(Basm *basm, String_View name, Word word)
{
  assert(basm->bindings_size < BASM_BINDINGS_CAPACITY);
 
  Word ignore = {0};
  if ( basm_resolve_binding(basm, name, &ignore)) {
    return 0;
  }
    
  basm->bindings[basm->bindings_size++] = (Binding) {.name = name, .value = word};
  return 1;
}

void basm_push_defered_operand(Basm *basm, Inst_Addr addr, String_View name)
{
  assert(basm->defered_operands_size < DEFERED_OPERANDS_CAPACITY);
  basm->defered_operands[basm->defered_operands_size++] =    
      (Defered_Operand) {.addr = addr, .name = name};
}

Word basm_push_string_to_memory(Basm *basm, String_View sv)
{
  assert(basm->memory_size + sv.count <= BASM_MEMORY_CAPACITY);

  Word result = word_u64(basm->memory_size);

  memcpy(basm->memory + basm->memory_size, sv.data, sv.count);

  basm->memory_size += sv.count;

  if (basm->memory_size > basm->memory_capacity) {
       basm->memory_capacity = basm->memory_size;   
  }
  
  return result;
}

bool basm_translate_literal(Basm *basm, String_View sv, Word *output)
{
  if (sv.count >= 2 && *sv.data == '"' && sv.data[sv.count - 1] == '"') {
      sv.data += 1;
      sv.count -= 2;  
      *output = basm_push_string_to_memory(basm, sv);
  } else {
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
           return false;
         } 
      } 
  
      *output = result;
  }
  
  return true;
}

void basm_save_to_file(Basm *basm, const char *output_file_path)
{
    FILE *f = fopen(output_file_path, "wb");
    if (f == NULL) {
      fprintf(stderr, "ERROR: could not open file %s : %s\n", output_file_path, strerror(errno));
      exit(1);
    }

    Bm_File_Meta meta = {
        .magic = BM_FILE_MAGIC,
        .version = BM_FILE_VERSION,
        .program_size = basm->program_size,
        .memory_size = basm->memory_size,
        .memory_capacity = basm->memory_capacity,
    };
  
    fwrite(&meta, sizeof(meta), 1, f);
    if (ferror(f)) {
      fprintf(stderr, "ERROR: could not write to file %s : %s\n", output_file_path, strerror(errno));
      exit(1);
    }
  
    fwrite(basm->program, sizeof(basm->program[0]), basm->program_size, f);  
    if (ferror(f)) {
      fprintf(stderr, "ERROR: could not write to file %s : %s\n", output_file_path, strerror(errno));
      exit(1);
    }

    fwrite(basm->memory, sizeof(basm->memory[0]), basm->memory_size, f);  
    if (ferror(f)) {
      fprintf(stderr, "ERROR: could not write to file %s : %s\n", output_file_path, strerror(errno));
      exit(1);
    }
  
    fclose(f);
}

void basm_translate_source(Basm *basm, String_View input_file_path, size_t level)
{
   String_View original_source = basm_slurp_file(basm, input_file_path);
   String_View source = original_source;
  
   int line_number = 0;

   // First pass
   while (source.count > 0) {
     String_View line = sv_trim(sv_chop_by_delim(&source, '\n'));     line_number += 1;
     if (line.count > 0 && *line.data != BASM_COMMENT_SYMBOL) {
        String_View token = sv_trim(sv_chop_by_delim(&line, ' '));

        // Pre-processor
        if (token.count > 0 && *token.data == BASM_PP_SYMBOL) {
           token.count -= 1;
           token.data += 1;
           if (sv_eq(token, cstr_as_sv("bind"))) {
              line = sv_trim(line);
              String_View name = sv_chop_by_delim(&line, ' ');
              if (name.count > 0) {
                line = sv_trim(line);
                String_View value = line;//sv_chop_by_delim(&line, ' ');
                Word word = {0};
                if (!basm_translate_literal(basm, value, &word)) {
                  fprintf(stderr, "%.*s:%d: ERROR: `%.*s` is not a number\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(value));
                   exit(1);
                }
                
                if (!basm_bind_value(basm, name, word)) {
                       fprintf(stderr, "%.*s:%d: ERROR: name `%.*s` is already bound\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(name));
                        exit(1);
                }
              } else {
                      fprintf(stderr, "%.*s:%d: ERROR: bind name is not provided\n", SV_FORMAT(input_file_path), line_number);
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
                    
                    basm_translate_source(basm,  line, level + 1);
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
          // binding
            if (token.count > 0 && token.data[token.count - 1] == ':') {
              String_View name = {
                 .count = token.count - 1,
                 .data = token.data,
              };

              if (!basm_bind_value(basm, name, word_u64(basm->program_size))) {
                // TODO: name redefinition does not tell where the name was already defined
                     fprintf(stderr, "%.*s:%d: ERROR: name `%.*s` is already bound\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(name));
                      exit(1);
              }

              token = sv_trim(sv_chop_by_delim(&line, ' '));
            } 

           // Instruction
           if (token.count > 0) {
               String_View operand = sv_trim(sv_chop_by_delim(&line, BASM_COMMENT_SYMBOL));

               Inst_Type inst_type = INST_NOP;
               if (inst_by_name(token, &inst_type)) {
                 assert(basm->program_size < BM_PROGRAM_CAPACITY);
                 basm->program[basm->program_size].type = inst_type;

                 if (inst_has_operand(inst_type)) {
                   if (operand.count == 0) {
                     fprintf(stderr, "%.*s:%d: ERROR: instruction `%.*s` requires an operand\n", SV_FORMAT(input_file_path), line_number, SV_FORMAT(token));
                     exit(1);
                   }
                   if (!basm_translate_literal(basm, operand, &basm->program[basm->program_size].operand)) {                
                    basm_push_defered_operand(
                     basm, basm->program_size, operand);
                   }
                 }
                 basm->program_size += 1;
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
        String_View name =  basm->defered_operands[i].name;

        if (! basm_resolve_binding(
          basm,
          name,
          &basm->program[basm->defered_operands[i].addr].operand)) {
                  // TODO: second pass resolutiion error
                  fprintf(stderr, "%.*s: ERROR: unknown name `%.*s`\n", SV_FORMAT(input_file_path), SV_FORMAT(name));
                  exit(1);
        }
    }
}

String_View basm_slurp_file(Basm *basm, String_View file_path)
{
  char *file_path_cstr = basm_alloc(basm, file_path.count + 1);

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

  char *buffer = basm_alloc(basm, (size_t) m);
  if (buffer == NULL) {
    fprintf(stderr, "ERROR: could not allocate memory for file: %s\n", strerror(errno));
     exit(1);
  }

  if (fseek(f, 0, SEEK_SET) < 0) {
     fprintf(stderr, "ERROR: could not read file `%s` : %s\n", file_path_cstr, strerror(errno));
     exit(1);
  }

  size_t n = fread(buffer, 1, (size_t) m, f);
  if (ferror(f)) {
     fprintf(stderr, "ERROR: could not read file `%s` : %s\n", file_path_cstr, strerror(errno));
     exit(1);
  }

  fclose(f);
  
  return (String_View) {
    .count = n,
    .data = buffer,
  };
}

#endif// BM_IMPLEMENTATION