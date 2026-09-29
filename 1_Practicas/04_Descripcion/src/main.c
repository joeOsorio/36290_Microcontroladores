/*
 * U.A.B.C. - Facultad de Ciencias Quimicas e Ingenieria
 * 36290 Microprocesadores y Microcontroladores
 * Practica 4 - Manejo de la seccion de E/S del ATmega1280/2560
 * Alumno: Joshua Osorio O. - 1293271
 *
 * Descripcion:
 *   Revisa el estado de los botones conectados en
 *   PK0 (incremento)
 *   PK1 (decremento)
 *   Segun la duracion de la pulsacion
 *   (corta o larga, umbral > 1 s) actualiza un contador que se despliega en el
 *   display de 7 segmentos conectado al Puerto L.
 *
 * Notas de plataforma:
 *   - Los botones se configuran con PULL-UP interno (DDRx=0, PORTx=1):
 *     en reposo el pin lee 1, al presionar el boton se conecta a GND
 *     y el pin lee 0.
 *   - SetBitPort/ClrBitPort estan implementadas como funciones reales en
 *     ensamblador (MyRutines.asm), que acceden al registro con ld/st a
 *     traves de un puntero (registro Z). A diferencia de una macro basada
 *     en SBI/CBI (limitada a E/S 0x00-0x1F), estas funciones si alcanzan
 *     PORTK y PORTL, que en el ATmega2560 son E/S EXTENDIDA (direccion de
 *     memoria >= 0x100).
 *   - delay() tambien esta implementada en ensamblador puro en el archivo
 *     externo MyRutines.asm, calibrada a F_CPU = 16 MHz. main.c y
 *     MyRutines.asm se compilan y enlazan juntos (ver src/compilar.sh).
 *   - Igual que en el codigo del profesor, check_Btn() no solo indica
 *     si hubo pulsacion: su valor de retorno (0/1/5) se suma o resta
 *     directamente al contador ("count += check_Btn(...)"), por lo
 *     que una pulsacion larga incrementa/decrementa de 5 en 5.
 */

#include <avr/io.h>

/* Asignacion de pines segun el diagrama */
#define INC_BUTTON PK0 /* Boton de incremento -> PK0 */
#define DEC_BUTTON PK1 /* Boton de decremento -> PK1 */
#define BUTTON_DDR DDRK
#define BUTTON_PORT PORTK
#define BUTTON_PIN PINK

#define DISPLAY_DDR DDRL
#define DISPLAY_PORT PORTL /* Display de 7 segmentos -> Puerto L  */

/* Parametros de tiempo */
#define DEBOUNCE_MS 20U     /* Ventana de anti-rebote */
#define LONG_PRESS_MS 1000U /* Umbral de pulsacion larga: > 1 seg  */

typedef enum ButtonStates
{
    eBtnUndefined = 0,
    eBtnShortPressed = 1,
    eBtnLongPressed = 5
} Button_State_t;

/* Prototipos del 3 al 6 */
extern void delay(uint16_t mseg);                            /* Implementada en MyRutines.asm */
extern void SetBitPort(volatile uint8_t *port, uint8_t bit); /* Implementada en MyRutines.asm */
extern void ClrBitPort(volatile uint8_t *port, uint8_t bit); /* Implementada en MyRutines.asm */
void InitPorts(void);
void Display7Seg(uint8_t data);
Button_State_t check_Btn(volatile uint8_t *pin_register, uint8_t pin_number);

/*
 * Tabla de segmentos para digitos 0-F.
 * Bit7=A Bit6=B Bit5=C Bit4=D Bit3=E Bit2=F Bit1=G Bit0=DP
 * (orden fisico real:
 * D42=A,
 * D43=B,
 * D44=C,
 * D45=D,
 * D46=E,
 * D47=F,
 * D48=G,
 * D49=DP
 * -- ver diagram.json.
 * Es el orden "al derecho": el pin mas bajo (D42=PL7) a la letra A,
 * subiendo hasta D49=PL0=DP.
 * Activo en BAJO (display comun anodo, COM -> 5V): un segmento se
 * enciende con 0.
 */
static const uint8_t seg_table[16] =
    {
        0x03, /* 0 */
        0x9F, /* 1 */
        0x25, /* 2 */
        0x0D, /* 3 */
        0x99, /* 4 */
        0x49, /* 5 */
        0x41, /* 6 */
        0x1F, /* 7 */
        0x01, /* 8 */
        0x09, /* 9 */
        0x11, /* A */
        0xC1, /* b */
        0x63, /* C */
        0x85, /* d */
        0x61, /* E */
        0x71  /* F */
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

/* 4 Inicializacion de los puertos utilizados segun el diagrama */
void InitPorts(void)
{
    uint8_t i;

    /* Display de 7 segmentos en PORTL como salida, inicia apagado
     * (comun anodo: 1 = segmento apagado). Se arma bit a bit con
     * SetBitPort porque PORTL es E/S extendida. */
    for (i = 0; i < 8; i++)
    {
        SetBitPort(&DISPLAY_DDR, i);
        SetBitPort(&DISPLAY_PORT, i);
    }

    /* Botones en PK0/PK1 como entrada con PULL-UP interno */
    ClrBitPort(&BUTTON_DDR, INC_BUTTON); /* Definir como entrada */
    ClrBitPort(&BUTTON_DDR, DEC_BUTTON);
    SetBitPort(&BUTTON_PORT, INC_BUTTON); /* Habilita pull-up */
    SetBitPort(&BUTTON_PORT, DEC_BUTTON);
}

/* (5) Despliega "data" (4 bits) sobre el display de 7 segmentos */
void Display7Seg(uint8_t data)
{
    DISPLAY_PORT = seg_table[data & 0x0F]; /* aseguro que los bits altos no tengan basura*/
}

/*
 * 6 Estado del boton en pin_register:pin_number, distinguiendo
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
