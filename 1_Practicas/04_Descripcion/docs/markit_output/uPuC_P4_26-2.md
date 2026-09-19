U.A.B.C.

Microprocesadores y Microcontroladores

Práctica 4

Manejo de la sección de E/S del micocrontrolador ATmega1280/2560

Objetivo: Mediante esta práctica el alumno analizará la implementación de retardos por
software, así como también se familiarizará con la configuración y uso de puertos.

Equipo:
Teoría:           1) Investigación a cerca de ensamblador en línea para GCC.

-  Computadora Personal con AVR Studio, AVRDUDE y tarjeta TJuino

2) Convención de llamadas a rutinas de ASM desde C en AVR-GCC.
3) Técnicas de anti-rebote de botones táctiles.

Descripción:

Implementar un programa en base al código del repositorio base, el cual revisa el
estado del los botones conectados en  PK0  y  PK1, y dependiendo de su duración
realiza un incremento o decremento de un conteo que es desplegado sobre el display
de siete segmentos conectado en el Puerto-L en base al siguiente diagrama:

Macros a implementar:
SetBitPort(port, bit)
Macro que inserta la instrucción de ensamblador SBI, mediante inline assembly.

1.

2. ClrBitPort(port, bit)

Macro que inserta la instrucción de ensamblador CBI, mediante inline assembly.

Funciones a implementar:

3.

4.

5.

void delay(uint16_t mseg);
Función   que   debe   de   tardarse  n  ms  en   retornar,   según   se   especifique   en   el
parámetro de entrada, la cual deberá estar implementada en  ensamblador  y ser
llamada desde C. Se pide una exactitud de ±5 us.

void InitPorts(void);
Inicialización requerida de los puertos utilizados en esta práctica según el
diagrama.

void Display7Seg(uint8_t data)
Función que despliega el parámetro data sobre el display de siete segmentos.

L. AGUILAR / J. GARCÍA

Pag.1

S26-2

U.A.B.C.

Microprocesadores y Microcontroladores

6. uint8_t checkButton(uint8_t *pin_register, uint8_t pin_number);

Retorna el estado del botón en la ubicado en sus parámetros, detectando entre un
presionado de duración corta o larga. Donde el umbral para una larga duración es
cualquiera que sea mayor a 1 seg. Implementar técnicas de anti-rebote.

Fig. 1. Ejemplo del rebote mecánico de un botón.

Comentarios y Conclusiones

Bibliografía y Referencias

L. AGUILAR / J. GARCÍA

Pag.2

S26-2

