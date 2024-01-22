/*
 * File:   PruebaHardwarePIC16F887EV.c
 *  Programa de prueba de Hardaware para Placa de Evaluacion de PIC16F887 v1.0
 *  Version 1.2
 *  Fecha 21/01/2024
 *  MPLAB X IDE v6.10
 *  Compilador XC8 v2.32
 */


// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

#include <xc.h>

// Definiciones
//Tonos
#define dTonoRX         2
#define dTonoTecla      4
//Estados/Seteos de Display
#define dDisH 0x01
#define dDisM 0x02
#define dDisL 0x04
//Estados/Seteos de Teclado
#define dCol1 0x01
#define dCol2 0x02
#define dCol3 0x04
#define dCol4 0x08
#define dFil1 0x01
#define dFil2 0x02
#define dFil3 0x04
#define dFil4 0x08
//Teclas especiales
#define dENT    14
#define dESC    15
#define dARRIBA 16
#define dABAJO  17

#define XTALFREC 4000000     //El oscilador interno anda  a 4 MHz

#define BUZZER  PORTAbits.RA4
#define tABAJO  PORTBbits.RB5
#define tARRIBA PORTBbits.RB4
#define DisplayL PORTEbits.RE2
#define DisplayM PORTEbits.RE1
#define DisplayH PORTEbits.RE0

#define pPROG         PORTCbits.RC5
#define LEDVERDE      PORTCbits.RC0
#define LEDAMARILLO   PORTCbits.RC1
#define LEDROJO       PORTCbits.RC2

static int TECLAS[]={0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111,0b01110111,0b01111100,0b00111001,0b01011110} ; // "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" "A" "b" "C" "d"
static int NUMEROS[]={0b00111111,0b00000110,0b01011011,0b01001111,0b01100110,0b01101101,0b01111101,0b00000111,0b01111111,0b01101111} ; // "0" "1" "2" "3" "4" "5" "6" "7" "8" "9"
static int LETRAS[]={0b01110111,0b01111100,0b00111001,0b01011110,0b01111001,0b01110001} ;  // "A" "b" "C" "d" "E" "F"
static int ENT[]={0b01111001,0b01010100,0b01111000};
static int ESC[]={0b01111001,0b01101101,0b00111001};
static int ARRIBA[]={0b01010000,0b00100011,0b01000100};
static int ABAJO[]={0b01100000,0b00011100,0b01000010};

//Variables Globales
int gcontadorTMR0;
int gcontadorAux;
int gDigL,gDigM,gDigH; // Valores que enciende cada display (H-M-L)
int gContadorAR;    //Contador Antirebote
int gContadorADC;    //Contador ADC
int gBuzzerOn;      //Activacion de Buzzer (en periodos de TMR0)
int gBuzzerCont;     //Contador para generar onda cuadrada
int gBuzzerT;        //Preiodo de onda cuadrada (en periodos de TMR0)
int gTeclaEspecial;  //Bandera para teclas ENT ESC ARRIBA y ABAJO
int gEstadoDisplay;  // Indica que display esta encendido
int gEstadoTeclado;  // Indica que columan se esta chequeando

// Prototipos de Funciones
void DisplayTecla(int tecla, int tono);     //Muestra un numero en Displays
void Hex2Decimal(int HEX);                  //Muestra el decimal de un hexa en displays
void Esperar(int N);                        //Espera un numero de ciclos de TMR0
void SetPWM(int Ancho);                     //Fija un valor de PWM entre 0 y 255
void CambiarPWM(int Salto);                 //Cambia el valor de PWM actual

