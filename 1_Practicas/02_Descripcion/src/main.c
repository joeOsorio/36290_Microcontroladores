/******************************************************************************
Prac 2 - AVR ASM OpCode Decoder
*******************************************************************************/

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h> /* Para comvertir a cadena decimal*/

void imprimir_binario16(uint16_t valor);
char *uint16_a_decimal(uint16_t valor, char *buffer);
/*
const uint8_t flash_mem[] = {
    0xf2, 0xe0, 0xdf, 0x2f, 0x08, 0xe1, 0x01, 0x93, 0x03, 0xe1, 0x01, 0x93, 0x05, 0xe1, 0x01, 0x93, 0x08, 0xe0, 0x01, 0x93, 0x03, 0xe0, 0x01, 0x93, 0x09, 0xe0, 0x01, 0x93, 0xa5, 0xe0, 0x5a, 0x2e, 0xa4, 0xe1, 0x22, 0x27, 0x09, 0x91, 0x0f, 0x71, 0x10, 0xe0, 0x22, 0x23, 0x09, 0xf0, 0x11, 0xd0, 0x3c, 0x91, 0x30, 0x2b, 0x3c, 0x93, 0x24, 0x30, 0x24, 0xf0, 0xa3, 0x95, 0x3c, 0x91, 0x31, 0x2b, 0x3c, 0x93, 0x25, 0x0d, 0x28, 0x30, 0x0c, 0xf0, 0x28, 0x50, 0xec, 0x17, 0x59, 0xf7, 0x00, 0x00, 0xfe, 0xcf, 0x2f, 0x93, 0x07, 0xfb, 0x00, 0x0f, 0x11, 0x0f, 0x10, 0xf9, 0x2a, 0x95, 0xd1, 0xf7, 0x2f, 0x91, 0x08, 0x95};
*/

const uint8_t flash_mem[] = {
    0xf2, 0xe0, 0xdf, 0x2f, 0x08, 0xe1, 0x01, 0x93, 0x03, 0xe1, 0x01, 0x93, 0x05, 0xe1, 0x01, 0x93, 0x08, 0xe0,
    0x01, 0x93, 0x03, 0xe0, 0x01, 0x93, 0x09, 0xe0, 0x01, 0x93, 0xa5, 0xe0, 0x5a, 0x2e, 0xa4, 0xe1, 0x22, 0x27,
    0x09, 0x91, 0x0f, 0x71, 0x10, 0xe0, 0x22, 0x23, 0x09, 0xf0, 0x11, 0xd0, 0x3c, 0x91, 0x30, 0x2b, 0x3c, 0x93,
    0x24, 0x30, 0x24, 0xf0, 0xa3, 0x95, 0x3c, 0x91, 0x31, 0x2b, 0x3c, 0x93, 0x25, 0x0d, 0x28, 0x30, 0x0c, 0xf0,
    0x28, 0x50, 0xec, 0x17, 0x59, 0xf7, 0x00, 0x00, 0xfe, 0xcf, 0x2f, 0x93, 0x07, 0xfb, 0x00, 0x0f, 0x11, 0x0f,
    0x10, 0xf9, 0x2a, 0x95, 0xd1, 0xf7, 0x2f, 0x91, 0x08, 0x95};

const uint16_t inst16_table[] = {
    0x0000, /* NOP */
    0x2C00, /* MOV */
    0xE000, /* LDI */
    0X9C00, /* MUL */
    0X9000, /* ST II */
    0x2400, /* CLR */
    0x2400, /* EOR */
    0x7000, /* ANDI */
    0x2322, /* AND */
    0x9508, /* RET */
    0xF001, /* BREQ */

    0xFFFF /* UABC */
};
/* Para Op code de 7 bits 0001 11rd dddd rrrr*/

