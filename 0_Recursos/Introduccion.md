# Repaso de Microcontroladores y Arquitectura de Computadoras

---

## 1. Conceptos Fundamentales: Microcomputadoras vs. Microcontroladores

- **Estructura de la CPU y Buses**: La arquitectura de la CPU define el tamaño del bus de datos. Los bloques internos comparten tres buses principales:
  - **Bus de Datos**: Transporta la información entre los bloques.
  - **Bus de Direcciones**: Especifica las posiciones de memoria o periféricos.
  - **Bus de Control**: Transporta señales de sincronización, lectura/escritura e interrupciones.

- **Definición de Microcontrolador**: A diferencia de una microcomputadora convencional (donde CPU, RAM, almacenamiento y periféricos son chips independientes), un microcontrolador integra en un único chip de silicio:
  - CPU (Unidad Central de Procesamiento)
  - Memoria volátil (RAM)
  - Memoria no volátil (ROM / Flash / EEPROM)
  - Periféricos integrados: Puertos de E/S (*I/O*), Temporizadores (*Timers*), Convertidor Analógico-Digital (*ADC*), Temporizador Guardián (*Watchdog Timer*), etc.

- **Señal de Reloj (Clock)**: 
  - Se puede conectar un cristal de cuarzo externo (`XTAL`) para máxima precisión temporal.
  - Los microcontroladores modernos cuentan con osciladores RC internos, requiriendo únicamente alimentación ($V_{CC}$ y $GND$) para operar de forma totalmente funcional.

---

## 2. Arquitecturas de Memoria y Conjuntos de Instrucciones

### 2.1. Arquitectura Von Neumann vs. Harvard

| Característica | Arquitectura Von Neumann | Arquitectura Harvard |
| :--- | :--- | :--- |
| **Buses** | Bus único para código (instrucciones) y datos. | Buses separados e independientes para código y datos. |
| **Acceso a Memoria** | Secuencial / Alternado (requiere múltiples ciclos). | Simultáneo (lee instrucción y accede a datos en el mismo ciclo). |
| **Eficiencia** | Menor velocidad por ciclo de reloj. | Alta velocidad / rendimiento optimizado. |
| **Uso Principal** | Microprocesadores de propósito general (PC, Servidores, x86). | Microcontroladores (AVR, PIC, ARM Cortex-M, ESP32). |

### 2.2. Conjuntos de Instrucciones: CISC vs. RISC

* **CISC (*Complex Instruction Set Computing*)**:
  * Instrucciones de tamaño variable (de 1 a 8+ bytes en x86).
  * Decodificación compleja por hardware; requiere múltiples ciclos de reloj por instrucción.
  * Diseñado originalmente para minimizar el uso de memoria de programa cuando la RAM era costosa.
* **RISC (*Reduced Instruction Set Computing*)**:
  * Instrucciones de tamaño fijo (ej. 16 bits en AVR, 32 bits en ARM).
  * Ejecución optimizada en un solo ciclo de reloj (*pipelining* altamente eficiente).
  * Predominante en microcontroladores y dispositivos embebidos.

### 2.3. ¿Por qué las computadoras de escritorio (PC) siguen usando CISC / Von Neumann?

1. **Altas frecuencias de reloj**: Las PC operan a frecuencias de varios GHz, compensando la ineficiencia por ciclo mediante fuerza bruta de cómputo (a costa de mayor consumo de energía).
2. **Compatibilidad heredada (*Legacy*)**: Existe una infraestructura masiva de software escrito para la arquitectura x86. El costo financiero y operativo de migrar todo el ecosistema de software a RISC históricamente ha sido prohibitivo.

---

## 3. Evolución Histórica de los Microcontroladores y Periféricos

### 3.1. Orígenes e Hitos
1. **Intel 4004 (1971)**: Primer microprocesador comercial en un solo chip.
2. **Texas Instruments TMS1000**: Considerado el primer microcontrolador real, diseñado inicialmente para calculadoras integrando CPU, RAM, ROM e I/O.

### 3.2. Evolución de la Tecnología de Memoria
* **ROM (Read-Only Memory)**: Programada en fábrica mediante máscaras de fotolitografía. Imposible de modificar. Aún se utiliza en producción masiva ultrabarata por su bajo costo por unidad.
* **EPROM / EEPROM**: Permitió grabar los microcontroladores en vacío y reescribirlos eléctricamente.
* **Memoria Flash**: Revolucionó la industria al abaratarse masivamente y permitir el repensado de sistemas mediante actualizaciones de firmware en el campo (*In-System Programming* / ISP).

### 3.3. Periféricos Integrados a través del Tiempo
* **Timers (8 y 16 bits)**: Medición precisa de tiempo, generación de demoras e interrupciones periódicas.
* **Interfaces de Comunicación Serial**:
  * **UART / SCI**: Comunicación asíncrona estándar (~1 MHz).
  * **I2C**: Protocolo síncrono de 2 hilos (SDA, SCL) a 400 kHz. Soporta múltiples dispositivos (hasta 255) en un mismo bus, aunque requiere gestión de protocolo más compleja en software.
  * **SPI / QSPI**: Bus síncrono de alta velocidad. QSPI usa 4 líneas de datos para permitir la ejecución directa de código desde memorias Flash externas (*Execute In Place* / XIP).
