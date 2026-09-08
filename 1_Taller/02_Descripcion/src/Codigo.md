- Practica 2: AVR OpCode -
LDI     R31,    2
MOV     R29,    R31
LDI     R16,    24
ST      Z+,     R16
LDI     R16,    19
ST      Z+,     R16
LDI     R16,    21
ST      Z+,     R16
LDI     R16,    8
ST      Z+,     R16
LDI     R16,    3
ST      Z+,     R16
LDI     R16,    9
ST      Z+,     R16
LDI     R26,    5
MOV     R5,     R26
LDI     R26,    20
CLR     R18
LD      R16,    Y+
ANDI    R16,    31
LDI     R17,    0
TST     R18
BREQ    2
RCALL   18
LD      R19,    X
OR      R19,    R16
ST      X,      R19
CPI     R18,    4
BRLT    5
INC     R26
LD      R19,    X
OR      R19,    R17
ST      X,      R19
ADD     R18,    R5
CPI     R18,    8
BRLT    2
SUBI    R18,    8
CP      R30,    R28
BRNE    -20
NOP
RJMP    -1
PUSH    R18
BST     R16,    7
LSL     R16
LSL     R17
BLD     R17,    0
DEC     R18
BRNE    -5
POP     R18
RET