#include <xc.h>
#define _XTAL_FREQ 4000000     // Frecuencia del oscilador (en Hz)

// Definición de los pines de control
#define CONTROL_PIN1   RE0
#define CONTROL_PIN2   RE1
#define CONTROL_PIN3   RE2


// Definición de los pines de datos
#define DATA_PORT      PORTD

//Prototipo de funciones
void Display(void);
//Variables globales
int valorDisplay[] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71, // F
    0x80 // PUNTO    
};

void main(void) {
    // Configuración de puertos
    TRISD = 0x00; // Configura el puerto D como salida
    TRISE = 0x00; // Configura el puerto E como salida

    while (1) {
    Display();
    }
    return;
}
void Display(void){
   for(int index = 0;index < 18;index++){
         
        for(int j=0;j<50;j++){   
           // Display 1
           CONTROL_PIN1 = 1;
           DATA_PORT = valorDisplay[index]; 
           __delay_ms(5);
           CONTROL_PIN1 = 0;

           // Display 2
           CONTROL_PIN2 = 1;
           DATA_PORT = valorDisplay[index];
           __delay_ms(5);
           CONTROL_PIN2 = 0;

           // Display 3
           CONTROL_PIN3 = 1;
           DATA_PORT = valorDisplay[index];
           __delay_ms(5);
           CONTROL_PIN3 = 0;
           }
        } 
   }