* **CAN Bus (*Controller Area Network*)**: Protocolo diferencial ultra robusto diseñado para entornos con alta interferencia electromagnética (automoción, aeronáutica, robótica industrial).
* **PWM (Modulación por Ancho de Pulso) y ADC (Convertidores Análogo-Digital)**: Control de potencia, motores y adquisición de señales del mundo físico.
* **Modos de Bajo Consumo (*Sleep Modes*)**: Apagado selectivo de módulos internos para ahorrar energía.
* **Texas Instruments MSP430**: Microcontrolador de 16 bits emblemático por su ultra bajo consumo de corriente, clave en el desarrollo de los primeros dispositivos vestibles (*smartwatches*).

---

## 4. El Panorama del Mercado: De los 8 bits a los 32 bits

### 4.1. La Era de los 8 Bits
* **Intel 8051**: Arquitectura histórica que sigue viva en chips integrados económicos (ej. módulos Bluetooth de bajo costo) porque expiraron sus patentes y no requiere pago de regalías.
* **Microchip (PIC)**: Enfocado históricamente en reducción agresiva de costos (~$0.40 USD). Dominó la industria por décadas y mantiene una gran presencia en exámenes académicos y certificaciones (como Ceneval en México).
* **Atmel (AVR)**: Presenta un diseño de registros y conjunto de instrucciones muy limpio, ordenado y eficiente. Aunque con un costo por unidad mayor (~$2.00 USD), se convirtió en el estándar educativo y creador al ser el corazón de la plataforma **Arduino**.
* **Impacto de la escasez de componentes (Pandemia de COVID-19)**: La falta de suministro afectó severamente a Microchip, lo que aceleró la migración de la industria desde diseños legados de PIC hacia arquitecturas ARM Cortex-M.

### 4.2. La Era de 32 Bits y el Dominio de ARM
* **Orígenes de ARM**: Surgió de las investigaciones de Acorn Computers en Cambridge, debutando en la computadora educativa *BBC Micro*.
* **Familias de Procesadores ARM**:
  * **Cortex-A**: Diseñada para alto rendimiento con sistemas operativos complejos (Smartphones, PC, Raspberry Pi).
  * **Cortex-M**: Diseñada específicamente para microcontroladores y aplicaciones de tiempo real.
* **Evolución de la Familia Cortex-M**:
  * **Cortex-M0 / M0+**: Microcontroladores de 32 bits a más de 70 MHz por menos de $1.00 USD. Desplazaron a los microcontroladores de 8 bits al ofrecer mucho mayor rendimiento de cómputo con un consumo de energía comparable.
  * **Cortex-M3 / M4**: Incorporan instrucciones dedicadas a **DSP** (Procesamiento Digital de Señales) y **FPU** (Unidad de Punto Flotante por hardware).
  * **Cortex-M33**: Introduce **TrustZone** para ARMv8-M, proporcionando aislamiento por hardware entre zonas de código seguro y no seguro (esencial para la seguridad en IoT, gestión de claves criptográficas y verificación de firmware).
* **Tendencia Actual (Edge AI / NPU)**: Integración de aceleradores de redes neuronales (NPU - *Neural Processing Unit*) en el silicio de los microcontroladores para ejecutar modelos de Inteligencia Artificial locales directamente en el dispositivo (*Edge Computing*), reduciendo latencia y dependencia de la nube.

---

## 5. Metodología del Curso y Entorno de Desarrollo

### 5.1. ¿Por qué se utiliza AVR (ATmega2560) en lugar de ARM o ESP32?
* **ARM Cortex-M**: Aunque es el estándar industrial, requiere una configuración inicial (*startup*) muy compleja de árboles de reloj, multiplexación de pines y registros de bajo nivel, lo que dificulta el aprendizaje inicial.
* **ESP32**: Es extremadamente potente (32 bits, dual-core, Wi-Fi/Bluetooth), pero su desarrollo depende fuertemente de librerías y del marco de trabajo (SDK / ESP-IDF / Arduino), lo que oculta los conceptos fundamentales de hardware.
* **Atmel AVR (ATmega2560)**: Ofrece un equilibrio perfecto: una arquitectura limpia de registros de 8 bits donde se manipula el hardware directamente a nivel de registros ("a nivel de silicio") sin capas de abstracción innecesarias.

### 5.2. Herramientas y Normas de Programación
* **Lenguaje**: Lenguaje C y ensamblador utilizando el compilador `avr-gcc`.
* **Entorno Integrado**: **Microchip Studio** (anteriormente Atmel Studio).
* **Restricción de Librerías Estándar**:
  * **No se utilizará la biblioteca estándar de C** (`<stdio.h>`) ni funciones como `printf()` o `scanf()`.
  * Se desarrollarán controladores (*drivers*) propios accediendo directamente a las cabeceras de hardware como `<avr/io.h>`.
* **Depuración**: Se empleará el simulador por software integrado en Microchip Studio para analizar el estado de los registros, banderas y memoria del ATmega2560 instrucción por instrucción.
