#define BM_IMPLEMENTATION
#include "./bm.h"

Basm basm = {0};

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
  fprintf(stream, "Usage: %s <input.basm> <output.bm>\n", program);
}

int main(int argc, char **argv)
{  
  const char *program = shift(&argc, &argv); // skip program name

  if (argc == 0) {
    usage(stderr, program);
    fprintf(stderr, "./basm <input.basm> <output.bm>\n");
    fprintf(stderr, "ERROR: expected input\n");
    exit(1);
  }

  const char *input_file_path = shift(&argc, &argv);
  if (argc == 0) {
    usage(stderr, program);
    fprintf(stderr, "./basm <input.basm> <output.bm>\n");
    fprintf(stderr, "ERROR: expected output\n");
    exit(1);
  }
  
  const char *output_file_path = shift(&argc, &argv);

  bm_translate_source(&bm, &basm, cstr_as_sv(input_file_path), 0);

  bm_save_program_to_file(&bm, output_file_path);

  printf("Consumed %zd bytes of memory\n", basm.memory_size);

  return 0;
}