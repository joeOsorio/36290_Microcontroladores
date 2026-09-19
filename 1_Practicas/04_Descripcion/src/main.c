/*
 * U.A.B.C. - Facultad de Ciencias Quimicas e Ingenieria
 * 36290 Microprocesadores y Microcontroladores
 * Practica 4 - Manejo de la seccion de E/S del ATmega1280/2560
 * Alumno: Joshua Osorio O. - 1293271
 *
 * Descripcion:
 *   Revisa el estado de los botones conectados en PK0 (incremento) y
 *   PK1 (decremento). Segun la duracion de la pulsacion (corta o
 *   larga, umbral > 1 s) actualiza un contador que se despliega en el
 *   display de 7 segmentos conectado al Puerto L.
 *
 * Notas de plataforma:
 *   - Los botones se configuran con PULL-UP interno (DDRx=0, PORTx=1):
 *     en reposo el pin lee 1, al presionar el boton se conecta a GND
 *     y el pin lee 0.
 *   - PORTK y PORTL son E/S EXTENDIDA en el ATmega2560 (direccion de
 *     memoria >= 0x100), por lo que quedan fuera del rango de las
 *     instrucciones SBI/CBI (solo direccionan E/S 0x00-0x1F). Por eso
 *     InitPorts() los configura con lectura-modificacion-escritura
 *     normal (&=, |=) en vez de las macros SetBitPort/ClrBitPort.
 *   - delay() esta implementada en ensamblador puro en el archivo
 *     externo MyRutines.asm (igual que en el codigo base del
 *     profesor: "extern void delay(uint16_t mseg); // To be defined
 *     in ASM"), calibrada a F_CPU = 16 MHz. main.c y MyRutines.asm se
 *     compilan y enlazan juntos (ver src/compilar.sh).
 *   - Igual que en el codigo del profesor, check_Btn() no solo indica
 *     si hubo pulsacion: su valor de retorno (0/1/5) se suma o resta
 *     directamente al contador ("count += check_Btn(...)"), por lo
 *     que una pulsacion larga incrementa/decrementa de 5 en 5.
 */

#include <avr/io.h>

/* ---------------------------------------------------------------------
 * (1)/(2) Macros requeridas: acceso a nivel de bit mediante SBI/CBI en
 * ensamblador en linea. Validas unicamente para registros de E/S
 * "clasica" (direccion < 0x20 de E/S, p. ej. PORTA..PORTG): el
 * ensamblador rechaza en tiempo de compilacion cualquier registro de
 * E/S extendida (PORTH/J/K/L) porque no cabe en el operando "I" de
 * SBI/CBI.
 * ------------------------------------------------------------------- */
#define SetBitPort(port, bit) \
    __asm__ __volatile__ ("sbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))

#define ClrBitPort(port, bit) \
    __asm__ __volatile__ ("cbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))

/* Asignacion de pines segun el diagrama (diagram.json) */
#define INC_BUTTON    PK0      /* Boton de incremento -> PK0 */
#define DEC_BUTTON    PK1      /* Boton de decremento -> PK1 */
#define BUTTON_DDR    DDRK
#define BUTTON_PORT   PORTK
#define BUTTON_PIN    PINK

#define DISPLAY_DDR   DDRL
#define DISPLAY_PORT  PORTL    /* Display de 7 segmentos -> Puerto L  */

/* Parametros de tiempo */
#define DEBOUNCE_MS    20U     /* Ventana de anti-rebote              */
#define LONG_PRESS_MS  1000U   /* Umbral de pulsacion larga: > 1 seg  */

typedef enum ButtonStates
{
    eBtnUndefined    = 0,
    eBtnShortPressed = 1,
    eBtnLongPressed  = 5
} Button_State_t;

/* Prototipos (3)-(6) */
extern void delay(uint16_t mseg); /* Implementada en MyRutines.asm */
void InitPorts(void);
void Display7Seg(uint8_t data);
Button_State_t check_Btn(volatile uint8_t *pin_register, uint8_t pin_number);

/*
 * Tabla de segmentos para digitos 0-F.
 * Bit0=A Bit1=B Bit2=C Bit3=D Bit4=E Bit5=F Bit6=G Bit7=DP
 * Activo en BAJO (display comun anodo, COM -> 5V, ver diagram.json).
 */
static const uint8_t seg_table[16] =
{
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E
};

int main(void)
{
    uint8_t count = 0;

    UCSR0B &= (uint8_t)~(1 << TXEN0); /* Deshabilita TX de UART0 */

    InitPorts();

    while (1)
    {
        /* El valor de retorno (0/1/5) se suma/resta directamente:
         * sin pulsar = +0, pulsacion corta = +-1, pulsacion larga = +-5 */
        count += check_Btn(&BUTTON_PIN, INC_BUTTON);
        count -= check_Btn(&BUTTON_PIN, DEC_BUTTON);

        Display7Seg(count & 0x0F);
    }
}

/* (4) Inicializacion de los puertos utilizados segun el diagrama */
void InitPorts(void)
{
    /* Display de 7 segmentos en PORTL como salida, inicia apagado */
    DISPLAY_DDR = 0xFF;
    DISPLAY_PORT = 0xFF; /* Comun anodo: 1 = segmento apagado */

    /* Botones en PK0/PK1 como entrada con PULL-UP interno */
    BUTTON_DDR &= (uint8_t)~((1 << INC_BUTTON) | (1 << DEC_BUTTON));
    BUTTON_PORT |= (uint8_t)((1 << INC_BUTTON) | (1 << DEC_BUTTON));
}

/* (5) Despliega "data" (4 bits) sobre el display de 7 segmentos */
void Display7Seg(uint8_t data)
{
    DISPLAY_PORT = seg_table[data & 0x0F];
}

/*
 * (6) Estado del boton en pin_register:pin_number, distinguiendo
 * pulsacion corta / larga (> 1 s), con anti-rebote a la entrada y a
 * la salida de la pulsacion.
 */
Button_State_t check_Btn(volatile uint8_t *pin_register, uint8_t pin_number)
{
    uint16_t held_ms = 0;

    /* Reposo (pull-up): nivel alto = boton libre */
    if ((*pin_register & (1 << pin_number)) != 0)
    {
        return eBtnUndefined;
    }

    /* Anti-rebote de flanco de bajada: confirma que el nivel bajo persiste */
    delay(DEBOUNCE_MS);
    if ((*pin_register & (1 << pin_number)) != 0)
    {
        return eBtnUndefined; /* fue rebote, no una pulsacion real */
    }

    /* Mide cuanto tiempo permanece presionado */
    while ((*pin_register & (1 << pin_number)) == 0)
    {
        delay(1);
        held_ms++;
    }

    /* Anti-rebote de flanco de subida (al soltar) */
    delay(DEBOUNCE_MS);

    return (held_ms > LONG_PRESS_MS) ? eBtnLongPressed : eBtnShortPressed;
}

/* (3) void delay(uint16_t mseg): ver MyRutines.asm */