enum
{
    e_NOP,
    e_MOV,
    e_LDI,
    e_MUL,
    e_ST,
    e_CLR,
    e_EOR,
    e_ANDI,
    e_AND,
    e_RET,
    e_BREQ,
    e_UABC
};
/*
Ok, al momento de utilizar la unión de Opcode guion bajo t, este, me ahorra todo el trabajo de hacer la línea, ya que, por ejemplo, en el tipo dos ya tiene la estructura canónica del Mult o del caso específico, el LDI, para el caso de Mult, que ya tiene, este, Los los cuatro, los primeros seis beats asignados a la operación de Mule y los demás, bueno, los otros dos beats son...

*/
// Op Code struct
typedef union
{
    uint16_t op16; // e.g.: watchdog, nop
    struct
    {
        uint16_t op4 : 4;
        uint16_t d5 : 5;
        uint16_t op7 : 7;
    } type1; // e.g: LSR
    struct
    {
        uint16_t r4 : 4;
        uint16_t d5 : 5;
        uint16_t r1 : 1;
        uint16_t op6 : 6;
    } type2; // e.g.: MOV,MUL,ADC,ADD,AND,
    // TO-DO: Add more types as needed
    struct
    {
        uint16_t k4 : 4;
        uint16_t d4 : 4;
        uint16_t k4_h : 4;
        uint16_t op4 : 4;
    } type3;
    struct
    {
        uint16_t d10 : 10;
        uint16_t op6 : 6;
    } type4;
    struct
    {
        uint16_t op3 : 3;
        uint16_t k : 7;
        uint16_t op6 : 6;
    } type5;
} Op_Code_t;