///////////////////////////////////////////////////////////////////////////////
// Interrupción
///////////////////////////////////////////////////////////////////////////////
void __interrupt() INTERRUPT_InterruptManager (void)
{
    // interrupt handler
    //  Solo tenemos la interrupcion de TMR0 aproximadamente cada 0,5mseg
    if(INTCONbits.PEIE == 1)   //Estan hanbilitadas las interrupciones de Periféricos?
    {
        if( INTCONbits.T0IF == 1)  //Interrupción de TMR0 ?
        {
            INTCONbits.T0IF=0; 
            gcontadorTMR0++;
            
            //Actualizamos contadores
            if(gContadorAR>0)
                gContadorAR--;
            if(gContadorADC>0)
                gContadorADC--; 
            if(gcontadorAux>0)
                gcontadorAux--; 
            
            //Buzzer
            if(gBuzzerOn>0){
               gBuzzerOn--;
               gBuzzerCont++;
               if(gBuzzerCont>=gBuzzerT) {
                   gBuzzerCont=0;
                   BUZZER ^= 1;
               }
            }
            else
                BUZZER=0;   //Para que no quede haciendo ruido por inteferencia

            
            //Barrido Teclado
            PORTA&=0xF0;  //Apago las 4
            switch (gEstadoTeclado) {
                case dCol1:
                    gEstadoTeclado=dCol2;
                    PORTA|=dCol2; //Enciendo Columna 2
                    break;
                 case dCol2:
                    gEstadoTeclado=dCol3;
                    PORTA|=dCol3; //Enciendo Columna 3
                    break;
                case dCol3:
                    gEstadoTeclado=dCol4;
                    PORTA|=dCol4; //Enciendo Columna 4
                    break;
                case dCol4:
                    gEstadoTeclado=dCol1;
                    PORTA|=dCol1; //Enciendo Columna 1
                    break;                    
            }             
            
            //Barrido Display
            PORTD=0;     //Apago todos los segmentos
            switch (gEstadoDisplay) {
                case dDisH:
                    PORTE=dDisL;  //Enciendo Display L
                    PORTD=gDigL;  //Enciendo segmentos correspondientes
                    gEstadoDisplay=dDisL;
                    break;
                 case dDisL:
                    PORTE=dDisM;  //Enciendo Display M
                    PORTD=gDigM;  //Enciendo segmentos correspondientes
                    gEstadoDisplay=dDisM;
                    break;
                case dDisM:
                    PORTE=dDisH;  //Enciendo Display H
                    PORTD=gDigH;  //Enciendo segmentos correspondientes
                    gEstadoDisplay=dDisH;
                    break;
            }                    
        } 
 
        else
        {
            //Unhandled Interrupt
        }
    }      
    else
    {
        //Unhandled Interrupt
    }
}


///////////////////////////////////////////////////////////////////////////////
// Funciones
///////////////////////////////////////////////////////////////////////////////
void DisplayTecla(int tecla, int tono)
{
    //Transmitimos por USART
    TXREG=tecla;
    
    //Teclas no "especiales", con desplazamiento
    if(tecla<14) {  
        if(gTeclaEspecial==1){  //Si viene de una especial, borro lo anterior
             gDigM=0;
             gDigL=0;
             }
        gTeclaEspecial=0;    //Reseteo bandera especial
        gDigH=gDigM;           //Desplazamiento
        gDigM=gDigL;           //Desplazamiento
        gDigL=TECLAS[tecla];           //Desplazamiento
        }
    else{
    gTeclaEspecial=1;       //Seteo bandera especial
            switch (tecla) {
                case dENT:  //"ENT"
                    gDigH=ENT[0];
                    gDigM=ENT[1];
                    gDigL=ENT[2];
                    break;
                case dESC:  //"ESC"
                    gDigH=ESC[0];
                    gDigM=ESC[1];
                    gDigL=ESC[2];
                    break;
                case dARRIBA:  //"ARRIBA"
                    gDigH=ARRIBA[0];
                    gDigM=ARRIBA[1];
                    gDigL=ARRIBA[2];
                    //Aumento 10% brillo LED central
                    CambiarPWM(10); 
                    break;
                case dABAJO:  //"ABAJO"
                    gDigH=ABAJO[0];
                    gDigM=ABAJO[1];
                    gDigL=ABAJO[2];
                    //Reduzco 10% brillo LED central
                    CambiarPWM(-10);                   
                    break;                    
            }     
    }
//beep    
gBuzzerT=tono;      //Tono
gBuzzerOn=100;      //Tiempo

}

