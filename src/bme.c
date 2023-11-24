#include "./bm.h"

int main(int argc, char **argv) 
{
 if (argc < 2) {
   fprintf(stderr, "./bmi <input.bm>\n");
   fprintf(stderr, "ERROR: expected input\n");
   exit(1);
 }

  bm_load_program_from_file(&bm, argv[1]);

  for (int i = 0; i < 69 && !bm.halt; ++i) {
    Trap trap = bm_execute_inst(&bm);
    bm_dump_stack(stdout, &bm);
    if (trap != TRAP_OK) {
      fprintf(stderr, "ERROR: %s\n", trap_as_cstr(trap));
      exit(1);
    }
  }
  return 0;
}