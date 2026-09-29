/*
 * U.A.B.C. - Facultad de Ciencias Quimicas e Ingenieria
 * 36290 Microprocesadores y Microcontroladores
 * Practica 5 - Manejo de la seccion de E/S del ATmega1280/2560 (II)
 *              Juego del "gato" (tic tac toe) con Charlieplexing
 * Alumno: Joshua Osorio O. - 1293271
 *
 * Hardware (ver diagram.json):
 *   - 18 LEDs (9 casillas bicolor: rojo + verde en antiparalelo)
 *     manejados por Charlieplexing con solo 5 lineas:
 *         L0 = PA1 (D23)   L1 = PA5 (D27)   L2 = PC6 (D31)
 *         L3 = PC4 (D33)   L4 = PC0 (D37)
 *     Cada linea lleva su resistencia serie de 100 ohm, asi cada LED
 *     encendido ve 2 resistencias (200 ohm) -> ~12 mA pico.
 *   - Un boton en PB2 (D51) a GND, con pull-up interno.
 *
 * Controles:
 *   - Pulsacion corta : mover cursor a la siguiente casilla libre.
 *   - Doble pulsacion : mover cursor a la casilla libre anterior.
 *   - Pulsacion larga : colocar la ficha del jugador en turno.
 *
 * Todo el programa es NO bloqueante: cada vuelta del lazo principal dura
 * ~1 ms (delay(1)) y en cada vuelta se enciende UN solo LED (barrido de
 * Charlieplexing), se muestrea el boton y se avanza el contador millis.
 */

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

/* ---------------------------------------------------------------- */
/* Lineas de Charlieplexing                                         */
/* ---------------------------------------------------------------- */
#define LED_LINE0 PA1 /* D23 */
#define LED_LINE1 PA5 /* D27 */
#define LED_LINE2 PC6 /* D31 */
#define LED_LINE3 PC4 /* D33 */
#define LED_LINE4 PC0 /* D37 */

#define LINES_PORTA_MASK ((1 << LED_LINE0) | (1 << LED_LINE1))
#define LINES_PORTC_MASK ((1 << LED_LINE2) | (1 << LED_LINE3) | (1 << LED_LINE4))

#define BTN_GPIO PB2 /* D51 */

#define NUM_LED_PER_COLOR 9
#define NUM_LINES 5

/* Tiempos (ms) */
#define DEBOUNCE_MS 20U
#define LONG_PRESS_MS 1000U
#define DOUBLE_PRESS_MS 500U
#define CURSOR_ON_MS 500U
#define CURSOR_OFF_MS 100U
#define SEQ_ON_MS 1000U
#define SEQ_OFF_MS 500U
#define SEQ_REPEAT 4U /* ciclos encendido/apagado de cada animacion */

/* ---------------------------------------------------------------- */
/* Tipos                                                            */
/* ---------------------------------------------------------------- */
typedef enum LedColor_tag
{
    eRedLed = 0,
    eGreenLed = 1,
    eNumOfColors = 2
} eLedColor_t;

typedef enum ButtonState_tag
{
    eBtnUndefined = 0,
    eBtnShortKeyPress,
    eBtnDoubleKeyPress,
    eBtnLongKeyPress
} eButtonState_t;

typedef enum GameState_tag
{
    eGameRestart = 0,
    eOngoingGame,
    eStalemate,
    eRedPlayerWin,
    eGreenPlayerWin
} eGameState_t;

typedef struct BoardState_tag
{
    bool gameBoard[eNumOfColors][NUM_LED_PER_COLOR];
    uint8_t cursor;
    eLedColor_t currentColor;
} sBoardState_t;

/* Descripcion de una linea: registro DDR, registro PORT y bit */
typedef struct
{
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    uint8_t bit;
} sLine_t;

/* ---------------------------------------------------------------- */
/* Variables globales                                               */
/* ---------------------------------------------------------------- */
uint32_t millis;

static const sLine_t lines[NUM_LINES] = {
    {&DDRA, &PORTA, LED_LINE0},
    {&DDRA, &PORTA, LED_LINE1},
    {&DDRC, &PORTC, LED_LINE2},
    {&DDRC, &PORTC, LED_LINE3},
    {&DDRC, &PORTC, LED_LINE4}};

/*
 * Mapa de Charlieplexing: {linea del anodo, linea del catodo} del LED
 * ROJO de cada casilla (0..8, de izquierda a derecha y de arriba abajo).
 * El LED VERDE de la misma casilla esta en antiparalelo, asi que usa las
 * mismas lineas pero invertidas.
 */
