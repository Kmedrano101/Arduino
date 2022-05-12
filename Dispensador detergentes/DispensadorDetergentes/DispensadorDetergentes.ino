 /* Detalles generales
    Proyecto: Dispensador de productos detergentes
    Desarrollador: Bittron
    Fecha: 12/05/2022
    Descripcion general: 10 productos, 10 precios, 10 tiempos de bombeo
*/

// Librerias y definiciones
#include <LiquidCrystal.h>
#include <EEPROM.h>
#define M1 2     // Entrada de Monedas Hooper
#define H1 3     // Salida de Monedas Hooper
#define D1 4     // Entrada selector detergente 1
#define D2 5     // Entrada selector detergente 2
#define D3 6     // Entrada selector detergente 3
#define D4 7     // Entrada selector detergente 4
#define D5 8     // Entrada selector detergente 5
#define D6 9     // Entrada selector detergente 6
#define D7 10    // Entrada selector detergente 7 
#define D8 11    // Entrada selector detergente 8
#define D9 12    // Entrada selector detergente 9
#define D10 13   // Entrada selector detergente 10
#define B1 22    // Salida bomba detergente 1
#define B2 23    // Salida bomba detergente 2
#define B3 24    // Salida bomba detergente 3
#define B4 25    // Salida bomba detergente 4
#define B5 26    // Salida bomba detergente 5
#define B6 27    // Salida bomba detergente 6
#define B7 28    // Salida bomba detergente 7 
#define B8 29    // Salida bomba detergente 8
#define B9 30    // Salida bomba detergente 9
#define B10 31   // Salida bomba detergente 10
#define C1 34    // Menu Config 
#define C2 36    // Incrementar Valor
#define C3 38    // Decrementar 
#define S1 40    // Motor Hooper

LiquidCrystal lcd(A0,A1,A2,A3,A4,A5); 
int CONTADOR = 0;
int AUX_CONTADOR = 0;
int AUX_INCREMENTAR = 0;
int AUX_DECREMENTAR = 0;
int CONFIG_OPCION = 1;
// Antirrebote para entradas con interrupts
unsigned long TH_ISR = 150;
unsigned long TH_LAST_TIME_IN = 0;
unsigned long TH_LAST_TIME_OUT = 0;

struct Dispenser_Values{
  unsigned int Prices[10]= {2,4,6,8,10,2,4,6,8,4};       // 0 - 11 -> Precios detergentes
  unsigned int Times[10] = {2,3,9,15,19,23,26,30,34,38};   // 0 - 11 -> Tiempos bombeo, Guarda valor el segundos
  volatile unsigned int Current_Money = 0; 
  int status;
};
Dispenser_Values VALUES;

int EE_ADDRESS = 0;
// Auxiliares
int Display = 1;
volatile int Cambio = 0;
unsigned int Seleccion = 0;
unsigned int SaldoCambio = 0;
int FlagDispensador = -1;
unsigned int FlagLavado = 0;
unsigned int FlagLavadoFinalizado = 0;
unsigned long StartTime = 0;
unsigned long StartTimeInactividad = 0;
unsigned long StartTimeLlenado = 0;

String CONF_MODE_L1 = "  CONFIC MODE   ";  
String CONF_MODE_L2 = " TIEMPO   :     "; // [Row: 1, Colum 8] [Row: 1, Colum 11]
String CONF_MODE_L3 = " PRECIO   :     ";
String CAD_SALDO = "   SALDO:  ";
String CAD_INFO = "  FALTA SALDO  "; // Información predeterminado

#define DEBUGING // Quitar/Comentar esta directiva para salir modo DEGUB
#ifdef DEBUGING
#define DEBUG(x) \
  Serial.print("DEBUG: ");  \
  Serial.print(__LINE__); \
  Serial.print(' '); \
  Serial.println(x);
#else
#define DEBUG(x)
#endif

