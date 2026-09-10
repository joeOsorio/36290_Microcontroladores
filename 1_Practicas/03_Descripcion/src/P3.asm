    /*
    *       Practica3: cracion de pausas.
    *       Created: 05/09/2026 07:03:27 p. m.
    *       Description:
    *       Basándose en la teoría sobre retardos por software y realice los
    *       cambios necesarios para
    *       incluir un determinado retardo. Los retardos por software a implementar son:
    *       a) 52us
    *       b) 16ms
    *       c) 1s
    *
    *       Adicionalmente implementar un procedimiento que genere un número pseudo-aleatorio de
    *       8 bits.
    */

    Estare  utilizando vscode para realizar codigo de asm, pero como necesito estar realizando algunas pruebas necesitare



    ; Se utiliza el simulador de avr y se ejecuta eldebuger
    call    delay
    ; break
    nop

delay:
    ldi     R24, 195
nxt:
    nop
    nop
    nop
    ldi     R25, 15
nxt2:
    nop
    nop
    nop
    dec     R25
    brne    nxt2
    dec     R24
    brne    nxt
    ret