static const uint8_t redMap[NUM_LED_PER_COLOR][2] = {
    {0, 1}, {0, 2}, {0, 3}, /* fila 0 */
    {1, 2}, {1, 3}, {3, 4}, /* fila 1 */
    {2, 3}, {1, 4}, {0, 4}  /* fila 2 */
};

/* Las 8 combinaciones ganadoras */
static const uint8_t winLines[8][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, /* horizontales */
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, /* verticales */
    {0, 4, 8}, {2, 4, 6}             /* diagonales */
};

/* Prototipos */
extern void delay(uint16_t ms); /* MyRutines.asm */
static void initIO(void);
static void ledsOff(void);
static void ledOn(eLedColor_t color, uint8_t cell);
static void scanLeds(uint16_t redMask, uint16_t greenMask);
static bool isCellFree(const sBoardState_t *b, uint8_t cell);
static bool findFreeCell(const sBoardState_t *b, uint8_t from, int8_t step, uint8_t *out);
static bool hasWon(const sBoardState_t *b, eLedColor_t color);
eButtonState_t checkButton(void);
bool playSequence(eGameState_t gameState);
eGameState_t checkBoard(sBoardState_t *boardState, eButtonState_t buttonState);
void displayBoard(sBoardState_t *boardState);
int app_main(void);

int main(void)
{
    return app_main();
}

/* ---------------------------------------------------------------- */
/* E/S de bajo nivel                                                */
/* ---------------------------------------------------------------- */
static void initIO(void)
{
    /* Todas las lineas de LEDs en alta impedancia (entrada, sin pull-up) */
    DDRA &= (uint8_t)~LINES_PORTA_MASK;
    PORTA &= (uint8_t)~LINES_PORTA_MASK;
    DDRC &= (uint8_t)~LINES_PORTC_MASK;
    PORTC &= (uint8_t)~LINES_PORTC_MASK;

    /* Boton: entrada con pull-up interno (reposo = 1, presionado = 0) */
    DDRB &= (uint8_t)~(1 << BTN_GPIO);
    PORTB |= (1 << BTN_GPIO);
}

/* Apaga todo: las 5 lineas pasan a alta impedancia */
static void ledsOff(void)
{
    DDRA &= (uint8_t)~LINES_PORTA_MASK;
    DDRC &= (uint8_t)~LINES_PORTC_MASK;
    PORTA &= (uint8_t)~LINES_PORTA_MASK;
    PORTC &= (uint8_t)~LINES_PORTC_MASK;
}

/*
 * Enciende un solo LED: anodo = salida en 1, catodo = salida en 0,
 * las otras 3 lineas quedan en alta impedancia (tri-state).
 */
static void ledOn(eLedColor_t color, uint8_t cell)
{
    uint8_t an = redMap[cell][0];
    uint8_t ca = redMap[cell][1];
    if (color == eGreenLed)
    {
        uint8_t t = an;
        an = ca;
        ca = t;
    }
    ledsOff(); /* evita "fantasmas" al cambiar de LED */
    *lines[an].port |= (uint8_t)(1 << lines[an].bit);
    *lines[ca].port &= (uint8_t) ~(1 << lines[ca].bit);
    *lines[an].ddr |= (uint8_t)(1 << lines[an].bit);
    *lines[ca].ddr |= (uint8_t)(1 << lines[ca].bit);
}

/*
 * Barrido de Charlieplexing: en cada llamada (~1 ms) enciende el
 * SIGUIENTE LED que deba estar prendido segun las mascaras
 * (bit n = casilla n). Por persistencia de vision se ven todos a la vez.
 */
static void scanLeds(uint16_t redMask, uint16_t greenMask)
{
    static uint8_t slot = 0; /* 0..17: 0-8 rojos, 9-17 verdes */
    uint8_t tries;

    for (tries = 0; tries < 2 * NUM_LED_PER_COLOR; tries++)
    {
        slot = (uint8_t)((slot + 1) % (2 * NUM_LED_PER_COLOR));
        if (slot < NUM_LED_PER_COLOR)
        {
            if (redMask & (1U << slot))
            {
                ledOn(eRedLed, slot);
                return;
            }
        }
        else if (greenMask & (1U << (slot - NUM_LED_PER_COLOR)))
        {
            ledOn(eGreenLed, (uint8_t)(slot - NUM_LED_PER_COLOR));
            return;
        }
    }
    ledsOff(); /* nada que mostrar */
}

