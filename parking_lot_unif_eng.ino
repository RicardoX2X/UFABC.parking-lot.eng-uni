#include <SPI.h> //INCLUSÃO DE BIBLIOTECA
#include <MFRC522.h> //INCLUSÃO DE BIBLIOTECA
#include <EEPROM.h>
#include <Wire.h> // Biblioteca utilizada para fazer a comunicação com o I2C
#include <LiquidCrystal_I2C.h> // Biblioteca utilizada para fazer a comunicação com o display 20x4
#include <Adafruit_NeoPixel.h>

#define SS_PIN 10 //PINO SDA
#define RST_PIN A0 //PINO DE RESET
#define LED_PIN A1 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 8
#define coluna 16 // Serve para definir o numero de colunas do display utilizado
#define linha  2 // Serve para definir o numero de linhas do display utilizado
#define ender  0x27 // Serve para definir o endereço do display.

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
LiquidCrystal_I2C lcd(ender,coluna,linha); // Chamada da funcação Liqui
Adafruit_NeoPixel strip(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

int cont = 0;
uint32_t millisTarefa1 = 0;
uint32_t tempo_tarefa1 = 10000;
int inMin = 2; // Lowest input pin
int inMax = 10; // Highest input pin
uint32_t verde = strip.Color(0, 50, 0);
uint32_t vermelho = strip.Color(50, 0, 0);
uint32_t azul = strip.Color(0, 50, 50);
uint32_t magenta = strip.Color(50, 0, 50);
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;
int incomingByte = 0;
const byte kNumberOfChannelsFromExcel = 6; 

const char kDelimiter = ',';
const int kSerialInterval = 50;
unsigned long serialPreviousTime; 

char* arr[kNumberOfChannelsFromExcel];

uint32_t status[8] = {verde,verde,verde,verde,verde,verde,verde,verde};
bool last_reading[8] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
bool reading[8] = {};

void setup() //Incia o display
{  
  lcd.init(); // Serve para iniciar a comunicação com o display já conectado
  lcd.backlight(); // Serve para ligar a luz do display
  lcd.clear(); // Serve para limpar a tela do display
  strip.begin();
  strip.show();
  Serial.begin(9600);
  while (!Serial);
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  for(int i=inMin; i<inMax; i++)
  {
     pinMode(i, INPUT_PULLUP);
  }

  pinMode(A3, OUTPUT);
  
  mensageminicial();
  for(int i=0; i<8; i++)
  {
    strip.setPixelColor(i, status[i]);
    strip.show();
  }
}
void loop() 
{  
  for(int i=inMin; i<inMax; i++)
  {
    int button_index = 9-i;
    reading[button_index] = digitalRead(i);
    if (reading[button_index] != last_reading[button_index])
    {      
      last_reading[button_index]=reading[button_index];
      acende_led(button_index);
    }
  }	
  
  if ((millis() - millisTarefa1) > tempo_tarefa1)
  {
    mensageminicial();
  	millisTarefa1 = millis();
  }
  String teste = "";
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) // Look for new cards
  {
  }
  else
  {
    teste = le_o_modulo_RFID();
    Serial.print(teste);
    //lcd.setCursor(0, 1); //Coloca o cursor do display na coluna 1 e linha 2
    //lcd.print(teste);  // Comando de saida com a mensagem que deve aparecer na coluna 2 e linha 2
    if(teste == " 4B 47 F3 3E"){
      lcd.clear();
      lcd.setCursor(4, 0); //Coloca o cursor do display na coluna 1 e linha 2
      lcd.print("BEM VINDO:");
      lcd.setCursor(4, 1); //Coloca o cursor do display na coluna 1 e linha 2
      lcd.print("RICARDO");
      Serial.print(kDelimiter);
      Serial.println("BEM VINDO RICARDO");
    }
    else if(teste == " 67 70 E7 F3" ){
      lcd.clear();
      lcd.setCursor(4, 0); //Coloca o cursor do display na coluna 1 e linha 2
      lcd.print("BEM VINDO:");
      lcd.setCursor(4, 1); //Coloca o cursor do display na coluna 1 e linha 2
      lcd.print("BEATRIZ");
      Serial.print(kDelimiter);
      Serial.println("BEM VINDO BEATRIZ");
    }
    else {
      lcd.clear();
      lcd.setCursor(1, 0); //Coloca o cursor do display na coluna 1 e linha 2
      lcd.print("ACESSO NEGADO:");
      lcd.setCursor(4, 1); //Coloca o cursor do display na coluna 1 e linha 2
      lcd.print("REGINALDO");
      Serial.print(kDelimiter);
      Serial.println("ACESSO NEGADO REGINALDO");
    }
    mfrc522.PICC_HaltA();
  }
  
  le_a_serial();
  //Serial.println(teste);
  atualiza_vagas_lcd();
  processIncomingSerial();
  //processOutgoingSerial();
  //if (arr[0]!=""){
  //Serial.print(arr[0]);
  //}
  //if ( strcmp ("2", arr[0]) == 0){ 
  //    Serial.println("working");
  //}
}

