#include "./include/assembler.h"

#include <stdio.h>

#include "checker.c"
#include "lexer.c"

fart_assembler *fart_assembler_init(fart_lexer *lexer)
{
    fart_token *tokens = fart_lexer_run(lexer);
    if (tokens == NULL)
    {
        puts("failed to lexerize source.");
        return NULL;
    }

    int result = run_checker(lexer, tokens);
    if (result != -1)
    {
        printf("bracket error at %d.\n", result);
        return NULL;
    }

    fart_assembler *assembler = malloc(sizeof(fart_assembler));
    if (assembler == NULL)
    {
        puts("alloc failed for assembler.");
        return NULL;
    }

    assembler->lexer = lexer;
    assembler->binary = calloc(lexer->binary_size, 1);
    assembler->tokens = tokens;

    // xor bx, bx
    assembler->binary[0] = 0x31;
    assembler->binary[1] = 0xDB;

    return assembler;
}

void fart_assembler_run(fart_assembler *assembler)
{
    size_t index = 0, byte_index = 2;

    size_t cell_start = (assembler->lexer->binary_size - 4096) + 0x100;
    unsigned char cell_offset[2] = {cell_start & 0x00FF, cell_start >> 8};

    for (;;)
    {
        fart_token token = assembler->tokens[index];

        switch (token.kind)
        {
        case FART_TOKEN_PLUS: {
            // add [offset + bx], <value>
            unsigned char opcodes[5] = {0x80, 0x87, 0x00, 0x00, 0x00};
            opcodes[2] = cell_offset[0];
            opcodes[3] = cell_offset[1];
            opcodes[4] = token.value;

            ITER_WRITE_OP(5)
            break;
        }
        case FART_TOKEN_MINUS: {
            // sub [offset + bx], <value>
            unsigned char opcodes[5] = {0x80, 0xAF, 0x00, 0x00, 0x00};
            opcodes[2] = cell_offset[0];
            opcodes[3] = cell_offset[1];
            opcodes[4] = token.value;

            ITER_WRITE_OP(5)
            break;
        }
        case FART_TOKEN_NEXT: {
            //    add bx, <value>
            //    cmp bx, 4096
            //    jl L
            //    sub bx, 4096
            // L:
            unsigned char opcodes[14] = {0x81, 0xC3, 0x00, 0x00, 0x81, 0xFB, 0x00,
                                         0x10, 0x7C, 0x04, 0x81, 0xEB, 0x00, 0x10};

            opcodes[2] = token.value & 0x00FF;
            opcodes[3] = token.value >> 8;

            ITER_WRITE_OP(14)
            break;
        }
        case FART_TOKEN_BACK: {
            //    cmp bx, <value>
            //    jnl L
            //    add bx, 4096
            // L: sub bx, <value>
            unsigned char opcodes[14] = {0x81, 0xFB, 0x00, 0x00, 0x7D, 0x04, 0x81,
                                         0xC3, 0x00, 0x10, 0x81, 0xEB, 0x00, 0x00};

            opcodes[2] = token.value & 0x00FF;
            opcodes[3] = token.value >> 8;

            opcodes[12] = opcodes[2];
            opcodes[13] = opcodes[3];

            ITER_WRITE_OP(14)
            break;
        }
        case FART_TOKEN_OUTPUT: {
            // mov dl, [offset + bx]
            // mov ah, 0eh
            // int 10h
            unsigned char opcodes[8] = {0x8A, 0x87, 0x00, 0x00, 0xB4, 0x0E, 0xCD, 0x10};

            opcodes[2] = cell_offset[0];
            opcodes[3] = cell_offset[1];

            ITER_WRITE_OP(8)
            break;
        }
        case FART_TOKEN_INPUT: {
            // mov ah, 00h
            // int 16h
            // mov byte [offset + bx], al
            unsigned char opcodes[8] = {0xB4, 0x00, 0xCD, 0x16, 0x88, 0x87, 0x00, 0x00};

            opcodes[6] = cell_offset[0];
            opcodes[7] = cell_offset[1];

            ITER_WRITE_OP(8)
            break;
        }
        case FART_TOKEN_LOOP_OPEN: {
            fart_jump_table loop_positions = assembler->lexer->jump_table[token.value];

            size_t jump_end_position = loop_positions.end - loop_positions.start;
            unsigned char jump_end_offset[2] = {jump_end_position & 0x00FF, jump_end_position >> 8};

            // cmp [offset + bx], 0
            // je near position
            unsigned char opcodes[9] = {0x80, 0xBF, 0x00, 0x00, 0x00, 0x0F, 0x84, 0x00, 0x00};

            opcodes[2] = cell_offset[0];
            opcodes[3] = cell_offset[1];

            opcodes[7] = jump_end_offset[0];
            opcodes[8] = jump_end_offset[1];

            ITER_WRITE_OP(9)
            break;
        }
        case FART_TOKEN_LOOP_CLOSE: {
            fart_jump_table loop_positions = assembler->lexer->jump_table[token.value];

            size_t jump_start_position = (loop_positions.start - loop_positions.end) - 9;
            unsigned char jump_start_offset[2] = {jump_start_position & 0x00FF, jump_start_position >> 8};

            // jmp near position
            unsigned char opcodes[3] = {0xE9, 0x00, 0x00};

            opcodes[1] = jump_start_offset[0];
            opcodes[2] = jump_start_offset[1];

            ITER_WRITE_OP(3)
            break;
        }
        default: {
            // mov ah, 4ch
            // int 21h
            assembler->binary[byte_index++] = 0xB4;
            assembler->binary[byte_index++] = 0x4C;
            assembler->binary[byte_index++] = 0xCD;
            assembler->binary[byte_index++] = 0x21;

            // cells
            for (size_t opcode_index = 0; opcode_index < 4096; opcode_index++)
                assembler->binary[byte_index++] = 0x00;

            return;
        }
        }

        index++;
    }
}

void fart_assembler_free(fart_assembler *assembler)
{
    fart_lexer_free(assembler->lexer);
    free(assembler->binary);
    free(assembler->tokens);
    free(assembler);
}