//////////////////////////////////////////////////////////////////////////
void Hex2Decimal(int HEX)    //Muesta valor de byte en decimal eb display
{
    int Aux;
    //Convertimos
    Aux=0;
    while(HEX>99) {
        HEX-=100;
        Aux++;
    }
    gDigH=TECLAS[Aux];
    Aux=0;
    while(HEX>9) {
        HEX-=10;
        Aux++;
    }
    gDigM=TECLAS[Aux];
    gDigL=TECLAS[HEX];
    gTeclaEspecial=1; 
}
//////////////////////////////////////////////////////////////////////////
void Esperar(int N)    //Espera N interrupciones
{
gcontadorAux=N;
while(gcontadorAux>0); 
}
//////////////////////////////////////////////////////////////////////////
void SetPWM(int Ancho)    //Espera N interrupciones
{
CCPR2L=Ancho;
}
//////////////////////////////////////////////////////////////////////////
void CambiarPWM(int Salto)
{
    int PWMactual=CCPR2L;
    if(Salto>0){
        if(PWMactual+Salto>255)
            SetPWM(255);
        else
            SetPWM(PWMactual+Salto);
    }
    else {
         if(PWMactual+Salto<0)
            SetPWM(0);
        else
            SetPWM(PWMactual+Salto);       
    }

}
///////////////////////////////////////////////////////////////////////////////
//  MAIN
///////////////////////////////////////////////////////////////////////////////
void main(void) {
    //Variables locales
    int TeclaP;
    int AuxByte,n,m;
    
    //Inicializacion
    //Puertos
    ANSEL=0x10;    //ANS4=RA5 analogica el resto digitales
    ANSELH=0x00;   //Todos digiales
    TRISA=0x20;    //RA7yRA6:Salidas:Reservados para oscilador. RA5:Entrada:ADC RA4:Salida:BUZZER  RA3-RA0:Salida:Teclado    
    PORTA=0x00;    // Todo apagado 
    TRISB=0xFF;    //RB7yRB6:Entradas:Reservadas para ICSP RB5yRB4:Entradas:Botones Arriba y Abajo  RB3-RB0:Entradas:Teclado
    TRISC=0xFF;    //RC7yRC6:Entradas:RXyTX Usart  RC5-RC3:Entradas:SPI(no implementado) RC5:Entrada:PROGRAMACION RC2-RC0:Salidas:LEDS   (Se Configura luego de setear PWM)
    PORTC=0x00;    //LEDS apagados
    TRISD=0x00;    //Salidas: los segmentos de los Displays
    PORTD=0x00;    // Todo apagado 
    TRISE=0xF8;    // RE2-RE0:Salidas:Catodos de los displays
    PORTE=0x00;    // Todo Apagado

   //Perifericos    
    //PWM
    PR2=100;              // Periodo del PWM   100x16ux= 1,6mseg
    CCP2CON=0b00001100;   // MODO PWM
    CCPR2L=0;             // Ancho Pulso=0;
    T2CON=0b00000110;     //TMR2: Pos escaler=1   Preescaler 16    TMR2=ON 
    TRISC=0xB8;           //Hsbilito salida de PWM
    //USART
    TXSTA=0b00100100;   //TX habilitado, modo asincronico, 8bits, High speed
    RCSTA=0b10010000;   //USART habilitada, RX habilitado, 8bits
    BAUDCTL=0b00000000; // AutoBaudRate deshabilitado, TX por RB7, 8bit baud rate generator
    SPBRG=25;           // 51: 4800bps  25: 9600bps  12:19200bps

    //ADC
    ADCON1=0b00000000;    // Justificado Iquierdo,  Vref+=VCC  Vref-=GND
    ADCON0=0b10010011;    // Frecuencia minima, AN=AN5 ADC=ON   Inicio de Conversion

    //TMR0
    //OPTION_REG=0b11000001; //Preescaler para TMR0, a 1:4   Interrupcion cada 1ms
    OPTION_REG=0b11000000; //Preescaler para TMR0, a 1:2   Interrupcion cada 0.5ms 

    
    //Variables
    
    //Estados iniciales
    gEstadoDisplay=dDisL;
    gEstadoTeclado=dCol1;
    gContadorAR=0;
    gContadorADC=0;   
    gTeclaEspecial=0; 
    gcontadorTMR0=0;
    gcontadorAux=0;
            
    //Display apagados
    gDigL=0;
    gDigM=0;
    gDigH=0;
    //Buzzer Apagado
    gBuzzerOn=0;       //Contador de activacion de buzzer
    gBuzzerCont=0;     //Contador para generar onda cuadrada
    gBuzzerT=1;        //Periodo inicial

    
    //Habilitamos interrpciones
    INTCONbits.T0IE=1;
    INTCONbits.PEIE=1;
    INTCONbits.GIE=1;
    
    //Chequeo de Hardware
    if(pPROG==0) {
        //LED ROJO 
            for(n=0;n<10;n++) {
                LEDROJO=1;
                Esperar(100);
                LEDROJO=0;
                Esperar(100);        
            }
        //LED ROJO 
            for(m=0;m<7;m++){
                for(n=0;n<10;n++) {
                    CambiarPWM(10);
                    Esperar(20);
                }
                for(n=0;n<10;n++) {
                    CambiarPWM(-10);
                    Esperar(20);    
                }
            }
         //LED VERDE 
            for(n=0;n<10;n++) {
                LEDVERDE=1;
                Esperar(100);
                LEDVERDE=0;
                Esperar(100);      
            }   

        //Display L
            gDigL=0xFF;
            Esperar(1500);
            gDigL=0x00;

         //Display M
            gDigM=0xFF;
            Esperar(1500);
            gDigM=0x00;

        //Display H
            gDigH=0xFF;
            Esperar(1500);
            gDigH=0x00;

        //Buzzer
            gBuzzerT=1;         //Tono
            gBuzzerOn=500;      //Tiempo 
            while(gBuzzerOn>0);
            gBuzzerT=2;         //Tono
            gBuzzerOn=500;      //Tiempo 
            while(gBuzzerOn>0);
            gBuzzerT=4;         //Tono
            gBuzzerOn=500;      //Tiempo 
            while(gBuzzerOn>0);
            gBuzzerT=7;         //Tono
            gBuzzerOn=500;      //Tiempo 
            while(gBuzzerOn>0);
            gBuzzerT=10;         //Tono
            gBuzzerOn=500;      //Tiempo 
            while(gBuzzerOn>0);   
    }
        
    //Bucle principal
    while(1)
    {

        // Conversion ADC
        if((ADCON0bits.GO_nDONE==0)&&(gContadorADC==0)) {
            Hex2Decimal(ADRESH);
            gDigL|=0x80;        //Punto en menos significativo
            gContadorADC=500;   //Esperamos 500 TMR0 para repetir
            ADCON0bits.GO_nDONE=1;
            
        }
        //Recepcion USART
        if(RCIF) {
            Hex2Decimal(RCREG);
            gContadorADC=3000;  //Ignoramos ADC por 3000 TMR0
            gDigL|=0x80;        //Punto en menos significativo
            gDigM|=0x80;        //Punto en menos significativo
            gDigH|=0x80;        //Punto en menos significativo
            gBuzzerT=dTonoRX;         //Tono
            gBuzzerOn=100;      //Tiempo            
            
        }   
        
        //Chequeo Teclado
        if(gContadorAR==0) {   //Si no estamos esperando antirrebote
            AuxByte=PORTB;
            if(AuxByte&0x3F) {   
                gContadorAR=500;   //Ignoramos teclas por 500 TMR0
                gContadorADC=3000; //Ignormaos ADC por 3000 TMR0
                switch (gEstadoTeclado) {
                    case dCol1:
                        if(AuxByte&dFil1)
                            TeclaP=1;
                        if(AuxByte&dFil2)
                            TeclaP=4;
                        if(AuxByte&dFil3)
                            TeclaP=7;
                        if(AuxByte&dFil4)
                            TeclaP=dENT;                        
                        break;
                     case dCol2:
                        if(AuxByte&dFil1)
                            TeclaP=2;
                        if(AuxByte&dFil2)
                            TeclaP=5;
                        if(AuxByte&dFil3)
                            TeclaP=8;
                        if(AuxByte&dFil4)
                            TeclaP=0;  
                        break;
                    case dCol3:
                        if(AuxByte&dFil1)
                            TeclaP=3;
                        if(AuxByte&dFil2)
                            TeclaP=6;
                        if(AuxByte&dFil3)
                            TeclaP=9;
                        if(AuxByte&dFil4)
                            TeclaP=dESC;  
                        break;
                    case dCol4:
                        if(AuxByte&dFil1)
                            TeclaP=10;
                        if(AuxByte&dFil2)
                            TeclaP=11;
                        if(AuxByte&dFil3)
                            TeclaP=12;
                        if(AuxByte&dFil4)
                            TeclaP=13;  
                        break;                    
                    } 
                if(tABAJO)
                    TeclaP=dABAJO;
                if(tARRIBA)
                    TeclaP=dARRIBA;    
                //Actualizamos teclado
                DisplayTecla(TeclaP,dTonoTecla);
            }
            
            
        }
    
    }
            
    return;
}
