/******************************************************************************
/****************************************************
 * Practica2: AVR ASM OpCode Decoder
 * Author: Joe O^2 *
 * Materia: Algoritmos y Estructura de Datos (551)
 * Date: Sep/04/2026
 * Comments:
 *******************************************************************************/

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h> /* Para comvertir a cadena decimal*/

void imprimir_binario16(uint16_t valor);
char *uint16_a_decimal(uint16_t valor, char *buffer);
/* este esta mal
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

/*
Tabla de "plantillas" de opcode: cada valor solo tiene encendidos los bits
FIJOS de la instrucción (los que nunca cambian), alineados a su posición
real dentro de la palabra de 16 bits. Los bits variables (registros,
constantes K, offsets k, número de bit b, etc.) se dejan en 0 aquí y se
extraen aparte con la unión Op_Code_t.

Nota importante (uno de los errores que tenía): varias instrucciones de
esta lista SOLO se pueden identificar comparando los bits fijos (con el
shift correspondiente), nunca comparando la palabra completa contra un
ejemplo puntual, porque eso solo funciona por casualidad para esa
instancia y falla en cuanto cambian los operandos (me pasaba con BREQ y
con AND).
*/
const uint16_t inst16_table[] = {
    0x0000, /* NOP                              */
    0x2C00, /* MOV   (type2.op6 << 10)          */
    0xE000, /* LDI   (type3.op4 << 12)          */
    0X9C00, /* MUL   (type2.op6 << 10)          */
    0x0C00, /* ADD   (type2.op6 << 10) -> alias LSL si Rd==Rr   */
    0x1400, /* CP    (type2.op6 << 10)          */
    0x2000, /* AND   (type2.op6 << 10) -> alias TST si Rd==Rr   */
    0x2800, /* OR    (type2.op6 << 10)          */
    0x2400, /* EOR   (type2.op6 << 10) -> alias CLR si Rd==Rr   */
    0x7000, /* ANDI  (type3.op4 << 12)          */
    0x3000, /* CPI   (type3.op4 << 12)          */
    0x5000, /* SUBI  (type3.op4 << 12)          */
    0x9200, /* PUSH  (type1.op7 << 9), suf 1111 */
    0x9000, /* POP   (type1.op7 << 9), suf 1111 */
    0x9400, /* DEC   (type1.op7 << 9), suf 1010 */
    0x9400, /* INC   (type1.op7 << 9), suf 0011 */
    0x9000, /* LD Rd,X   (type1.op7 << 9), suf 1100 */
    0x9000, /* LD Rd,Y+  (type1.op7 << 9), suf 1001 */
    0x9200, /* ST X,Rr   (type1.op7 << 9), suf 1100 */
    0x9200, /* ST Z+,Rr  (type1.op7 << 9), suf 0001 */
    0xFA00, /* BST   (type1.op7 << 9)           */
    0xF800, /* BLD   (type1.op7 << 9)           */
    0x9508, /* RET   (palabra completa)         */
    0xD000, /* RCALL (nibble alto, op16 & 0xF000) */
    0xC000, /* RJMP  (nibble alto, op16 & 0xF000) */
    0xF000, /* BRBS  (type4.op6 << 10)          */
    0xF400, /* BRBC  (type4.op6 << 10)          */
            /* 0xFFFF  /* Prueba */
};

enum
{
    e_NOP,
    e_MOV,
    e_LDI,
    e_MUL,
    e_ADD,
    e_CP,
    e_AND,
    e_OR,
    e_EOR,
    e_ANDI,
    e_CPI,
    e_SUBI,
    e_PUSH,
    e_POP,
    e_DEC,
    e_INC,
    e_LD_X,
    e_LD_Yi,
    e_ST_X,
    e_ST_Zi,
    e_BST,
    e_BLD,
    e_RET,
    e_RCALL,
    e_RJMP,
    e_BRBS,
    e_BRBC,
    e_UABC
};
/* Para Op code de 7 bits 0001 11rd dddd rrrr*/

/*
Ok, al momento de utilizar la unión de Opcode guion bajo t, este, me ahorra todo el trabajo de hacer la línea, ya que, por ejemplo, en el tipo dos ya tiene la estructura canónica del Mult o del caso específico, el LDI, para el caso de Mult, que ya tiene, este, Los los cuatro, los primeros seis beats asignados a la operación de Mule y los demás, bueno, los otros dos beats son...

*/
/* Nombres de las 8 banderas del SREG, en el orden real de sus bits (s = 0..7),
   usados por BRBS/BRBC para elegir el mnemónico específico (BREQ, BRNE, BRLT...) */
