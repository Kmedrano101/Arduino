/*
    Proyecto: Dispensador de Agua
    Desarrollador: Bittron
    Fecha: 21/02/2022
    Descripcion general: None
*/

// Librerias y definiciones
#include <LiquidCrystal.h>
#include <EEPROM.h>
#define M1 2    // Entrada de Monedas
#define H1 3    // Salida de Monedas
#define D1 4    // Boton de Lavado
#define D2 5    // Boton de Llenado
#define B1 6    // Menu Config 
#define B2 7    // Incrementar Valor
#define B3 8    // Decrementar Valor
#define S1 10   // EV Lavado 
#define S2 11   // EV Llenado
#define S3 12   // Motor Hooper
#define S4 13   // Motor Agua


// Variables
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5); // (RS, EN, D4, D5, D6, D7)
int CONTADOR = 0;
int AUX_CONTADOR = 0;
int AUX_INCREMENTAR = 0;
int AUX_DECREMENTAR = 0;
int OPCION = 1;

struct Dispenser_Values{
  int Price=12;       // Price Max Bottle
  unsigned int Times[12];   // 0 - 11 -> Tiempos de llenado Agua, Guarda valor el segundos
  volatile unsigned int Current_Money; 
  int status;
};
Dispenser_Values VALUES;
//VALUES.Price = 12;
int EE_ADDRESS = 0;
unsigned int TIEMPO_LAVADO = 180;
// Auxiliares
int Display = 1;
volatile int Cambio = 0;
unsigned int Seleccion = 0;
unsigned int SaldoCambio = 0;
unsigned int FlagDispensador = 0;
unsigned int FlagLavado = 0;
unsigned int FlagLavadoFinalizado = 0;
unsigned long StartTime = 0;
unsigned long StartTimeInactividad = 0;
unsigned long StartTimeLlenado = 0;