/* ---------------------------------------------------------------- */
/* 1. checkButton                                                   */
/* ---------------------------------------------------------------- */
/*
 * Maquina de estados no bloqueante, se llama cada ~1 ms.
 *   - eBtnLongKeyPress   : se mantiene presionado > 1000 ms.
 *   - eBtnDoubleKeyPress : segunda pulsacion < 500 ms despues de soltar.
 *   - eBtnShortKeyPress  : cualquier otra pulsacion.
 *   - eBtnUndefined      : aun no se clasifica / no hay accion.
 */
eButtonState_t checkButton(void)
{
    typedef enum
    {
        sIdle,
        sPressed,
        sWaitSecond,
        sWaitRelease
    } eBtnFsm_t;
    static eBtnFsm_t fsm = sIdle;
    static bool rawPrev = false;
    static bool stable = false; /* true = presionado (ya sin rebote) */
    static uint8_t debounceCnt = 0;
    static uint32_t tMark = 0;
    eButtonState_t result = eBtnUndefined;

    /* Anti-rebote: el nivel debe mantenerse DEBOUNCE_MS muestras */
    bool raw = (PINB & (1 << BTN_GPIO)) == 0;
    if (raw != rawPrev)
    {
        rawPrev = raw;
        debounceCnt = 0;
    }
    else if (debounceCnt < DEBOUNCE_MS)
    {
        debounceCnt++;
        if (debounceCnt == DEBOUNCE_MS)
        {
            stable = raw;
        }
    }

    switch (fsm)
    {
    case sIdle:
        if (stable)
        {
            tMark = millis;
            fsm = sPressed;
        }
        break;
    case sPressed:
        if (!stable)
        {
            tMark = millis; /* se solto: empieza ventana de doble clic */
            fsm = sWaitSecond;
        }
        else if ((millis - tMark) > LONG_PRESS_MS)
        {
            result = eBtnLongKeyPress;
            fsm = sWaitRelease;
        }
        break;
    case sWaitSecond:
        if (stable)
        {
            result = eBtnDoubleKeyPress;
            fsm = sWaitRelease;
        }
        else if ((millis - tMark) >= DOUBLE_PRESS_MS)
        {
            result = eBtnShortKeyPress;
            fsm = sIdle;
        }
        break;
    case sWaitRelease:
    default:
        if (!stable)
        {
            fsm = sIdle;
        }
        break;
    }
    return result;
}

/* ---------------------------------------------------------------- */
/* 2. playSequence                                                  */
/* ---------------------------------------------------------------- */
/*
 * Reproduce la animacion del estado final (no bloqueante):
 *   - eStalemate     : 'X' alternando rojo/verde.
 *   - eRedPlayerWin  : todo el tablero en rojo.
 *   - eGreenPlayerWin: todo el tablero en verde.
 * Cada ciclo: 1000 ms encendido, 500 ms apagado; se repite SEQ_REPEAT
 * veces. Retorna true mientras se reproduce y false al terminar.
 */
bool playSequence(eGameState_t gameState)
{
    static bool running = false;
    static uint32_t tStart = 0;
    const uint16_t ALL = 0x1FF;           /* casillas 0..8 */
    const uint16_t X_PATTERN = 0x155;     /* casillas 0,2,4,6,8 */
    const uint16_t PERIOD = SEQ_ON_MS + SEQ_OFF_MS;
    uint32_t elapsed;
    uint16_t cycle;
    uint16_t phase;
    uint16_t red = 0, green = 0;

    if (!running)
    {
        running = true;
        tStart = millis;
    }
    elapsed = millis - tStart;
    cycle = (uint16_t)(elapsed / PERIOD);
    phase = (uint16_t)(elapsed % PERIOD);

    if (cycle >= SEQ_REPEAT)
    {
        running = false;
        ledsOff();
        return false; /* animacion terminada */
    }

    if (phase < SEQ_ON_MS)
    {
        switch (gameState)
        {
        case eStalemate:
            if (cycle & 1U)
                green = X_PATTERN;
            else
                red = X_PATTERN;
            break;
        case eRedPlayerWin:
            red = ALL;
            break;
        case eGreenPlayerWin:
            green = ALL;
            break;
        default:
            break;
        }
    }
    scanLeds(red, green);
    return true;
}

/* ---------------------------------------------------------------- */
/* 3. checkBoard                                                    */
/* ---------------------------------------------------------------- */
static bool isCellFree(const sBoardState_t *b, uint8_t cell)
{
    return !b->gameBoard[eRedLed][cell] && !b->gameBoard[eGreenLed][cell];
}

