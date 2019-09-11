#include <LiquidCrystal.h>
#include <Wire.h>;
#include "RTClib.h"
#include <Keypad.h>
#include <String.h>

#define BUZZER 23

RTC_DS3231 rtc;

char keys[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[4] = {32, 33, 25, 26};
byte colPins[4] = {27, 14, 12, 13};

typedef struct Ponto {
  uint32_t hr_entrada; // 32 bits
  uint32_t hr_saida;  // 32 bits
  char matricula[5]; //32 bits
  Ponto *prox;
} Ponto; // 96 bits 

const int rs = 19, en = 18, d4 = 5, d5 = 4, d6 = 2, d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, 4, 4);

/*
 *  VARIAVEIS
 */
// matricula
char buffer_matricula[5];
int cnt_buffer_matricula = 0;
void imprimeHorario(void);

//pontos
Ponto* pontos;
int cnt_pontos = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT);
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  lcd.begin(16, 4);
}


uint32_t agora() {
  DateTime now = rtc.now(); 
  return now.unixtime() + 10800; // + 3 hours GMT
}

void imprimeHorario() {
  DateTime now = rtc.now();
  
  uint8_t hour = now.hour();
  uint8_t minute = now.minute();
  if(hour >= 6 && hour <= 11 && minute <= 59) {
    lcd.print("   Bom dia");
  } else if(hour >= 12 && hour <= 19 && minute <= 59) {
    lcd.print("   Boa tarde!");
  } else if() {
  
  lcd.setCursor(1, 1);
  lcd.print("Horario:");
  lcd.setCursor(1, 2);
  lcd.print(now.day());
  lcd.print('/');
  lcd.print(now.month());
  lcd.print('/');  
  lcd.print(now.year());
  lcd.setCursor(1, 3);
  lcd.print(now.hour());
  lcd.print(':');
  lcd.print(now.minute());
  lcd.print(':');
  lcd.print(now.second());
  lcd.println();
}

short atualizaSaida() {
  short retorno = 0;
  Ponto *proximo = pontos;
  while (proximo != NULL) {
    if(proximo->hr_saida == 0 && strcmp(proximo->matricula, buffer_matricula) == 0) {
      proximo->hr_saida = agora();
      retorno = 1;
    }
    
    proximo = proximo->prox; 
  }
  
  return retorno;
}

void registraPonto() {
  /*
   * FINALIZA NOSSA STRING
   */
  //buffer_matricula[(cnt_buffer_matricula+1)] = '\0';  
 

  if(!atualizaSaida()) {
    Ponto *novoPonto = (Ponto *) malloc(sizeof(Ponto));
    novoPonto->hr_entrada = agora();
    novoPonto->hr_saida   = 0;
    strcpy(novoPonto->matricula, buffer_matricula);    
    novoPonto->prox = NULL;
  
    /*
     * Salva novo ponto
     */
    if (cnt_pontos == 0) {
      pontos = novoPonto;
    } else {
      // DESCOBRE QUEM Q EH O ULTIMO, E COLOCA O END NOVO
      Ponto *proximo = pontos;
      while(proximo->prox != NULL) {
        proximo = proximo->prox;
      }
      proximo->prox = novoPonto;
    }
  
    cnt_pontos++;  
  }
  
  /*
   * LIMPA BUFFER
   */
  int i;
  for(i = 0; i < 4; i++){
   buffer_matricula[i] = '\0';
  }
  cnt_buffer_matricula = 0;
}

void leTeclado() {
  char myKey = myKeypad.getKey();
  if (myKey != NULL) {
    Serial.println("Nova tecla:");
    Serial.println(myKey);
    if(cnt_buffer_matricula < 4 && myKey != 'c') {
      buffer_matricula[cnt_buffer_matricula] = myKey;
      cnt_buffer_matricula++;
      if(cnt_buffer_matricula == 4) {       
        registraPonto();
        imprimePontos();
      }
    } else {
      registraPonto();
      imprimePontos();
    }
  }
}

char lerTeclado(){
  char myKey = myKeypad.getKey();
  if(myKey != NULL){
    return myKey;  
  } else {
    return ' ';
  }
}


void recuperaPontos() {

}

int leNumeroDaSerial() {
  short pronto = 0;
  char numeros[5];
  int tamanho = 0;
  
  while(!pronto) {
    while(Serial.available() > 0) {
      char numero = Serial.read();
      if(numero == '\n' || tamanho > 4) {
        pronto = 1;
        break;
      } else {
        numeros[tamanho] = numero;
        tamanho++; 
        numeros[tamanho] = '\0';
      }
    }
  }

  return atoi(numeros);
}

void configTime() {
  String recebido;
  
  Serial.println("Qual ano que estamos? ");
  int ano = leNumeroDaSerial();
  Serial.println(ano);
  
  Serial.println("Qual mes que estamos? ");
  int mes = leNumeroDaSerial();
  Serial.println(mes);

  Serial.println("Qual dia que estamos? ");
  int dia = leNumeroDaSerial();
  Serial.println(dia);
    
  Serial.println("Qual hora que estamos? ");
  int hora = leNumeroDaSerial();
  Serial.println(hora);
    
  Serial.println("Qual minuto que estamos? ");
  int minuto = leNumeroDaSerial();
  Serial.println(minuto);
    
  Serial.println("Qual segundo que estamos? ");
  int segundo = leNumeroDaSerial();
  Serial.println(segundo);
  
  rtc.adjust(DateTime(ano, mes, dia, hora, minuto, segundo));
  
  imprimeHorario();
}

void imprimePontos() {
    Serial.println("---- LOG ----");
  if(cnt_pontos > 0) {
    Ponto *proximo = pontos;
    int registros = 1;
    while (proximo != NULL) {
      Serial.print(registros);
      Serial.println("*) Registro:");
      Serial.print("Matricula: ");
      Serial.println(proximo->matricula);
      Serial.print("Horario de entrada: ");
      Serial.println(proximo->hr_entrada);
      Serial.print("Horario de saida: ");
      if(proximo->hr_saida == 0) {
        Serial.println("Ainda nao saiu, ate agora hehe");
      } else {
        Serial.println(proximo->hr_saida);
      }

      Serial.println("---------");
      proximo = proximo->prox; 
      registros++;
    }
  } else {
    Serial.println("SEM REGISTROS");
  }
  Serial.println("---- FIM LOG ----");
}

void loop() {
  imprimeHorario();
//  char opcao = lerTeclado();
//  switch(opcao) {
//    case 'a':
//      lcd.clear();
//      lcd.print("apertou a");
//      break;
//    case 'b':
//      lcd.clear();
//      lcd.print("apertou b");
//      break;
//    case 'c':
//      lcd.clear();
//      lcd.print("apertou c");
//      break;
//    case 'd':
//      lcd.clear();
//      lcd.print("apertou d");
//      break;
//    default:
//      lcd.clear();
//      lcd.print("aperta certo essa buceta");
//      lerTeclado();
//  }
  while (Serial.available() > 0) {
    char digito = Serial.read();
    if(digito == 'c') {
      while (Serial.available() > 0) Serial.read();
      configTime();
    } else if(digito == 'm') {
      imprimePontos();
    } else if(digito == 'b') {
      lcd.print("Digite a matricula");
    }
  }
  
  leTeclado();
}