String CONF_MODE_L1 = "  CONFIC MODE   ";  
String CONF_MODE_L2 = " TIEMPO   :     "; // [Row: 1, Colum 8] [Row: 1, Colum 11]
String CONF_MODE_L3 = " PRECIO MAX:    ";
String CAD_SALDO = "   SALDO:  "; 
// Main
void setup()
{
    
    lcd.begin(16,2);
    pinMode(M1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(M1), ContadorSaldo, FALLING);
    pinMode(H1, INPUT);
    attachInterrupt(digitalPinToInterrupt(H1), ContadorCambio, RISING);
    pinMode(B1, INPUT_PULLUP);
    pinMode(B2, INPUT_PULLUP);
    pinMode(B3, INPUT_PULLUP);
    pinMode(D1, INPUT_PULLUP);
    pinMode(D2, INPUT_PULLUP);
    pinMode(S1, OUTPUT);
    digitalWrite(S1,0);
    pinMode(S2, OUTPUT);
    digitalWrite(S2,0);
    pinMode(S3, OUTPUT);
    digitalWrite(S3,0);
    pinMode(S4, OUTPUT);
    digitalWrite(S4,0);
    lcd.print("BIENVENIDO"); // Mensaje de Bienvenida del Sistema
    lcd.print(VALUES.Price);
  	GetValues();
    //VALUES.Price = 12;
  	delay(1200);  
  	// TEST 
    //VALUES.Times[0] = 2;
    //VALUES.Times[1] = 3;
    //VALUES.Times[2] = 2;
  	//VALUES.Times[3] = 3;
    //VALUES.Times[4] = 2;
    //VALUES.Times[5] = 3;
  	//VALUES.Times[6] = 2;
    //VALUES.Times[7] = 3;
    //VALUES.Times[8] = 3;
  	//VALUES.Times[9] = 12;
    //VALUES.Times[10] = 12;
    //VALUES.Times[11] = 12;
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
void RetornoCambio(){
    // Modo Cambio
    if (SaldoCambio>= 1)
    {
        digitalWrite(S3,1);
        Inactividad();
    }
}
void Menus(){
    if(Display==1){
        CAD_SALDO = CAD_SALDO.substring(0,10)+ String(VALUES.Current_Money)+" $   ";
        ShowDisplay("INGRESAR MODENAS",CAD_SALDO);
    }
    else
    {
        ShowDisplay("  DESPACHANDO   ","  PRODUCTO ...  ");
    }
}
void Dispensador(){
    if (FlagDispensador >= 1)
    {
        digitalWrite(S2,1); // Activar  EV llenado
        digitalWrite(S4,1); // Activar Motor Agua
        Display = 2;
        if(millis() > (StartTime + (VALUES.Times[FlagDispensador-1]*1000))){ // Se multiplica por 1000 para pasar Segundos a Milisegundos
            digitalWrite(S2,0); // Apagar EV llenado
            digitalWrite(S4,0);  // Apagar Motor Agua
            FlagDispensador = 0;
            Display = 1;
            FlagLavadoFinalizado = 0;
            //StartTime = millis();
            //VALUES.Current_Money = 0;
        }
    }
    if (FlagLavado == 1 && FlagLavadoFinalizado == 0)
    {
        digitalWrite(S1,1); // Activar  EV lavado
        digitalWrite(S4,1); // Activar Motor Agua
        if(millis() > (StartTime + TIEMPO_LAVADO)){
            digitalWrite(S1,0); // Apagar EV lavado
            digitalWrite(S4,0);  // Apagar Motor Agua
            FlagLavado = 0;
            FlagLavadoFinalizado = 1;
        }
    }
}
void SeleccionDispensador(){
    if (millis() > StartTimeLlenado + 1800) // Tiempo de espera minimo antes de lavado o dispensado
    {
        if (!digitalRead(D1)) // Caso de Lavado
        {
            if (VALUES.Current_Money >= 1)
            {
                // Dispensar para Lavado
                FlagLavado = 1;
                StartTime = millis();
            }
        }

        if (!digitalRead(D2)) // Caso de Dispensado
        {
            if ((VALUES.Current_Money >= 1 && VALUES.Current_Money <= VALUES.Price) && FlagDispensador == 0)
            {
                // Dispensar Agua
                FlagDispensador = VALUES.Current_Money;
                VALUES.Current_Money = 0;
                StartTime = millis();
            }
            if ((VALUES.Current_Money > VALUES.Price) && FlagDispensador == 0)
            {
                SaldoCambio = VALUES.Current_Money - VALUES.Price;
                VALUES.Current_Money = SaldoCambio;
                FlagDispensador = sizeof(VALUES.Price); // En caso de Saldo Actual mayor a doce se activa temp 12(vector 11)
                StartTime = millis();
                StartTimeInactividad = millis();
            }
        }
            
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
void SaveValues(){
    EEPROM.put(EE_ADDRESS, VALUES);
}
void GetValues(){
    EEPROM.get(0, VALUES);
}
void ConfigMode(){
    if(!digitalRead(B1))
    {
        while (CONTADOR<=14)
        {
            if (!digitalRead(B1))
            {
                 AUX_CONTADOR = 1;
            }
            if (AUX_CONTADOR == 1 && digitalRead(B1))
            {
                CONTADOR += 1;
                AUX_CONTADOR = 0;
            }
            if (!digitalRead(B2))
            {
                 AUX_INCREMENTAR = 1;
            }
            if (AUX_INCREMENTAR == 1 && digitalRead(B2))
            {
                if (CONTADOR<=12)
                {
                    VALUES.Times[CONTADOR-1] += 1;        
                }
                else
                {
                    VALUES.Price += 1;      
                }
                AUX_INCREMENTAR = 0;
            }
            if (!digitalRead(B3))
            {   
                AUX_DECREMENTAR = 1;
            }
            if (AUX_DECREMENTAR == 1 && digitalRead(B3))
            {
                if (CONTADOR<=12)
                {
                    VALUES.Times[CONTADOR-1] -= 1;
                }
                else
                {
                    VALUES.Price -= 1; 
                }
                AUX_DECREMENTAR = 0;
            }
            if(CONTADOR<=12)
            {
                if(CONTADOR<=9){CONF_MODE_L2[8] = (char)CONTADOR+48;}
                if(CONTADOR>=10 && CONTADOR <=12)
                {
                    String SPlit = (String)CONTADOR;
                    CONF_MODE_L2[8] = SPlit[0];
                    CONF_MODE_L2[9] = SPlit[1];
                }
                CONF_MODE_L2 = CONF_MODE_L2.substring(0,12)+ String(VALUES.Times[CONTADOR-1])+"    "; 
                ShowDisplay(CONF_MODE_L1,CONF_MODE_L2);   
            }
            if(CONTADOR>=13)
            {
                CONF_MODE_L3 = CONF_MODE_L3.substring(0,13)+ String(VALUES.Price)+"    ";
                ShowDisplay(CONF_MODE_L1,CONF_MODE_L3);
                if(CONTADOR==14){break;}
            }
                  
        }
        CONTADOR = 0;
        CONF_MODE_L2 = " TIEMPO   :     ";
        CONF_MODE_L3 = " PRECIO MAX:    ";
        ClearDisplay();
        ShowDisplay("GUARDANDO","DATOS");
        SaveValues();
        delay(3000); // Guardar a EEPROM
        ClearDisplay();
    }
}
void ContadorCambio(){
    Cambio++;
    VALUES.Current_Money -= 1;
    if (Cambio >= SaldoCambio) 
    {
        digitalWrite(S3,0);
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
        digitalWrite(S3,0);
    }
}