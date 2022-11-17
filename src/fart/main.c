#include "../libfart/assembler.c"
#include "file.c"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        puts("usage: fart input.bf output.com");
        return -1;
    }

    char *content = read_file(argv[1]);
    char *output = argv[2];

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
        free(content);
        return -1;
    }

    fart_assembler_run(assembler);

    write_file(output, assembler->binary, assembler->lexer->binary_size);
    printf("passed 3, wrote %zu byte.\n", assembler->lexer->binary_size);

    fart_assembler_free(assembler);
    free(content);

    return 0;
}
