/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
                                                    // Declaración de Constantes y Variables Generales
uint8 estado = 0;                                  
uint16 retardo = 0;
uint8 posicion = 0;
uint8 aciertos = 0;
uint8 segundos = 0;
unsigned char estadoLeds = 0b00000000;

uint8 claveOriginal[4] = {2,5,0,6};                  // Variables de acceso al sistema
uint8 claveIngresada[4] = {0,0,0,0};

                                                    // Declaración de Interrupciones
CY_ISR_PROTO(AjustarDigito);
CY_ISR_PROTO(Confirmar);
CY_ISR_PROTO(Salir);
CY_ISR_PROTO(Cronometro_1_segundo);

                                                    // Declaración de Funciones
void titilar(void);
void restablecerBrillo(void);
void comparar(const uint8 *ingresada, const uint8 *original);
void rutinas(void);
void reiniciar(void);

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    isr_ajustarDigito_StartEx(AjustarDigito);                       // Inicializacion de Interrupciones
    isr_confirmar_StartEx(Confirmar);
    isr_salir_StartEx(Salir);
    isr_1_segundo_StartEx(Cronometro_1_segundo);
    
    Displays_Start();                                               // Inicializacion de Componentes
    Displays_Write7SegNumberDec(0,0,4,Displays_ZERO_PAD);
    
    for(;;)
    { 
        switch(estado){
            case 0:                                                 // Titilar displays en ceros
                titilar();
                break;
            case 1:                                                 // Ingresar Clave
                titilar();
                break;
            case 2:                                                 // Comparar Clave Ingresada VS Clave Original
                comparar(&claveIngresada[0],&claveOriginal[0]);
                break;
            case 3:                                                 // Ejecucion de tareas
                rutinas();
                break;
        } 
    }
}

// Definición de Funciones

void reiniciar(void){                                   // Reinicia las variables del sistema, los perifericos y las tareas
    Leds_Write(0b00000000);
    Displays_Write7SegNumberDec(0,0,4,Displays_ZERO_PAD);
    restablecerBrillo();
    for (uint8 i=0; i<4; i++){
        claveIngresada[i]=0;
    }
    estado = 0;                                         // Ademas pone el sistema en el estado inicial
    posicion=0;                                  
    aciertos = 0;
    segundos = 0;
}

void rutinas(void){                              // Ejecuta las tareas de acuerdo al estado del dip_switch
    uint16 velocidad;
    
    switch(Tarea_Read()){
        case 0:                                  // Tarea 0: Juego de leds y corrimiento de letra A en los displays
            velocidad=500;                       // Retardo que permite una frecuencia de 2 Hz y el condicional 
                                                 // permite validar que la accion se realice en el estado de tareas (3) 
            estadoLeds = 0b00000000;    if (estado!=3 || Tarea_Read()!=0) return;
            Leds_Write(estadoLeds);     Displays_WriteString7Seg("A   ",0);     CyDelay(velocidad);  
            estadoLeds = 0b00011000;    if (estado!=3 || Tarea_Read()!=0) return;   
            Leds_Write(estadoLeds);     Displays_WriteString7Seg(" A  ",0);     CyDelay(velocidad);
            estadoLeds = 0b00111100;    if (estado!=3 || Tarea_Read()!=0) return;
            Leds_Write(estadoLeds);     Displays_WriteString7Seg("  A ",0);     CyDelay(velocidad);     
            estadoLeds = 0b01111110;    if (estado!=3 || Tarea_Read()!=0) return;
            Leds_Write(estadoLeds);     Displays_WriteString7Seg("   A",0);     CyDelay(velocidad);     
            estadoLeds = 0b11111111;    if (estado!=3 || Tarea_Read()!=0) return;
            Leds_Write(estadoLeds);     Displays_WriteString7Seg("   A",0);     CyDelay(velocidad);     
            estadoLeds = 0b11100111;    if (estado!=3 || Tarea_Read()!=0) return; 
            Leds_Write(estadoLeds);     Displays_WriteString7Seg("  A ",0);     CyDelay(velocidad);     
            estadoLeds = 0b11000011;    if (estado!=3 || Tarea_Read()!=0) return; 
            Leds_Write(estadoLeds);     Displays_WriteString7Seg(" A  ",0);     CyDelay(velocidad);     
            estadoLeds = 0b10000001;    if (estado!=3 || Tarea_Read()!=0) return; 
            Leds_Write(estadoLeds);     Displays_WriteString7Seg("A   ",0);     CyDelay(velocidad);    
            break;
            
        case 1:                                                             // Tarea 1: Control de brillo de los displays
            Leds_Write(0b00000000);                                         // Reinicio de leds y displays7seg a 0
            Displays_Write7SegNumberDec(1234,0,4,Displays_ZERO_PAD);        
            uint8 brillo[7]={255, 100, 30, 0, 30, 100, 255};                // Se recorre el vector de brillos en el orden determinado.
            
            velocidad = 1667;                                               // Con un retardo especifico
                                                                            // para que los displays se apaguen en 5 segundos
            for (uint8 i=0; i<7; i++){                                      // y vuelvan a encenderse en otros 5 segundos
                if (estado!=3) return;
                if (i == 3) velocidad=2300;
                if (estado!=3 || Tarea_Read()!=1) return;
                for (uint8 posicion_display = 0; posicion_display < 5; posicion_display++){
                    if (estado!=3 || Tarea_Read()!=1) return;
                    Displays_SetBrightness(brillo[i],posicion_display);
                }
                CyDelay(velocidad);
            }
    }
}

