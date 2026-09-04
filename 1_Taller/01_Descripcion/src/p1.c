#include <stdio.h>
#include <inttypes.h>

typedef struct
{
	uint16_t bl;
	uint16_t bh;
} campos_bits;

int main(void)
{
	uint16_t a = 0;
	uint32_t b = 0x1155;

	// printf("\n1: %X", a);

	/*
		OBJETIVO --> B0B0
		a = 0000 --> 0000 0000 0000 0000	( valor que se tiene )
			B0B0 --> 1011 0000 1011 0000	( valor que se busca )

			Debido a que UNICAMENTE para este caso se busca habilitar bits, se usa OR

			0000 --> 0000 0000 0000 0000	( valor que se tiene )
		OR	mask --> 1011 0000 1011 0000	( valor que se busca )
		--------------------------------
					 1011 0000 1011 0000
	*/
	a |= (1 << 15) | (3 << 12) | (1 << 7) | (3 << 4);
	printf("\n1: %X", a);

	// BABA --> 1011 1010 1011 1010

	a |= (1 << 15) | (3 << 12) | (1 << 11) | (1 << 9) | (1 << 7) | (3 << 4) | (1 << 3) | (1 << 1);
	//           B					 A				   B				 A
	printf("\n2: %X", a);

	// BEBE
	// tengo --> 1011 1010 1011 1010
	// busco --> 1011 1110 1011 1110

	a |= (1 << 10) | (1 << 2);
	printf("\n3: %X", a);

	// FE00
	a |= (1 << 14) | (1 << 10);
	// si quiero apagar los bits del 7 al 0 se usa AND y se niega la mascara
	// 0000 0000 1111 1111
	a &= ~((3 << 6) | (3 << 4) | (3 << 2) | (3 << 0));
	printf("\n4: %X", a);

	// FEAA
	// tengo --> 1111 1110 0000 0000
	// busco --> 1111 1110 1010 1010	busco encender bits

	a |= (1 << 7) | (1 << 5) | (1 << 3) | (1 << 1);
	printf("\n5: %X", a);

	// 0155
	// tengo --> 1111 1110 1010 1010
	// busco --> 0000 0001 0101 0101 directamente se puede negar todo para que se invierta

	a = ~a;
	printf("\n6: %X", a);

	// 0001
	// tengo --> 0000 0001 0101 0101
	// busco --> 0000 0000 0000 0001

	a &= (1 << 0);
	printf("\n7: %X", a);

	// 0100
	// tengo --> 0000 0000 0000 0001
	// busco --> 0000 0001 0000 0000

	a ^= (1 << 8) | (1 << 0);
	printf("\n8: %X", a);

	// APUNTADORES
	printf("\n** Apuntadores **");
	printf("\nValor de b: %X", b);
	b = ((~b) << 16) | b;
	printf("\nValor de b 32b: %X", b);
	uint16_t *ptr16 = (uint16_t *)&b;
	printf("\nValor de b 16b: %X", *ptr16);
	uint8_t *ptr8 = (uint8_t *)&b;
	printf("\nValor de b 8b: %X", *ptr8);

	// DEADBEEF
	// tenemos 		 --> 1110 1110 1010 1010
	// buscamos DEAD --> 1101 1110 1010 1101
	// aplicamos xor --> 0011 0001 1010 0111
	*(ptr16 + 1) ^= (3 << 12) | 7;
	//  			 D		D

	// tenemos		 --> 0001 0001 0101 0101
	// buscamos BEEF ->> 1011 1110 1110 1111
	// aplicamos xor	 1010 1111 1011 1010
	*ptr16 ^= (10 << 12) | (15 << 8) | (11 << 4) | 10;
	printf("\nValor de b 32b: %X", b);

	// ESTRUCTURA
	campos_bits *ptr = (campos_bits *)&b;
	printf("\n10. ESTRUCTURA");
	printf("\nValor de b 32b: %X", b);

	// tenemos  DEADBEEF --> 1101 1110 1010 1101 1011 1110 1110 1111
	// buscamos BADDCODE --> 1011 1010 1101 1101 1100 0000 1101 1110
	// aplicamos xor	 --> 0110 0100 0111 0000 0111 1110 0011 0001
	//						|		  H		    |         L			|

	// necesitamos ingresar a los valores de la estructura
	// parte alta de b
	ptr->bh ^= (6 << 12) | (4 << 8) | (7 << 4) | 0;
	// parte baja de b
	ptr->bl ^= (7 << 12) | (14 << 8) | (3 << 4) | 1;

	printf("\nValor de b 32b: %X", b);

	return 0;
}