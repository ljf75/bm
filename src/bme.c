#define BM_IMPLEMENTATION
#include "./bm.h"

static char *shift(int *argc, char ***argv)
{
   assert(*argc > 0);
   char *result = **argv;
   *argv += 1;
   *argc -= 1;
   return result;
}

static void usage(FILE *stream, const char *program)
{
  fprintf(stream, "Usage: %s -i <input.bm> [-l <limit>] [-h] [-d]\n", program);
}

static Trap bm_alloc(Bm *bm)
{
  if (bm->stack_size < 1) {
    return TRAP_STACK_UNDERFLOW;
  }

  bm->stack[bm->stack_size - 1].as_ptr = malloc(bm->stack[bm->stack_size - 1].as_u64);

  return TRAP_OK;
}

static Trap bm_free(Bm *bm)
{
  if (bm->stack_size < 1) {
    return TRAP_STACK_UNDERFLOW;
  }
  
  free(bm->stack[bm->stack_size - 1].as_ptr);
  bm->stack_size -= 1;

  return TRAP_OK;
}

static Trap bm_print_f64(Bm *bm)
{
  if (bm->stack_size < 1) {
    return TRAP_STACK_UNDERFLOW;
  }

  printf("%lf\n", bm->stack[bm->stack_size - 1].as_f64);
  bm->stack_size -= 1;
  return TRAP_OK;
}

static Trap bm_print_i64(Bm *bm)
{
  if (bm->stack_size < 1) {
    return TRAP_STACK_UNDERFLOW;
  }

  printf("%" PRId64 "\n", bm->stack[bm->stack_size - 1].as_i64);
  bm->stack_size -= 1;
  return TRAP_OK;
}

static Trap bm_print_u64(Bm *bm)
{
  if (bm->stack_size < 1) {
    return TRAP_STACK_UNDERFLOW;
  }

  printf("%" PRIu64 "\n", bm->stack[bm->stack_size - 1].as_u64);
  bm->stack_size -= 1;
  return TRAP_OK;
}

static Trap bm_print_ptr(Bm *bm)
{
  if (bm->stack_size < 1) {
    return TRAP_STACK_UNDERFLOW;
  }

  printf("%p\n", bm->stack[bm->stack_size - 1].as_ptr);
  bm->stack_size -= 1;
  return TRAP_OK;
}

int main(int argc, char **argv) 
{
 const char *program = shift(&argc, &argv);
 const char *input_file_path = NULL;
 int limit = -1;
 int debug = 0;

  while (argc > 0) {
    const char *flag = shift(&argc, &argv);

    if (strcmp(flag, "-i") == 0) {
      if (argc == 0) {
        usage(stderr, program);
        fprintf(stderr, "ERROR: No argument is provided for flags `%s`\n", flag);
        exit(1);
      }
      input_file_path = shift(&argc, &argv);
    } else if (strcmp(flag, "-l") == 0) {
        if (argc == 0) {
          usage(stderr, program);
          fprintf(stderr, "ERROR: No argument is provided for flags `%s`\n", flag);
          exit(1);
        }
       limit = atoi(shift(&argc, &argv));
    }  else if (strcmp(flag, "-h") == 0) {
      usage(stdout, program);
      exit(0);
    } else if(strcmp(flag, "-d") == 0) {
      debug = 1;
    }  else {
       usage(stderr, program);
       fprintf(stderr, "ERROR: Unknown flag `%s`\n", flag);
       exit(1);
    }
  }

  if (input_file_path == NULL) {
       usage(stderr, program);
       fprintf(stderr, "ERROR: input was not provided\n");
       exit(1);
  }
  
  bm_load_program_from_file(&bm, input_file_path);
  bm_push_native(&bm, bm_alloc);     // 0
  bm_push_native(&bm, bm_free);      // 1
  bm_push_native(&bm, bm_print_f64); // 2
  bm_push_native(&bm, bm_print_i64); // 3
  bm_push_native(&bm, bm_print_u64); // 4
  bm_push_native(&bm, bm_print_ptr); // 5

  if (!debug) {
    Trap trap = bm_execute_program(&bm, limit);

    if (trap != TRAP_OK) {
      fprintf(stderr, "ERROR: %s\n", trap_as_cstr(trap));
      return 1;
    }
  } else {
      while (limit != 0 && !bm.halt) {
        bm_dump_stack(stdout, &bm);
        printf("Instruction: %s %" PRIu64  "\n", inst_name(bm.program[bm.ip].type), bm.program[bm.ip].operand.as_u64);
        getchar();
        Trap trap = bm_execute_inst(&bm);
        if (trap != TRAP_OK) {
            fprintf(stderr, "ERROR: %s\n", trap_as_cstr(trap));
            return 1;
        }
        if (limit > 0) {
          --limit;
        }
      }
  }


  return 0;
}