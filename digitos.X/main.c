#include <xc.h>

void main(void)
{
    OSCCONbits.IRCF2 = 0; //De 4MHz a 250kHz
    OPTION_REGbits.PS1 = 0; //Factor de escala de 256 a 64
    OPTION_REGbits.PSA = 0; //Usar preescalador
    OPTION_REGbits.T0CS = 0; //Activar el contar pulsos de oscilador principal
    
    TRISD = 0x00; //Puerto D como salida
    TRISB = 0xf0; //Puerto B<3:0> como salida
    ANSELH = 0; //Desactivar convertidor analogico
    unsigned char index = 0, data[3] = {0, 0, 0}, count = 0;
    static unsigned char const digits[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x4b, 0x7f, 0x6f};
    static unsigned char const dispSel[3] = {0xE, 0xD, 0xB};
    
    while(1)
    {
        PORTD = digits[data[index]];
        PORTB = dispSel[index];
        //Retardo
        while(INTCONbits.T0IF != 1)
            ;
        TMR0 = 12;
        INTCONbits.T0IF = 0;
        
        if(index < 2)
            index++;
        else
            index = 0;
        if(++count == 3)
        {
            count = 0;
            if(++data[2] == 10)
            {
                data[2] = 0;
                if(++data[1] == 10)
                {
                    data[1] = 0;
                    if(++data[0] == 10)
                        data[0] = 0;
                }
            }
        }
    }
}
