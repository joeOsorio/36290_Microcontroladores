/******************************************************************************
Prac 2 - AVR ASM OpCode Decoder
*******************************************************************************/

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h> /* Para comvertir a cadena decimal*/

void imprimir_binario16(uint16_t valor);
char *uint16_a_decimal(uint16_t valor, char *buffer);

const uint8_t flash_mem[] = {
    0xf2, 0xe0, 0xdf, 0x2f, 0x08, 0xe1, 0x01, 0x93, 0x03, 0xe1, 0x01, 0x93, 0x05, 0xe1, 0x01, 0x93, 0x08, 0xe0,
    0x01, 0x93, 0x03, 0xe0, 0x01, 0x93, 0x09, 0xe0, 0x01, 0x93, 0xa5, 0xe0, 0x5a, 0x2e, 0xa4, 0xe1, 0x22, 0x27,
    0x09, 0x91, 0x0f, 0x71, 0x10, 0xe0, 0x22, 0x23, 0x11, 0xf0, 0x0e, 0x94, 0x2a, 0x00, 0x3c, 0x91, 0x30, 0x2b,
    0x3c, 0x93, 0x24, 0x30, 0x24, 0xf0, 0xa3, 0x95, 0x3c, 0x91, 0x31, 0x2b, 0x3c, 0x93, 0x25, 0x0d, 0x28, 0x30,
    0x0c, 0xf0, 0x28, 0x50, 0xec, 0x17, 0x51, 0xf7, 0x00, 0x00, 0xfe, 0xcf, 0x2f, 0x93, 0x07, 0xfb, 0x00, 0x0f,
    0x11, 0x0f, 0x10, 0xf9, 0x2a, 0x95, 0xd1, 0xf7, 0x2f, 0x91, 0x08, 0x95};

const uint16_t inst16_table[] = {
    0x0000, /* NOP */
    0x2C00, /* MOV */
    0xE000, /* LDI */
    0xFFFF  /* UABC */
};
/* Para Op code de 7 bits 0001 11rd dddd rrrr*/

enum
{
    e_NOP,
    e_MOV,
    e_LDI,
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
} Op_Code_t;

int main()
{
    int i = 1;
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
            printf("NOP\n");
        }
        else
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
                char cadena1[6], cadena2[6];
                printf("MOV\tR%s,\tR%s\n\n", uint16_a_decimal(instruction->type2.d5, cadena1), uint16_a_decimal((((instruction->type2.r1) << 4) | (instruction->type2.r4)), cadena2));
                break;
            /* case 0xE000: */
            case inst16_table[e_LDI]:
                printf("hola\n\n\n\n\n\n");

                break;

            default:
                printf("\nunknown\n");
                break;
            }
        }
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