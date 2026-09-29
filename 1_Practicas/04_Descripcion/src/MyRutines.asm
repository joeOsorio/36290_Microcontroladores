#include <avr/io.h>

    .section .text
    .global delay
    .global SetBitPort
    .global ClrBitPort

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

SetBitPort:
    movw    r30, r24        ; Z = port
    ld      r18, Z          ; r18 = *port
    ldi     r19, 1
SetBitPort_mask:
    cpi     r22, 0
    breq    SetBitPort_apply
    lsl     r19
    dec     r22
    rjmp    SetBitPort_mask
SetBitPort_apply:
    or      r18, r19
    st      Z, r18
    ret

ClrBitPort:
    movw    r30, r24        ; Z = port
    ld      r18, Z          ; r18 = *port
    ldi     r19, 1
ClrBitPort_mask:
    cpi     r22, 0
    breq    ClrBitPort_apply
    lsl     r19
    dec     r22
    rjmp    ClrBitPort_mask
ClrBitPort_apply:
    com     r19             ; ~(1<<bit)
    and     r18, r19
    st      Z, r18
    ret