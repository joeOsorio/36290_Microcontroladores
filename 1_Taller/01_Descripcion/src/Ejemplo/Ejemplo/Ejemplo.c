/*
 * Ejemplo.c
 *
 * Created: 21/08/2026 08:30:57 p. m.
 *  Author: Okuyt
 */

#include <avr/io.h>

// Funcion de retardo por software (espera activa) para poder apreciar
// el parpadeo del LED a simple vista.
void delay(unsigned long ciclos)
{
    while (ciclos--)
    {
        __asm__ __volatile__("nop");
    }
}

int main(void)
{
    // PB7 como salida -> LED "L" integrado en la tarjeta (Arduino Mega 2560,
    // pin digital 13). El 1 se coloca en el bit 7 mediante un corrimiento.
    DDRB |= (1 << PB7);

    while(1)
    {
        // Corrimiento: (1 << PB7) desplaza el bit 1 hasta la posicion 7.
        // El XOR contra PORTB invierte ese bit en cada vuelta -> parpadeo.
        PORTB ^= (1 << PB7);

        delay(200000);
    }
}