int main()
{
    int i = 1;
    char cadena1[6], cadena2[6];
    Op_Code_t *instruction;
    printf("- Practica 2: AVR OpCode -\n");
    // Decode the instructions by cycling through the array
    for (uint8_t idx = 0; idx < sizeof(flash_mem); idx += 2)
    {
        instruction = (Op_Code_t *)&flash_mem[idx];
        printf("\n%d: %X\t->\t", i++, instruction);
        imprimir_binario16(instruction->op16);

        printf("\n");
        if (instruction->op16 == inst16_table[e_NOP])
        {
            printf("NOP\n\n");
        }
        else if ((instruction->type3.op4) << 12 == inst16_table[e_LDI])
        {
            /*
            Description
            Loads an 8-bit constant directly to register 16 to 31.
            Operation:
            (i) Rd ← K
            Syntax:          Operands:                      Program Counter:
            (i) LDI Rd,K     16 ≤ d ≤ 31, 0 ≤ K ≤ 255       PC ← PC + 1
            16-bit Opcode:
            1110 KKKK dddd KKKK
            */
            printf("LDI\tR%s,\t%s\n\n", uint16_a_decimal(((instruction->type3.d4) + 0x10), cadena1), uint16_a_decimal((((instruction->type3.k4_h) << 4) | (instruction->type3.k4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_MOV])
        {
            printf("MOV\tR%s,\tR%s\n\n", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
            continue;
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_MUL])
        {
            char cadena1[6], cadena2[6];
            printf("MUL\tR%s,\tR%s\n\n", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
            continue;
        }
        /*         else if ((instruction->type2.op6) << 10 == inst16_table[e_PUSH])
        {
            char cadena1[6], cadena2[6];
            printf("PUSH\tR%s,\tR%s\n\n", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
            continue;
            } */
        else if ((instruction->type2.op6) << 10 == inst16_table[e_ST])
        { /*
         Operation:                              Comment:
         (ii)    DS(Z) ← Rr, Z ← Z+1             Z: Post incremente

                 Syntax:     Operands:           Program Counter:
         (ii)    ST Z+,      Rr 0 ≤ r ≤ 31       PC ← PC + 1
                 16-bit Opcode :
         (ii)    1001 001r rrrr 0001
       */
            printf("ST\tZ+,\tR%s\n\n", uint16_a_decimal(instruction->type2.d5, cadena1));
            continue;
        }
        else if ((instruction->type4.op6) << 10 == inst16_table[e_CLR] && (instruction->type4.d10) <= 0x1f)
        { /*
            Operation:                              Comment:
            (i)     Rd ← Rd ⊕ Rd

            Syntax:     Operands:           Program Counter:
            (i)     CLR Rd      0 ≤ d ≤ 31          PC ← PC + 1
            16-bit Opcode :
            (i)     0010 01dd dddd dddd
            */
            printf("CLR\tR%s\n\n", uint16_a_decimal(instruction->type2.d5, cadena1));
            continue;
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_EOR])
        { /*
            Operation:                                     Comment:
            (i)     Rd ← Rd ⊕ Rr

                    Syntax:     Operands:               Program Counter:
            (i)     EOR Rd,Rr   0 ≤ d ≤ 31, 0 ≤ r ≤ 31  PC ← PC + 1
            16-bit Opcode :
            (i)     0010 01rd dddd rrrr
            */
            printf("EOR\tR%s,\tR%s\n\n", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_ANDI])
        { /*
            Operation:                                      Comment:
            (i)     Rd ← Rd ∧ K

                    Syntax:     Operands:                   Program Counter:
            (i)     ANDI Rd,K   16 ≤ d ≤ 31, 0 ≤ K ≤ 255    PC ← PC + 1
            16-bit Opcode :
            (i)     0111 KKKK dddd KKKK
            */
            printf("ANDI\tR%s,\t%s\n\n", uint16_a_decimal(instruction->type3.d4, cadena1), uint16_a_decimal((((instruction->type3.k4_h) << 4) | (instruction->type3.k4)), cadena2));
        }
        else if ((instruction->op16) == inst16_table[e_RET])
        { /*
            Description
            Returns from the subroutine. The return address is loaded from the STACK. The Stack Pointer uses a pre-increment
            scheme during RET.
            Operation:
            Operation: Comment:
            (i) PC(15:0) ← STACK Devices with 16-bit PC, 128 KB program memory maximum.
            (ii) PC(21:0) ← STACK Devices with 22-bit PC, 8 MB program memory maximum.
            Syntax: Operands: Program Counter: Stack:
            (i) RET None See Operation SP ← SP + 2, (2 bytes,16
            bits)
            (ii) RET None See Operation SP ← SP + 3, (3 bytes,22
            bits)
            16-bit Opcode:
            1001 0101 0000 1000
            */
            printf("RET\n\n");
        }
        else if ((instruction->op16) == inst16_table[e_BREQ])
        { /*
            Description
Conditional relative branch. Tests the Zero (Z) flag and branches relatively to the PC if Z is set. If the instruction is
executed immediately after any of the instructions CP, CPI, SUB, or SUBI, the branch will occur only if the unsigned
or signed binary number represented in Rd was equal to the unsigned or signed binary number represented in Rr.
This instruction branches relatively to the PC in either direction (PC - 63 ≤ destination ≤ PC + 64). Parameter k is the
offset from the PC and is represented in two’s complement form. (Equivalent to instruction BRBS 1,k.)
Operation:
(i) If Rd == Rr (Z == 1) then PC ← PC + k + 1, else PC ← PC + 1
Syntax: Operands: Program Counter:
(i) BREQ k -64 ≤ k ≤ +63 PC ← PC + k + 1
PC ← PC + 1, if the condition is
false
16-bit Opcode:
1111 00kk kkkk k001
            */
            uint16_t k = instruction->type5.k << 4;
            printf("BREQ\t%x\n\n", k);
        }
        else if ((instruction->op16) == inst16_table[e_BREQ])
        {
            printf("BREQ\t%x\n\n");
        }
        else if ((instruction->op16) == inst16_table[e_BREQ])
        {
            uint16_t k = instruction->type5.k << 4;
            printf("BREQ\t%x\n\n", k);
        }
        else if ((instruction->op16) == inst16_table[e_BREQ])
        {
            uint16_t k = instruction->type5.k << 4;
            printf("BREQ\t%x\n\n", k);
        }
        else if ((instruction->op16) == inst16_table[e_BREQ])
        {
            uint16_t k = instruction->type5.k << 4;
            printf("BREQ\t%x\n\n", k);
        }

        else
            printf("\nunknown\n");

        /*
                {
                    uint16_t tmp = (instruction->type2.op6) << 10;
                    imprimir_binario16(((uint16_t)(tmp)));
                    switch (tmp)
                    {
                    case 0x9C00:
                        printf("MUL\n\n\n\n\n");
                        break;
                    case 0x2C00:
                        printf("\n");

                        break;

                    default:
                        break;
                    }
                } */
    }
    return 0;
}

void imprimir_binario16(uint16_t valor)
{
    for (int8_t bit = 15; bit >= 0; bit--)
    {
        putchar((valor & (1u << bit)) ? '1' : '0');
        if (bit % 4 == 0 && bit != 0)
            putchar(' ');
    }
}

char *uint16_a_decimal(uint16_t valor, char *buffer)
{
    char temp[6];
    int pos = 0;

    if (valor == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return buffer;
    }

    while (valor > 0)
    {
        temp[pos++] = (valor % 10) + '0'; // saca el dígito menos significativo
        valor /= 10;
    }

    // los dígitos salieron al revés (unidades primero), hay que invertirlos
    for (int i = 0; i < pos; i++)
        buffer[i] = temp[pos - 1 - i];
    buffer[pos] = '\0';

    return buffer;
}