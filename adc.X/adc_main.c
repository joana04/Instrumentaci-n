#include <xc.h>
#include <stdlib.h>

#pragma config FOSC=INTRC_NOCLKOUT
#pragma config WDTE=OFF
#pragma config IESO=OFF
#pragma config FCMEN=OFF

//ADC
void configTimer(void);
void configADC(void);
void configUART(void);
void waitTimer(void);
void wait10Timers(void);
void runADC(void);
void sendADC(void);
void printADC(float value);

//LCD
void confBusLCD(void);
void comando(unsigned int com);
void dato(unsigned int dat);
void defineChar(void);
void configLCD(void);

void main(void) {    
    configTimer();
    configADC();
    configUART();
    configLCD();
    
    while(1) {
        waitTimer(); //Un segundo
//        wait10Timers(); //Diez segundos
        runADC();
        sendADC();
    }
}

void configTimer(void) {
    OSCCONbits.IRCF2 = 0; //De 4MHz a 250kHz
    OPTION_REGbits.PSA = 0; //Usar preescalador 250kHz/4 = 62500Hz; 62500Hz/256=244.1406Hz
    OPTION_REGbits.T0CS = 0; //Activar el contar pulsos de oscilador principal
    INTCONbits.T0IF = 0;
    TMR0 = 99; //Se tuvo que ajustar para que fuera exactamente un segundo. Contar solo 244 veces, 244.1406Hz/244 = 1.00057623Hz
}

void configADC(void) {
    TRISB=0XFF;
    ADCON0bits.ADON = 1; // Convertidor Enable 
    ADCON0bits.CHS3 = 1; // Pin B2
    ADCON0bits.ADCS1 = 0; // Iteracion de la conversion a FOSC/32
    ADCON1bits.ADFM = 0; // Justificado a la izquierda
    ANSEL= 0;
    ANSELH = 0;
    ADRESL = 0;
}

void configUART(void) {
    RCSTAbits.SPEN = 1;
    RCSTAbits.CREN = 1;
    TXSTAbits.BRGH = 1;
    TXSTAbits.TXEN = 1;
    PIR1bits.ADIF=0;
}

void waitTimer(void) {
    while(INTCONbits.T0IF != 1)
        ;
    TMR0 = 99; //Se tuvo que ajustar para que fuera exactamente un segundo. Contar solo 244 veces, 244.1406Hz/244 = 1.00057623Hz
    INTCONbits.T0IF = 0;
}

void wait10Timers(void) {
    unsigned char i;
    for (i = 0; i < 10; i++)
        waitTimer();
}

void runADC(void) {
    ADCON0bits.GO = 1;
    while(PIR1bits.ADIF != 1)
        ;
    PIR1bits.ADIF = 0;
}

void sendADC(void) {
    unsigned char parteB, parteA, aux;
    unsigned short int medicion;
    float converted;
    parteA = ADRESH;
    parteB = ADRESL;
        
    aux = parteA & 0x07; //Se guardan los 3 bits menos signifcativos de parte alta
    aux = aux << 2; //Se guarda espacio para los 2 bits de la parte baja
        
    parteA = parteA >> 3; //Se dejan solo 5 bits en parte alta
    parteA = parteA | 0x80; //Se enciende bandera de que es parte alta
        
    parteB = parteB >> 6; //Se recorren bits al lado derecho
    parteB = parteB | aux; //Se junta parte baja con los 3 bits de parte alta
        
    TXREG = parteB;
    TXREG = parteA;
    
    parteA = parteA & 0x1F;
    medicion = (((short)parteA) << 5) | parteB;
    converted = (float)(5 * medicion) / 1023;
    printADC(converted);
}

void printADC(float value)
{
    char * buf;
    unsigned char j;
    int i;
    float temperature = (value*50)/5; //Suponiendo que 50C = 5V y 0C = 0V
    buf = ftoa(temperature, i);
    comando(0xC5); // Apunta a la primera columna del segundo renglon
    if (temperature < 10)
        dato('0');
    for (i = 0; buf[i] != '.'; i++)
        dato(buf[i]);
    for (j = i; j < i + 3; j++)
        dato(buf[j]);
    dato(0);
    dato('C');
}

void defineChar()
{
    unsigned char t, i, data[8] = {0x0F, 0x09, 0x09, 0x0F, 0x00, 0x00, 0x00, 0x00}; //Grado
    comando(0x40);
    for(i = 0; i < 8; i++)
        dato(data[i]);
}