/* Busca la siguiente casilla libre partiendo de 'from' (sin incluirla)
 * en direccion 'step' (+1 / -1), dando la vuelta al tablero. */
static bool findFreeCell(const sBoardState_t *b, uint8_t from, int8_t step, uint8_t *out)
{
    uint8_t i;
    uint8_t cell = from;
    for (i = 0; i < NUM_LED_PER_COLOR; i++)
    {
        cell = (uint8_t)((cell + NUM_LED_PER_COLOR + step) % NUM_LED_PER_COLOR);
        if (isCellFree(b, cell))
        {
            *out = cell;
            return true;
        }
    }
    return false;
}

static bool hasWon(const sBoardState_t *b, eLedColor_t color)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (b->gameBoard[color][winLines[i][0]] &&
            b->gameBoard[color][winLines[i][1]] &&
            b->gameBoard[color][winLines[i][2]])
        {
            return true;
        }
    }
    return false;
}

eGameState_t checkBoard(sBoardState_t *boardState, eButtonState_t buttonState)
{
    uint8_t next;

    switch (buttonState)
    {
    case eBtnShortKeyPress: /* siguiente casilla libre */
        if (findFreeCell(boardState, boardState->cursor, +1, &next))
            boardState->cursor = next;
        break;
    case eBtnDoubleKeyPress: /* casilla libre anterior */
        if (findFreeCell(boardState, boardState->cursor, -1, &next))
            boardState->cursor = next;
        break;
    case eBtnLongKeyPress: /* colocar ficha */
        if (isCellFree(boardState, boardState->cursor))
        {
            boardState->gameBoard[boardState->currentColor][boardState->cursor] = true;

            if (hasWon(boardState, boardState->currentColor))
            {
                return (boardState->currentColor == eRedLed) ? eRedPlayerWin
                                                             : eGreenPlayerWin;
            }
            boardState->currentColor =
                (boardState->currentColor == eRedLed) ? eGreenLed : eRedLed;

            /* Mover el cursor a una casilla disponible; si no hay, empate */
            if (!findFreeCell(boardState, boardState->cursor, +1, &next))
            {
                return eStalemate;
            }
            boardState->cursor = next;
        }
        break;
    default:
        break;
    }
    return eOngoingGame;
}

/* ---------------------------------------------------------------- */
/* 4. displayBoard                                                  */
/* ---------------------------------------------------------------- */
/*
 * Muestra las fichas de boardState->gameBoard[][] y hace parpadear
 * (500 ms encendido / 100 ms apagado) el LED del color en turno en la
 * posicion del cursor.
 */
void displayBoard(sBoardState_t *boardState)
{
    uint16_t red = 0, green = 0;
    uint8_t i;

    for (i = 0; i < NUM_LED_PER_COLOR; i++)
    {
        if (boardState->gameBoard[eRedLed][i])
            red |= (uint16_t)(1U << i);
        if (boardState->gameBoard[eGreenLed][i])
            green |= (uint16_t)(1U << i);
    }

    if ((millis % (CURSOR_ON_MS + CURSOR_OFF_MS)) < CURSOR_ON_MS)
    {
        if (boardState->currentColor == eRedLed)
            red |= (uint16_t)(1U << boardState->cursor);
        else
            green |= (uint16_t)(1U << boardState->cursor);
    }
    scanLeds(red, green);
}

/* ---------------------------------------------------------------- */
/* Lazo principal                                                   */
/* ---------------------------------------------------------------- */
int app_main(void)
{
    eGameState_t currentGameState = eGameRestart;
    eButtonState_t buttonState;
    sBoardState_t boardState;
    uint8_t idx;

    initIO();

    while (1)
    {
        buttonState = checkButton();

        switch (currentGameState)
        {
        case eGameRestart:
            for (idx = 0; idx < NUM_LED_PER_COLOR; idx++)
            {
                boardState.gameBoard[eRedLed][idx] = false;
                boardState.gameBoard[eGreenLed][idx] = false;
            }
            boardState.cursor = 0;
            boardState.currentColor = eRedLed;
            currentGameState = eOngoingGame;
            break;
        case eOngoingGame:
            if (buttonState != eBtnUndefined)
                currentGameState = checkBoard(&boardState, buttonState);
            displayBoard(&boardState);
            break;
        case eStalemate:
        case eRedPlayerWin:
        case eGreenPlayerWin:
            if (!playSequence(currentGameState))
                currentGameState = eGameRestart;
            break;
        default:
            currentGameState = eGameRestart;
            break;
        }
        delay(1);
        millis++;
    }
    return 0;
}
