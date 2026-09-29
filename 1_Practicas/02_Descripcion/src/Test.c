#include <stdio.h>
int main()
{
    char *car1, car2; // Ambas instancias son apuntadores? Answer 1 Question 1 Si

    char cadena[10] = {0}; // Inicializar todos los elementos con cadena vacia
    int multi_enteros[5][5], *apuntador, valor = 2;

    cadena[1] = 'H';
    cadena[2] = 'o';
    cadena[3] = 'l';
    cadena[4] = 'a';
    printf("Inicia aqui\n\n"); // Que imprime? Answer 2 Question 1 "Hola"
    printf("%s", cadena);      // Que imprime? Answer 2 Question 1 "Hola"

    printf("%s", &cadena[1]); // Imprimir "Hola"
    printf("\n");
    printf("%s", cadena + 3); // Que imprime? "la"

    printf("\n");

    apuntador = (int *)multi_enteros; // Aplicar cast apropiado para el apuntador receptor
    multi_enteros[1][1] = 1;
    multi_enteros[1][2] = valor;
    multi_enteros[1][3] = 3;

    printf("%d", apuntador[7]); // Que imprime? Answer 7 Question 1 "1"

    printf("\n");

    car1 = (char *)&multi_enteros[3][2];
    car1[0] = '1';
    *(car1 + 1) = '2';

    printf("%d", apuntador[17]); // Que imprime? Answer 8 Question 1 "21"

    return 0;
}