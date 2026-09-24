#!/bin/bash

# Script para compilar la practica con avr-gcc (WinAVR) y generar el .hex
# Requiere que avr-gcc / avr-objcopy / avr-size esten en el PATH
#
# Ahora compila DOS archivos fuente y los enlaza juntos:
#   - main.c        
#   - MyRutines.asm 

MCU="atmega2560"
F_CPU="16000000UL"
NOMBRE="main"

echo "Compilando main.c para ${MCU} @ ${F_CPU}..."
avr-gcc -mmcu=${MCU} -DF_CPU=${F_CPU} -Os -c main.c -o main.o
if [ $? -ne 0 ]; then
  echo "Error al compilar main.c. No se genero el .hex."
  exit 1
fi

echo "Ensamblando MyRutines.asm..."
avr-gcc -mmcu=${MCU} -DF_CPU=${F_CPU} -x assembler-with-cpp -c MyRutines.asm -o MyRutines.o
if [ $? -ne 0 ]; then
  echo "Error al ensamblar MyRutines.asm. No se genero el .hex."
  exit 1
fi

echo "Enlazando..."
avr-gcc -mmcu=${MCU} -o "${NOMBRE}.elf" main.o MyRutines.o
if [ $? -ne 0 ]; then
  echo "Error al enlazar. No se genero el .hex."
  exit 1
fi

avr-objcopy -O ihex -R .eeprom "${NOMBRE}.elf" "${NOMBRE}.hex"
avr-size --mcu=${MCU} -C "${NOMBRE}.elf"

echo "Listo: ${NOMBRE}.hex generado correctamente."

# Para flashear a la Elegoo Mega 2560 con avrdude (descomenta y ajusta el puerto COM):
# Se usa "-c arduino" en vez de "-c wiring": el avrdude.conf de este WinAVR (2010)
# es anterior a que avrdude agregara el programador "wiring", pero "arduino" habla
# el mismo protocolo STK500v1 que usa el bootloader de fabrica de la Mega2560/Elegoo
# a 115200 baudios, asi que funciona igual para flashear.
avrdude -c arduino -p m2560 -P COM3 -b 115200 -D -U flash:w:${NOMBRE}.hex:i
