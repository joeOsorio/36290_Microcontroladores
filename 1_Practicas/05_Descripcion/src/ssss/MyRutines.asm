/*
 * MyRutines.asm - Practica 5
 * delay(uint16_t mseg): retardo por software calibrado a F_CPU = 16 MHz
 * (misma rutina de la Practica 4). mseg llega en r25:r24 (ABI de avr-gcc).
 */
#include <avr/io.h>

    .section .text
    .global delay

delay:
        sbiw    r24, 0
        breq    delay_fin

delay_lazo_ms:
        ldi     r30, 0x9E
        ldi     r31, 0x0F

delay_lazo_us:
        sbiw    r30, 1
        brne    delay_lazo_us

        nop
        nop
        nop

        sbiw    r24, 1              ; mseg--
        brne    delay_lazo_ms

delay_fin:
        ret