static const char *sreg_bits[8] = {"C", "Z", "N", "V", "S", "H", "T", "I"};
static const char *brbs_alias[8] = {"BRCS", "BREQ", "BRMI", "BRVS", "BRLT", "BRHS", "BRTS", "BRIE"};
static const char *brbc_alias[8] = {"BRCC", "BRNE", "BRPL", "BRVC", "BRGE", "BRHC", "BRTC", "BRID"};

/* Convierte un valor de "bits" bits (2's complement) a int16_t con signo correcto.
   Se usa para los offsets k de BRBS/BRBC (7 bits) y de RCALL/RJMP (12 bits). */
int16_t extender_signo(uint16_t valor, uint8_t bits)
{
    uint16_t signo = (uint16_t)(1u << (bits - 1));
    if (valor & signo)
    {
        valor |= (uint16_t)(~((uint16_t)(signo << 1) - 1u));
    }
    return (int16_t)valor;
}

// Op Code struct
typedef union
{
    uint16_t op16; // e.g.: watchdog, nop
    struct
    {
        uint16_t op4 : 4;
        uint16_t d5 : 5;
        uint16_t op7 : 7;
    } type1; // e.g: LSR, DEC, PUSH, POP, LD Rd,X / LD Rd,Y+, ST X,Rr / ST Z+,Rr, BST, BLD
    // 7 bits fijos (op7) + 1 registro de 5 bits (d5, bits 8..4) + 4 bits (op4: sufijo fijo o bits b del SREG)
    struct
    {
        uint16_t r4 : 4;
        uint16_t d5 : 5;
        uint16_t r1 : 1;
        uint16_t op6 : 6;
    } type2; // e.g.: MOV,MUL,ADC,ADD,AND,OR,EOR,CP...
    // 6 bits fijos (op6) + Rd de 5 bits (d5) + Rr de 5 bits partido en r1(bit9)+r4(bits3..0)
    struct
    {
        uint16_t k4 : 4;
        uint16_t d4 : 4;
        uint16_t k4_h : 4;
        uint16_t op4 : 4;
    } type3; // LDI, ANDI, CPI, SUBI: Rd = 16 + d4 ; K = (k4_h << 4) | k4
    struct
    {
        uint16_t op3 : 3;
        uint16_t k : 7;
        uint16_t op6 : 6;
    } type4; // BRBS/BRBC: op6 distingue BRBS(0x3C)/BRBC(0x3D), k = offset con signo (7 bits), op3 = s (bit del SREG)
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
        uint8_t pc = idx / 2; // dirección (en palabras) de la instrucción actual, usada por los saltos relativos
        instruction = (Op_Code_t *)&flash_mem[idx];
        printf("\n");
        /*
        printf("\n%d:\t", i++);
        printf("\n%d: %04X\t->\t", i++, instruction->op16);
        imprimir_binario16(instruction->op16);
        */

        if (instruction->op16 == inst16_table[e_NOP])
        {
            printf("NOP");
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
            printf("LDI\tR%s,\t%s", uint16_a_decimal(((instruction->type3.d4) + 0x10), cadena1), uint16_a_decimal((((instruction->type3.k4_h) << 4) | (instruction->type3.k4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_MOV])
        {
            printf("MOV\tR%s,\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_MUL])
        {
            printf("MUL\tR%s,\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_ST_Zi]) && (instruction->type1.op4 == 0x1))
        { /*
         Operation:                              Comment:
         (ii)    DS(Z) ← Rr, Z ← Z+1             Z: Post incremente

                 Syntax:     Operands:           Program Counter:
         (ii)    ST Z+,      Rr 0 ≤ r ≤ 31       PC ← PC + 1
                 16-bit Opcode :
         (ii)    1001 001r rrrr 0001
       */
            printf("ST\tZ+,\tR%s", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_ST_X]) && (instruction->type1.op4 == 0xC))
        { /*
            Operation:                  Comment:
            (i) DS(X) ← Rr              X: Unchanged

                Syntax:       Operands:           Program Counter:
            (i) ST X, Rr      0 ≤ r ≤ 31          PC ← PC + 1
            16-bit Opcode:
            (i) 1001 001r rrrr 1100
            */
            printf("ST\tX,\tR%s", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_LD_X]) && (instruction->type1.op4 == 0xC))
        { /*
            Operation:                  Comment:
            (i) Rd ← DS(X)              X: Unchanged

                Syntax:       Operands:           Program Counter:
            (i) LD Rd, X      0 ≤ d ≤ 31          PC ← PC + 1
            16-bit Opcode:
            (i) 1001 000d dddd 1100
            */
            printf("LD\tR%s,\tX", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_LD_Yi]) && (instruction->type1.op4 == 0x9))
        { /*
            Operation:                              Comment:
            (ii) Rd ← DS(Y), Y ← Y+1                Y: Post incremented

                 Syntax:       Operands:            Program Counter:
            (ii) LD Rd, Y+     0 ≤ d ≤ 31           PC ← PC + 1
            16-bit Opcode:
            (ii) 1001 000d dddd 1001
            */
            printf("LD\tR%s,\tY+", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_PUSH]) && (instruction->type1.op4 == 0xF))
        { /*
            Operation:                  Stack:
            (i) STACK ← Rr              SP ← SP - 1

                Syntax:        Operands:           Program Counter:
            (i) PUSH Rr        0 ≤ r ≤ 31          PC ← PC + 1
            16-bit Opcode:
            (i) 1001 001d dddd 1111
            */
            printf("PUSH\tR%s", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_POP]) && (instruction->type1.op4 == 0xF))
        { /*
            Operation:                  Stack:
            (i) Rd ← STACK              SP ← SP + 1

                Syntax:       Operands:            Program Counter:
            (i) POP Rd        0 ≤ d ≤ 31           PC ← PC + 1
            16-bit Opcode:
            (i) 1001 000d dddd 1111
            */
            printf("POP\tR%s", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_DEC]) && (instruction->type1.op4 == 0xA))
        { /*
            Operation:
            (i) Rd ← Rd - 1

                Syntax:     Operands:           Program Counter:
            (i) DEC Rd      0 ≤ d ≤ 31          PC ← PC + 1
            16-bit Opcode:
            (i) 1001 010d dddd 1010
            */
            printf("DEC\tR%s", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_INC]) && (instruction->type1.op4 == 0x3))
        { /*
            Operation:
            (i) Rd ← Rd + 1

                Syntax:     Operands:           Program Counter:
            (i) INC Rd      0 ≤ d ≤ 31          PC ← PC + 1
            16-bit Opcode:
            (i) 1001 010d dddd 0011
            */
            printf("INC\tR%s", uint16_a_decimal(instruction->type1.d5, cadena1));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_BST]))
        { /*
            Operation:
            (i) T ← Rd(b)

                Syntax:      Operands:               Program Counter:
            (i) BST Rd,b     0 ≤ d ≤ 31, 0 ≤ b ≤ 7    PC ← PC + 1
            16-bit Opcode:
            (i) 1111 101d dddd 0bbb
            */
            printf("BST\tR%s,\t%s", uint16_a_decimal(instruction->type1.d5, cadena1), uint16_a_decimal((instruction->type1.op4) & 0x7, cadena2));
        }
        else if (((instruction->type1.op7) << 9 == inst16_table[e_BLD]))
        { /*
            Operation:
            (i) Rd(b) ← T

                Syntax:      Operands:               Program Counter:
            (i) BLD Rd,b     0 ≤ d ≤ 31, 0 ≤ b ≤ 7    PC ← PC + 1
            16-bit Opcode:
            (i) 1111 100d dddd 0bbb
            */
            printf("BLD\tR%s,\t%s", uint16_a_decimal(instruction->type1.d5, cadena1), uint16_a_decimal((instruction->type1.op4) & 0x7, cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_EOR] && instruction->type2.d5 == (((instruction->type2.r1) << 4) | (instruction->type2.r4)))
        { /*
            Operation:                              Comment:
            (i)     Rd ← Rd ⊕ Rd

            Syntax:     Operands:           Program Counter:
            (i)     CLR Rd      0 ≤ d ≤ 31          PC ← PC + 1
            16-bit Opcode :
            (i)     0010 01dd dddd dddd
            NOTA: CLR es un alias de EOR Rd,Rd (Rd=Rr). En vez de intentar
            comparar el word completo (como lo hacía yo antes), primero
            reconozco el patrón de EOR por sus 6 bits fijos y luego reviso
            si el registro destino (d5) y el registro fuente (r1:r4)
            coinciden: solo entonces es CLR, si no, es un EOR normal.
            */
            printf("CLR\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1));
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
            printf("EOR\tR%s,\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_AND] && instruction->type2.d5 == (((instruction->type2.r1) << 4) | (instruction->type2.r4)))
        { /*
            Operation:                              Comment:
            (i)     Rd ← Rd ∧ Rd

                    Syntax:     Operands:           Program Counter:
            (i)     TST Rd      0 ≤ d ≤ 31          PC ← PC + 1
            16-bit Opcode (alias de AND Rd,Rd):
            (i)     0010 00dd dddd dddd
            */
            printf("TST\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_AND])
        { /*
            Operation:                                     Comment:
            (i)     Rd ← Rd ∧ Rr

                    Syntax:     Operands:               Program Counter:
            (i)     AND Rd,Rr   0 ≤ d ≤ 31, 0 ≤ r ≤ 31  PC ← PC + 1
            16-bit Opcode:
            (i)     0010 00rd dddd rrrr
            */
            printf("AND\tR%s,\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_OR])
        { /*
            Operation:                                     Comment:
            (i)     Rd ← Rd ∨ Rr

                    Syntax:     Operands:               Program Counter:
            (i)     OR Rd,Rr    0 ≤ d ≤ 31, 0 ≤ r ≤ 31  PC ← PC + 1
            16-bit Opcode:
            (i)     0010 10rd dddd rrrr
            */
            printf("OR\tR%s,\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_ADD] && instruction->type2.d5 == (((instruction->type2.r1) << 4) | (instruction->type2.r4)))
        { /*
            Operation:                              Comment:
            (i)     Rd ← Rd + Rd

                    Syntax:     Operands:           Program Counter:
            (i)     LSL Rd      0 ≤ d ≤ 31          PC ← PC + 1
            16-bit Opcode (alias de ADD Rd,Rd):
            (i)     0000 11dd dddd dddd
            */
            printf("LSL\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_ADD])
        { /*
            Operation:                                     Comment:
            (i)     Rd ← Rd + Rr

                    Syntax:     Operands:               Program Counter:
            (i)     ADD Rd,Rr   0 ≤ d ≤ 31, 0 ≤ r ≤ 31  PC ← PC + 1
            16-bit Opcode:
            (i)     0000 11rd dddd rrrr
            */
            printf("ADD\tR%s,\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if ((instruction->type2.op6) << 10 == inst16_table[e_CP])
        { /*
            Description
            This instruction performs a compare between two registers Rd and Rr. None of the registers are changed.
            Operation:
            (i) Rd - Rr
            Syntax:     Operands:               Program Counter:
            (i) CP Rd,Rr 0 ≤ d ≤ 31, 0 ≤ r ≤ 31  PC ← PC + 1
            16-bit Opcode:
            0001 01rd dddd rrrr
            */
            printf("CP\tR%s,\tR%s", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
        }
        else if ((instruction->type3.op4) << 12 == inst16_table[e_ANDI])
        { /*
            Operation:                                      Comment:
            (i)     Rd ← Rd ∧ K

                    Syntax:     Operands:                   Program Counter:
            (i)     ANDI Rd,K   16 ≤ d ≤ 31, 0 ≤ K ≤ 255    PC ← PC + 1
            16-bit Opcode :
            (i)     0111 KKKK dddd KKKK
            */
            printf("ANDI\tR%s,\t%s", uint16_a_decimal(((instruction->type3.d4) + 0x10), cadena1), uint16_a_decimal((((instruction->type3.k4_h) << 4) | (instruction->type3.k4)), cadena2));
        }
        else if ((instruction->type3.op4) << 12 == inst16_table[e_CPI])
        { /*
            Description
            This instruction performs a compare between register Rd and a constant. The register is not changed.
            Operation:
            (i) Rd - K
            Syntax:       Operands:                   Program Counter:
            (i) CPI Rd,K  16 ≤ d ≤ 31, 0 ≤ K ≤ 255     PC ← PC + 1
            16-bit Opcode:
            0011 KKKK dddd KKKK
            */
            printf("CPI\tR%s,\t%s", uint16_a_decimal(((instruction->type3.d4) + 0x10), cadena1), uint16_a_decimal((((instruction->type3.k4_h) << 4) | (instruction->type3.k4)), cadena2));
        }
        else if ((instruction->type3.op4) << 12 == inst16_table[e_SUBI])
        { /*
            Description
            Subtracts a register and a constant, and places the result in the destination register Rd.
            Operation:
            (i) Rd ← Rd - K
            Syntax:        Operands:                   Program Counter:
            (i) SUBI Rd,K  16 ≤ d ≤ 31, 0 ≤ K ≤ 255     PC ← PC + 1
            16-bit Opcode:
            0101 KKKK dddd KKKK
            */
            printf("SUBI\tR%s,\t%s", uint16_a_decimal(((instruction->type3.d4) + 0x10), cadena1), uint16_a_decimal((((instruction->type3.k4_h) << 4) | (instruction->type3.k4)), cadena2));
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
            printf("RET");
        }
        else if (((instruction->op16) & 0xF000) == inst16_table[e_RCALL])
        { /*
            Description
            Relative call to an address within PC - 2K + 1 and PC + 2K (words). The return address (the instruction
            after the RCALL) is stored onto the Stack.
            Operation:
            (i) PC ← PC + k + 1
            Syntax:  Operands:        Program Counter:
            (i) RCALL k  -2K ≤ k < 2K  PC ← PC + k + 1
            16-bit Opcode:
            1101 kkkk kkkk kkkk
            */
            int16_t k = extender_signo(instruction->op16 & 0x0FFF, 12);
            /* printf("RCALL\t%d\t(palabra %d)", k, pc + 1 + k); */
            printf("RCALL\t%d", k + 1);
        }
        else if (((instruction->op16) & 0xF000) == inst16_table[e_RJMP])
        { /*
            Description
            Relative jump to an address within PC - 2K + 1 and PC + 2K (words).
            Operation:
            (i) PC ← PC + k + 1
            Syntax: Operands:        Program Counter:
            (i) RJMP k  -2K ≤ k < 2K  PC ← PC + k + 1
            16-bit Opcode:
            1100 kkkk kkkk kkkk
            */
            int16_t k = extender_signo(instruction->op16 & 0x0FFF, 12);
            /* printf("RJMP\t%d\t(palabra %d)", k, pc + 1 + k); */
            printf("RJMP\t%d", k + 1);
        }
        else if ((instruction->type4.op6) << 10 == inst16_table[e_BRBS])
        { /*
            Description
            Conditional relative branch. Tests a single bit in SREG (s) y salta si esa bandera esta en 1.
            Operation:
            (i) If SREG(s) == 1 then PC ← PC + k + 1, else PC ← PC + 1
            Syntax:      Operands:                   Program Counter:
            (i) BRBS s,k 0 ≤ s ≤ 7, -64 ≤ k ≤ +63     PC ← PC + k + 1
            16-bit Opcode:
            1111 00kk kkkk ksss

            NOTA: aqui esta el otro error grande que tenia: comparaba
            (instruction->op16) contra un valor fijo de la tabla (por
            ejemplo 0xF001 para BREQ), lo cual solo funciona si k vale 0.
            Lo correcto es comparar unicamente los 6 bits fijos del opcode
            (type4.op6) y despues leer aparte el offset k (con signo) y la
            bandera s para saber cual BRxx especifico es.
            */
            int16_t k = extender_signo(instruction->type4.k, 7);
            uint8_t s = instruction->type4.op3;
            /* printf("%s\t%d\t(palabra %d, bandera %s = 1)", brbs_alias[s], k, pc + 1 + k, sreg_bits[s]); */
            printf("%s\t%d", brbs_alias[s], k + 1);
        }
        else if ((instruction->type4.op6) << 10 == inst16_table[e_BRBC])
        { /*
            Description
            Conditional relative branch. Tests a single bit in SREG (s) y salta si esa bandera esta en 0.
            Operation:
            (i) If SREG(s) == 0 then PC ← PC + k + 1, else PC ← PC + 1
            Syntax:      Operands:                   Program Counter:
            (i) BRBC s,k 0 ≤ s ≤ 7, -64 ≤ k ≤ +63     PC ← PC + k + 1
            16-bit Opcode:
            1111 01kk kkkk ksss
            */
            int16_t k = extender_signo(instruction->type4.k, 7);
            uint8_t s = instruction->type4.op3;
            /* printf("%s\t%d\t(palabra %d, bandera %s = 0)", brbc_alias[s], k, pc + 1 + k, sreg_bits[s]); */
            printf("%s\t%d", brbc_alias[s], k + 1);
        }
        else
            printf("\nunknown\n");
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
        temp[pos++] = (char)((valor % 10) + '0'); // saca el dígito menos significativo
        valor /= 10;
    }

    // los dígitos salieron al revés (unidades primero), hay que invertirlos
    for (int i = 0; i < pos; i++)
        buffer[i] = temp[pos - 1 - i];
    buffer[pos] = '\0';

    return buffer;
}