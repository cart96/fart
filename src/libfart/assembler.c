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

    size_t cell_start = CALCULATE_MEMORY_OFFSET(assembler->lexer->binary_size - BINARY_TAIL);
    unsigned char cell_offset[2] = {cell_start & 0x00FF, cell_start >> 8};

    size_t next_cell_func_start =
        assembler->lexer->binary_size - (NEXT_CELL_SIZE + BACK_CELL_SIZE + WRITE_OUTPUT_SIZE + GET_INPUT_SIZE);
    size_t back_cell_func_start = assembler->lexer->binary_size - (BACK_CELL_SIZE + WRITE_OUTPUT_SIZE + GET_INPUT_SIZE);
    size_t write_output_func_start = assembler->lexer->binary_size - (WRITE_OUTPUT_SIZE + GET_INPUT_SIZE);
    size_t get_input_func_start = assembler->lexer->binary_size - GET_INPUT_SIZE;

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

            ITER_WRITE_OP(5, opcodes)
            break;
        }
        case FART_TOKEN_MINUS: {
            // sub [offset + bx], <value>
            unsigned char opcodes[5] = {0x80, 0xAF, 0x00, 0x00, 0x00};
            opcodes[2] = cell_offset[0];
            opcodes[3] = cell_offset[1];
            opcodes[4] = token.value;

            ITER_WRITE_OP(5, opcodes)
            break;
        }
        case FART_TOKEN_NEXT: {
            // add bx, <value>
            // call NEXT_CELL
            unsigned char opcodes[7] = {0x81, 0xC3, 0x00, 0x00, 0xE8, 0x00, 0x00};

            opcodes[2] = token.value & 0x00FF;
            opcodes[3] = token.value >> 8;

            size_t next_cell_func_rel_pos = next_cell_func_start - token.value2;

            opcodes[5] = next_cell_func_rel_pos & 0x00FF;
            opcodes[6] = next_cell_func_rel_pos >> 8;

            ITER_WRITE_OP(7, opcodes)
            break;
        }
        case FART_TOKEN_BACK: {
            // mov ax, <value>
            // call BACK_CELL
            // sub bx, ax
            unsigned char opcodes[8] = {0xB8, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x29, 0xC3};

            opcodes[1] = token.value & 0x00FF;
            opcodes[2] = token.value >> 8;

            size_t back_cell_func_rel_pos = back_cell_func_start - (token.value2 - 2);

            opcodes[4] = back_cell_func_rel_pos & 0x00FF;
            opcodes[5] = back_cell_func_rel_pos >> 8;

            ITER_WRITE_OP(8, opcodes)
            break;
        }
        case FART_TOKEN_OUTPUT: {
            // call WRITE_OUTPUT
            unsigned char opcodes[3] = {0xE8, 0x00, 0x00};

            size_t write_output_func_rel_pos = write_output_func_start - token.value;

            opcodes[1] = write_output_func_rel_pos & 0x00FF;
            opcodes[2] = write_output_func_rel_pos >> 8;

            ITER_WRITE_OP(3, opcodes)
            break;
        }
        case FART_TOKEN_INPUT: {
            // call GET_INPUT
            unsigned char opcodes[3] = {0xE8, 0x00, 0x00};

            size_t get_input_func_rel_pos = get_input_func_start - token.value;

            opcodes[1] = get_input_func_rel_pos & 0x00FF;
            opcodes[2] = get_input_func_rel_pos >> 8;

            ITER_WRITE_OP(3, opcodes)
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

            ITER_WRITE_OP(9, opcodes)
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

            ITER_WRITE_OP(3, opcodes)
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
            for (size_t opcode_index = 0; opcode_index < MEMORY_SIZE; opcode_index++)
                assembler->binary[byte_index++] = 0x00;

            // NEXT_CELL:
            // cmp bx, 4096
            // jl L
            // sub bx, 4096
            // L: ret
            unsigned char next_cell_func[11] = {0x81, 0xFB, 0x00, 0x10, 0x7C, 0x04, 0x81, 0xEB, 0x00, 0x10, 0xC3};
            ITER_WRITE_OP(11, next_cell_func)

            // BACK_CELL:
            // cmp bx, ax
            // jnl L
            // add bx, 4096
            // L: ret
            unsigned char back_cell_func[9] = {0x39, 0xC3, 0x7D, 0x04, 0x81, 0xC3, 0x00, 0x10, 0xC3};
            ITER_WRITE_OP(9, back_cell_func)

            // WRITE_OUTPUT:
            // mov dl, [offset + bx]
            // mov ah, 2
            // int 21h
            // ret
            unsigned char write_output_func[9] = {0x8A, 0x97, cell_offset[0], cell_offset[1], 0xB4, 0x02, 0xCD,
                                                  0x21, 0xC3};
            ITER_WRITE_OP(9, write_output_func)

            // GET_INPUT:
            // mov ah, 7
            // int 21h
            // mov byte [offset + bx], al
            // ret
            unsigned char get_input_func[9] = {0xB4, 0x07,           0xCD,           0x21, 0x88,
                                               0x87, cell_offset[0], cell_offset[1], 0xC3};
            ITER_WRITE_OP(9, get_input_func)

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
