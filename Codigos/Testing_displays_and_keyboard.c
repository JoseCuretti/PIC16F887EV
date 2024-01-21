#include <xc.h>
#define _XTAL_FREQ 4000000     // Frecuencia del oscilador (en Hz)

// Definición de los pines de control
#define CONTROL_PIN1   RE0
#define CONTROL_PIN2   RE1
#define CONTROL_PIN3   RE2


// Definición de los pines de datos
#define DATA_PORT      PORTD

//Prototipo de funciones
void Display(int i);
void Teclado(void);
//Variables globales
int valorDisplay[] = {
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x77, // A
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x7C, // B
    0x07, // 7
    0x7F, // 8
    0x6F, // 9
    0x39, // C
    0x79, // E
    0x3F, // 0
    0x71, // F
    0x5E  // D     
};

void main(void) {
    // Configuración de puertos
    ANSEL  = 0x00; // Todos los pines I/O son digitales
    ANSELH = 0x00; // Todos los pines I/O son digitales
    TRISD  = 0x00; // Configura el puerto D como salida
    TRISE  = 0x00; // Configura el puerto E como salida
    TRISA  = 0x0F; // Parte baja del PORTA como entrada
    TRISB  = 0x00; // PORTB como salida
    
    for(int index = 0;index < 16;index++){
     Display(index);
    }
    while (1) {
     Teclado();
    }
    return;
}
void Display(int i){
            
        for(int j=0;j<40;j++){   
           // Display 1
           CONTROL_PIN1 = 1;
           DATA_PORT = valorDisplay[i]; 
           __delay_ms(5);
           CONTROL_PIN1 = 0;

           // Display 2
           CONTROL_PIN2 = 1;
           DATA_PORT = valorDisplay[i];
           __delay_ms(5);
           CONTROL_PIN2 = 0;

           // Display 3
           CONTROL_PIN3 = 1;
           DATA_PORT = valorDisplay[i];
           __delay_ms(5);
           CONTROL_PIN3 = 0;
           }
        }
void Teclado(void){
    for (int fila = 0; fila < 4; fila++) {
        // Envío un 0 por cada fila
        PORTB = ~(1 << fila);

        // Lee las columnas y verifica si alguna tecla está presionada
        for (int col = 0; col < 4; col++) {
            if (!(PORTA & (1 << col))) {
               // Una tecla ha sido presionada
               // retardo de antirebote
                __delay_ms(100);
               // Verifica nuevamente si la tecla sigue presionada después del retardo
               if (!(PORTA & (1 << col))) {
               // Muestra el valor de la tecla en los displays   
                    Display(fila * 4 + col);
               // Espera a que se libere la tecla
                    while (!(PORTA & (1 << col)));
                }
            }
        }
        // Pone al PORTB en 1 nuevamente
        PORTB = 0x0F;
    }
}