void comparar(const uint8 *ingresada , const uint8 *original){            // Compara las claves ingresada y original 
    aciertos=0;
    for (uint8 i = 0; i <4; i++){     
        if (*ingresada == *original) aciertos++;            // Cuenta los digitos correctos
        ingresada++;                                        // Aumenta la posicion del digito a validar 
        original++;                                         // en los vectores clave ingresada y original
    }  
    
    if (aciertos == 4){
        Displays_WriteString7Seg("   ",0);             // Si la Clave Correcta, se escribe la letra C en el display final
        Displays_Write7SegDigitHex(12,3);              // y parpadea durante 3 segundos, (La letra C mayuscula es el 12
                                                       // en Hexadecimal
        ContadorSegundos_Start();                      // Se inicia el contador para que registre 3 segundos                       
        while (segundos < 3){
            titilar();
        }                                              // El tiempo de parpadeo lo define un contador de segundos
        restablecerBrillo();
        ContadorSegundos_Stop();
        segundos = 0;
        estado = 3;
    }
    
    if (aciertos != 4) {
        Displays_WriteString7Seg("   F",0);             // Si la Clave es Falsa, se escribe la letra F en el display final
        ContadorSegundos_Start();                       // y parpadea durante 2 segundos
        while (segundos < 2){                           // Se inicia el contador para que registre 2 segundos  
            titilar();
        }
        restablecerBrillo();
        ContadorSegundos_Stop();                        // El tiempo de parpadeo lo define un contador de segundos
        segundos = 0;
        reiniciar();
    }   
}

void restablecerBrillo(void){                                       // Pone alto el brillo de los displays
    for (uint8 posicion_display = 0; posicion_display < 5; posicion_display++)
        {
            Displays_SetBrightness(200,posicion_display);
        }
}

void titilar(void){                                             // MECANISMO: Brillo a 0 -> Delay -> Brillo a 200 -> Delay
    retardo=400;
    if (estado==0 || estado==2){                                // Si el estado del sistema es 0, todos los displays titilan
        for (uint8 posicion_display = 0; posicion_display < 5; posicion_display++){
            Displays_SetBrightness(200,posicion_display);
        }
        CyDelay(retardo);                                       // delay de 333 milisegundos
        for (uint8 posicion_display = 0; posicion_display < 5; posicion_display++){
            Displays_SetBrightness(0,posicion_display);
        }
        CyDelay(retardo);                                       // delay de 333 milisegundos
        if (estado==1) restablecerBrillo();                     // Esta linea evita que si cambia el estado estando
    }                                                           // en este bucle, no se apaguen los displays
    
    if (estado==1){                                             // Si el estado del sistema es 1, 
        Displays_SetBrightness(200,posicion);                     // solo titila el display de la posicion actual
        CyDelay(retardo);
        Displays_SetBrightness(0,posicion);
        CyDelay(retardo);
    }
}

// Definición de Interrupciones
CY_ISR(AjustarDigito){
    if (estado==0) estado=1;                             // Esta linea evita que todos los leds parpadeen cambiando al estado 1
    if (posicion < 4){
        claveIngresada[posicion]++;                      // Aumenta en 1 el digito cada que se presiona el boton
        if (claveIngresada[posicion]==10) claveIngresada[posicion]=0;       // Cuando el digito llegue a 10, se reinicia a 0
        Displays_Write7SegDigitDec(claveIngresada[posicion],posicion);      // Escribe en el display cada que se presiona el boton
    }
}

CY_ISR(Confirmar){
    if (estado==0) estado=1;                    // Esta linea evita que todos los leds parpadeen cambiando al estado 1
    if (posicion < 4){                          // Mientras el sistema continue en el estado 1, se restablece el brillo 
        restablecerBrillo();                    // y continua con el siguiente digito
        posicion++;
    }
    if (posicion==4) estado=2;                  // Si se terminan de ingresar los 4 digitos, el sistema cambia al estado 2
}

CY_ISR(Salir){                                  // Reinicia el sistema al estado 0 (inicial)
    reiniciar();
}

CY_ISR(Cronometro_1_segundo){                   // Contador de segundos
    segundos++;
}

/* [] END OF FILE */