void confBusLCD(void)
{
 int t, estado;
 
 // Asigné ceros a t para producir retardos pequeños.
 PORTEbits.RE0=0; t=0;  // RS indica comando.
 PORTEbits.RE1=1; t=0;  // RW indica leer del LCD.
 do
 {
  PORTEbits.RE2=1; t=0; // Sube el habilitador E.
  estado=(PORTD&0x80);  // Lee bandera de ocupado (BF).
  PORTEbits.RE2=0; t=0; // Baja el habilitador E.
 } while (estado!=0);   // Espera a que el LCD esté desocupado.
 PORTEbits.RE1=0;       // RW indica escribir al LCD.
 TRISD=0x0F;            // RD7 a RD3 como salidas.
 PORTD=0x20;            // Ordena usar bus de 4 bits.
 PORTEbits.RE2=1; t=0;  // Sube el habilitador E.
 PORTEbits.RE2=0; t=0;  // Baja el habilitador E.
 TRISD=0xFF;            // Puerto D como entrada.
}

void comando(unsigned int com)
{
 int t, estado;
 
 // Asigné ceros a t para producir retardos pequeños.
 PORTEbits.RE0=0; t=0;  // RS indica comando.
 PORTEbits.RE1=1; t=0;  // RW indica leer del LCD.
 do
 {
  PORTEbits.RE2=1; t=0; // Sube el habilitador E.
  estado=(PORTD&0x80);  // Lee bandera de ocupado (BF).
  PORTEbits.RE2=0; t=0; // Baja el habilitador E.
  PORTEbits.RE2=1; t=0; // Sube el habilitador E.
  PORTEbits.RE2=0; t=0; // Baja el habilitador E.
 } while (estado!=0);   // Espera a que el LCD esté desocupado.
 PORTEbits.RE1=0;       // RW indica escribir al LCD.
 TRISD=0x0F;            // RD7 a RD3 como salidas.
 PORTD=com;
 PORTEbits.RE2=1; t=0;  // Sube el habilitador E.
 PORTEbits.RE2=0; t=0;  // Baja el habilitador E.
 PORTD=(com<<4);
 PORTEbits.RE2=1; t=0;  // Sube el habilitador E.
 PORTEbits.RE2=0; t=0;  // Baja el habilitador E.
 TRISD=0xFF;            // Puerto D como entrada.
}

void dato(unsigned int dat)
{
 int t, estado;
 
 // Asigné ceros a t para producir retardos pequeños.
 PORTEbits.RE0=0; t=0;  // RS indica comando.
 PORTEbits.RE1=1; t=0;  // RW indica leer del LCD.
 do
 {
  PORTEbits.RE2=1; t=0; // Sube el habilitador E.
  estado=(PORTD&0x80);  // Lee bandera de ocupado (BF).
  PORTEbits.RE2=0; t=0; // Baja el habilitador E.
  PORTEbits.RE2=1; t=0; // Sube el habilitador E.
  PORTEbits.RE2=0; t=0; // Baja el habilitador E.
 } while (estado!=0);   // Espera a que el LCD esté desocupado.
 PORTEbits.RE0=1; t=0;  // RS indica dato.
 PORTEbits.RE1=0;       // RW indica escribir al LCD.
 TRISD=0x0F;            // RD7 a RD3 como salidas.
 PORTD=dat;
 PORTEbits.RE2=1; t=0;  // Sube el habilitador E.
 PORTEbits.RE2=0; t=0;  // Baja el habilitador E.
 PORTD=(dat<<4);
 PORTEbits.RE2=1; t=0;  // Sube el habilitador E.
 PORTEbits.RE2=0; t=0;  // Baja el habilitador E.
 TRISD=0xFF;            // Puerto D como entrada.
}

void configLCD(void)
{
    unsigned char i, msg[] = "Temperatura";
    TRISE=0xF8;    // RE0, RE1 y RE2 como salidas.
    ANSEL=0;       // Terminales compartidas como digitales.
    confBusLCD();  // Configura el LCD para usar un bus de 4 bits.
    comando(0x0C); // Enciende el LCD sin cursor.
    comando(0x28); // 2 renglones, caracteres de 5 x 8, nuevamente bus de 4 bits.
    comando(0x01); // Limpia el exhibidor y pone las direcciones a 0.
    comando(0x82);
    for(i = 0; msg[i]; i++)
        dato(msg[i]);
    defineChar();
}