void contador (int vaga)
{
  strip.clear();
  lcd.setCursor(15,1); // Coloca o cursor do display na coluna 1 e linha 1
  lcd.print(cont);// Comando de saída com a mensagem que deve aparecer na coluna 2 e linha 1.
  //Serial.print("ricardo\n");
  strip.setPixelColor(cont, 255, 0, 0);
  strip.setPixelColor(7-cont, azul);
  strip.show();
  cont++;
  if(cont>7)
  {
    cont=0;
  }
}

void acende_led (int vaga)
{
  if (status[vaga]==verde)
  {
    strip.setPixelColor(vaga, magenta);
    status[vaga]=magenta;
    strip.show();
    Serial.print(vaga);
    Serial.print(kDelimiter);
    Serial.println("ocupada sem reserva");
  }
  else if (status[vaga]==vermelho)
  {
    strip.setPixelColor(vaga, verde);
    status[vaga]=verde;
    strip.show();
    Serial.print(vaga);
    Serial.print(kDelimiter);
    Serial.println("livre");
  }
  else if (status[vaga]==azul)
  {
    status[vaga]=vermelho;
    strip.setPixelColor(vaga, status[vaga]);
    strip.show();
    Serial.print(vaga);
    Serial.print(kDelimiter);
    Serial.println("ocupada");
  }
  else if(status[vaga]==magenta)
  {
    status[vaga]=verde;
    strip.setPixelColor(vaga, status[vaga]);
    strip.show();
    Serial.print(vaga);
    Serial.print(kDelimiter);
    Serial.println("vaga ilegal liberada");
  }

}

void le_a_serial()
{
  if (Serial.available() > 0) 
  {
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    //Serial.print("I received: ");
    if (incomingByte>55 || incomingByte<48)
    {
      Serial.println("erro");
    }
    else
    {
      int vaga_serial = incomingByte-48;
      //Serial.println(vaga_serial, DEC);
      if (status[vaga_serial]==verde)
      {
        status[vaga_serial]=azul;
        strip.setPixelColor(vaga_serial, status[vaga_serial]);
        strip.show();
        Serial.print(vaga_serial);
        Serial.print(kDelimiter);
        Serial.println("reservada");
      }
      else if(status[vaga_serial]==azul)
      {
        status[vaga_serial]=verde;
        strip.setPixelColor(vaga_serial, status[vaga_serial]);
        strip.show();
        Serial.print(vaga_serial);
        Serial.print(kDelimiter);
        Serial.println("reserva cancelada");
      }
    }
  }
}
void atualiza_vagas_lcd(){
  int vagas_livre=0;
  for(int i=0; i<8; i++)
  {
    if (status[i] == verde)
    {
      vagas_livre++;
    }
  }  
  lcd.setCursor(15,1); // Coloca o cursor do display na coluna 1 e linha 1
  lcd.print(vagas_livre);// Comando de saída com a mensagem que deve aparecer na coluna 2 e linha 1.
}
void mensageminicial()
{
  lcd.setCursor(1,0); // Coloca o cursor do display na coluna 1 e linha 1
  lcd.print("PROJ ENG.UNI 1"); // Comando de saída com a mensagem que deve aparecer na coluna 2 e linha 1.
  lcd.setCursor(0, 1); //Coloca o cursor do display na coluna 1 e linha 2
  lcd.print("VAGAS Livres:");  // Comando de saida com a mensagem que deve aparecer na coluna 2 e linha 2
}
String le_o_modulo_RFID()
{
  
  String conteudo= "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  conteudo.toUpperCase();
  return conteudo;
}
void sendDataToSerial()
{
  Serial.print("");
  Serial.print(kDelimiter);
  Serial.print("");
  Serial.println(); // Add final line ending character only once
}
//-----------------------------------------------------------------------------
// DO NOT EDIT ANYTHING BELOW THIS LINE
//-----------------------------------------------------------------------------
void processOutgoingSerial()// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
{
  if((millis() - serialPreviousTime) > kSerialInterval) // Enter into this only when serial interval has elapsed
  {
    // Reset serial interval timestamp
    serialPreviousTime = millis(); 
    sendDataToSerial(); 
  }
}
void processIncomingSerial()// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
{
  if(Serial.available()){
    parseData(GetSerialData());
  }
}
char* GetSerialData() // Gathers bytes from serial port to build inputString
{
  static char inputString[64]; // Create a char array to store incoming data
  memset(inputString, 0, sizeof(inputString)); // Clear the memory from a pervious reading
  while (Serial.available()){
    Serial.readBytesUntil('\n', inputString, 64); //Read every byte in Serial buffer until line end or 64 bytes
  }
  return inputString;
}
void parseData(char data[]) // Seperate the data at each delimeter
{
    char *token = strtok(data, ","); // Find the first delimeter and return the token before it
    int index = 0; // Index to track storage in the array
    while (token != NULL){ // Char* strings terminate w/ a Null character. We'll keep running the command until we hit it
      arr[index] = token; // Assign the token to an array
      token = strtok(NULL, ","); // Conintue to the next delimeter
      index++; // incremenet index to store next value
    }
}