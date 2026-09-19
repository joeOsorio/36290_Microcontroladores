;=============================================================================
; MyRutines.asm
; U.A.B.C. - Facultad de Ciencias Quimicas e Ingenieria
; 36290 Microprocesadores y Microcontroladores - Practica 4
; Alumno: Joshua Osorio O. - 1293271
;
; Rutina de retardo por software pedida en la practica:
;   "void delay(uint16_t mseg); Funcion que debe tardarse n ms en
;   retornar ... la cual debera estar implementada en ensamblador y
;   ser llamada desde C. Se pide una exactitud de +-5 us."
;
; Se ensambla junto con main.c (ver src/compilar.sh) y se enlaza como
; una funcion global normal, igual que declara el codigo base del
; profesor:  extern void delay(uint16_t mseg); // To be defined in ASM
;=============================================================================

#include <avr/io.h>

    .section .text
    .global delay

;-----------------------------------------------------------------------------
; void delay(uint16_t mseg)
;
; Convencion de llamada de avr-gcc: el argumento de 16 bits llega en el
; par de registros r25:r24 (r24 = byte bajo, r25 = byte alto). No se usan
; registros "call-saved" (r2-r17, r28-r29, etc.), por lo que no hace
; falta push/pop.
;
; Cada vuelta del lazo externo (delay_lazo_ms) tarda EXACTAMENTE 16000
; ciclos de reloj. El lazo interno usa el par Z (r31:r30) porque sbiw
; solo admite los pares r25:r24, r27:r26, r29:r28 o r31:r30, y r25:r24
; ya esta ocupado por el argumento mseg.
;
;   ldi r30,K_bajo      1 ciclo
;   ldi r31,K_alto      1 ciclo
;   lazo interno (N=3998 vueltas de sbiw+brne):
;       (N-1) vueltas x (sbiw=2 + brne tomado=2)   = 4*(N-1) ciclos
;       1 vuelta final   (sbiw=2 + brne no tomado=1) = 3 ciclos
;                                   => 4*N - 1 = 15991 ciclos
;   nop x3              3 ciclos
;   sbiw r24,1          2 ciclos
;   brne delay_lazo_ms  2 ciclos
;   -----------------------------------------------
;   Total = 1+1+15991+3+2+2 = 16000 ciclos
;
; A F_CPU = 16 MHz: 16000 ciclos / 16 000 000 Hz = 1.000000 ms exactos
; (unicamente la ultima vuelta del lazo externo, donde el brne final no
; se toma, ahorra 1 ciclo = 62.5 ns; muy por debajo de los +-5 us
; pedidos).
;-----------------------------------------------------------------------------
delay:
        ; Si mseg == 0 no hay nada que esperar (evita que r25:r24
        ; se desborde a 0xFFFF si se decrementara desde 0)
        sbiw    r24, 0
        breq    delay_fin

delay_lazo_ms:
        ldi     r30, 0x9E           ; cuenta interna = 3998 (0x0F9E)
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