// Main
void setup()
{
    Serial.begin(9600);
    lcd.begin(16,2);
    pinMode(M1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(M1), ContadorSaldo, FALLING);
    pinMode(H1, INPUT);
    attachInterrupt(digitalPinToInterrupt(H1), ContadorCambio, RISING);
    for (int pin = 4; pin <= 13; pin++)
    {
        pinMode(pin, INPUT_PULLUP);
        pinMode(pin+18, OUTPUT);
        digitalWrite(pin+18, 0);
    }
    pinMode(C1, INPUT_PULLUP);
    pinMode(C2, INPUT_PULLUP);
    pinMode(C3, INPUT_PULLUP);
    pinMode(S1, OUTPUT);
    lcd.print("BIENVENIDO"); // Mensaje de Bienvenida del Sistema
    //SaveValues(); // 
    GetValues();
    delay(1200);  
}
// Loop
void loop()
{
    Menus();
    SeleccionDispensador();
    RetornoCambio();
    Dispensador();
    ConfigMode();
}
// Metodos
void Menus(){
    if(Display==1){
        CAD_SALDO = CAD_SALDO.substring(0,8)+ " $  " +String(VALUES.Current_Money);
        ShowDisplay("INGRESAR MONEDAS",CAD_SALDO);
    }
    else
    {
        ShowDisplay("  DESPACHANDO   ","  PRODUCTO ...  ");
    }
}
void ShowDisplay(String L1, String L2){
    lcd.setCursor(0,0); 
    lcd.print(L1);
    lcd.setCursor(0,1);
    lcd.print(L2);  
}
void ClearDisplay(){
    String Cl = "                ";
    lcd.setCursor(0,0); 
    lcd.print(Cl);
    lcd.setCursor(0,1);
    lcd.print(Cl);
}
void RetornoCambio(){
    // Modo Cambio
    if (SaldoCambio>= 1)
    {
        digitalWrite(S1,1);
        Inactividad();
    }
}
void Dispensador(){
    if (FlagDispensador >= 0)
    {
        DEBUG("Listo para dispensar!");
        digitalWrite((B1+FlagDispensador),1); // Activar Motor Agua
        Display = 2;
        if(millis() > (StartTime + (VALUES.Times[FlagDispensador]*1000))){ // Se multiplica por 1000 para pasar Segundos a Milisegundos
            digitalWrite((B1+FlagDispensador),0); // Apagar EV llenado
            FlagDispensador = -1;
            Display = 1;
        }
    }
}
void validarPrecio(unsigned int valorPrecio, int producto)
{
    DEBUG("Validar Precio de Compra");
    if (VALUES.Current_Money >= valorPrecio && FlagDispensador == -1)
    {
        FlagDispensador = producto;
        if (VALUES.Current_Money > valorPrecio)
        {
            SaldoCambio = VALUES.Current_Money - valorPrecio;
            VALUES.Current_Money = SaldoCambio;
        }
        else
        {
            VALUES.Current_Money = 0;
        }
        StartTime = millis();
        StartTimeInactividad = millis();
    }
    else
    {
        /* Notificar mensaje a usuario de falta de saldo */
    }
    
}
void SeleccionDispensador(){
    if (millis() > StartTimeLlenado + 1800) // Tiempo de espera minimo antes dispensar detergente
    {
        if (!digitalRead(D1)) { validarPrecio(VALUES.Prices[0],0); }
        if (!digitalRead(D2)) { validarPrecio(VALUES.Prices[1],1); }
        if (!digitalRead(D3)) { validarPrecio(VALUES.Prices[2],2); }
        if (!digitalRead(D4)) { validarPrecio(VALUES.Prices[3],3); }
        if (!digitalRead(D5)) { validarPrecio(VALUES.Prices[4],4); }
        if (!digitalRead(D6)) { validarPrecio(VALUES.Prices[5],5); }
        if (!digitalRead(D7)) { validarPrecio(VALUES.Prices[6],6); }
        if (!digitalRead(D8)) { validarPrecio(VALUES.Prices[7],7); }
        if (!digitalRead(D9)) { validarPrecio(VALUES.Prices[8],8); }
        if (!digitalRead(D10)) { validarPrecio(VALUES.Prices[9],9); }
    }
}
void SaveValues(){
    EEPROM.put(EE_ADDRESS, VALUES);
}
void GetValues(){
    EEPROM.get(0, VALUES);
}
void ConfigMode(){
    if(!digitalRead(C1))
    {
        while (CONTADOR<=11)
        {
            if (!digitalRead(C1))
            {
                 AUX_CONTADOR = 1;
            }
            if (AUX_CONTADOR == 1 && digitalRead(C1))
            {
                CONTADOR += 1;
                AUX_CONTADOR = 0;
            }
            if (!digitalRead(C2)){ AUX_INCREMENTAR = 1;}
            if (AUX_INCREMENTAR == 1 && digitalRead(C2))
            {
                if (CONFIG_OPCION==1)
                {
                    VALUES.Prices[CONTADOR-1] += 1;        
                }
                else
                {
                    VALUES.Times[CONTADOR-1] += 1;        
                }
                AUX_INCREMENTAR = 0;
            }
            if (!digitalRead(C3)){ AUX_DECREMENTAR = 1;}
            if (AUX_DECREMENTAR == 1 && digitalRead(C3))
            {
                if (CONFIG_OPCION==1)
                {
                    VALUES.Prices[CONTADOR-1] -= 1;        
                }
                else
                {
                    VALUES.Times[CONTADOR-1] -= 1;        
                }
                AUX_DECREMENTAR = 0;
            }
            if(CONTADOR<=9){CONF_MODE_L3[8] = (char)CONTADOR+48;CONF_MODE_L2[8] = (char)CONTADOR+48;}
            if(CONTADOR>=10)
            {
                String SPlit = (String)CONTADOR;
                if (CONFIG_OPCION==1)
                {
                    CONF_MODE_L3[8] = SPlit[0];
                    CONF_MODE_L3[9] = SPlit[1];
                }
                else
                {
                    CONF_MODE_L2[8] = SPlit[0];
                    CONF_MODE_L2[9] = SPlit[1];
                }
            }
            if(CONFIG_OPCION==1)
            {
                CONF_MODE_L3 = CONF_MODE_L3.substring(0,11)+ String(VALUES.Prices[CONTADOR-1])+"    "; 
                ShowDisplay(CONF_MODE_L1,CONF_MODE_L3);
                if(CONTADOR==11){CONTADOR = 1; CONFIG_OPCION = 2;}   
            }
            else
            {
                CONF_MODE_L2 = CONF_MODE_L2.substring(0,11)+ String(VALUES.Times[CONTADOR-1])+"    ";
                ShowDisplay(CONF_MODE_L1,CONF_MODE_L2);
                if(CONTADOR>=11){CONFIG_OPCION=1;break;}
            }
                  
        }
        CONTADOR = 0;
        CONF_MODE_L2 = " TIEMPO   :     ";
        CONF_MODE_L3 = " PRECIO   :     ";
        ClearDisplay();
        ShowDisplay("GUARDANDO","DATOS...");
        SaveValues(); // Guardar a EEPROM
        delay(2000); 
        ClearDisplay();
    }
}
void ContadorCambio(){
    if((millis()-TH_LAST_TIME_OUT) > TH_ISR){
        // Activar flags
        Cambio++;
        VALUES.Current_Money -= 1; 
        TH_LAST_TIME_OUT = millis();
    }
    if (Cambio >= SaldoCambio) 
    {
        digitalWrite(S1,0);
        Cambio = 0;
        SaldoCambio = 0;
        VALUES.Current_Money = 0;
    }
    StartTimeInactividad = millis();
}
void ContadorSaldo(){
    VALUES.Current_Money++;
    StartTimeLlenado = millis();
}
void Inactividad(){
    if(millis() > (StartTimeInactividad + 3000))
    { 
        digitalWrite(S1,0);
    }
}