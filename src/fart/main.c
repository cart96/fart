#include "../libfart/assembler.c"
#include "file.c"

static char fart_info[] = "fart v0.1.1 - brainfuck assembler for DOS";

int main(int argc, char **argv)
{
    puts(fart_info);

    if (argc < 2)
    {
        puts("usage: fart <input file> [output file]");
        return -1;
    }

    char *content = read_file(argv[1]);
    char *output = argc > 2 ? argv[2] : "A.COM";

    if (content == NULL)
    {
        puts("failed to read file.");
        return -1;
    }

    fart_lexer *lexer = fart_lexer_init(content);
    if (lexer == NULL)
    {
        free(content);
        return -1;
    }

    fart_assembler *assembler = fart_assembler_init(lexer);
    if (assembler == NULL)
    {
        fart_lexer_free(lexer);
        free(content);
        return -1;
    }

    fart_assembler_run(assembler);

    if (write_file(output, assembler->binary, assembler->lexer->binary_size) == 0)
    {
        puts("failed to write output.");

        fart_assembler_free(assembler);
        free(content);

        return -1;
    }
    else
    {
        printf("passed 3, wrote %zu byte.\n", assembler->lexer->binary_size);

        fart_assembler_free(assembler);
        free(content);

        return 0;
    }
}
