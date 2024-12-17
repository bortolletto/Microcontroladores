#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>  // Biblioteca para o RTC



// --- Constantes do anemômetro ---
const float pi = 3.14159265;
int period = 5000;               // Tempo de medida (milissegundos)
int delaytime = 1000;            // Intervalo entre as amostras (milissegundos)
int radius = 147;                // Raio do anemômetro (mm)

// --- Variáveis do anemômetro ---
unsigned int Sample = 0;         // Armazena o número de amostras
unsigned int counter = 0;        // Contador para o sensor
unsigned int RPM = 0;            // Rotações por minuto
float speedwind = 0.0;           // Velocidade do vento (m/s)
float windspeed = 0.0;           // Velocidade do vento (km/h)

// --- Variáveis do LED ---
int azul = 9;
int verde = 10;
int vermelho = 7;

// --- Variáveis do SD ---
File cartao; // Variável para o arquivo no cartão SD
const int CS = 8; // Pino Chip Select (geralmente 10 para Arduino Uno)

// --- Instância do RTC ---
RTC_DS1307 rtc;

// Função para incrementar o contador
void addcount() {
  counter++;
}

// Função para medir a velocidade do vento
void windvelocity() {
  speedwind = 0;
  windspeed = 0;
  counter = 0;
  attachInterrupt(0, addcount, RISING);
  unsigned long millis();
  long startTime = millis();
  while (millis() < startTime + period) {}
}

// Função para calcular o RPM
void RPMcalc() {
  RPM = ((counter) * 60) / (period / 1000); // Calcula o RPM
}

// Função para calcular a velocidade do vento em m/s
void WindSpeed() {
  windspeed = ((4 * pi * radius * RPM) / 60) / 1000; // Velocidade do vento em m/s
}

// Função para calcular a velocidade do vento em km/h
void SpeedWind() {
  speedwind = (((4 * pi * radius * RPM) / 60) / 1000) * 3.6; // Velocidade do vento em km/h
}

// --- Configurações Iniciais ---
void setup() {
  Serial.begin(9600);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);    // Ativa pull-up interno
  
  pinMode(azul, OUTPUT);
  pinMode(verde, OUTPUT);
  pinMode(vermelho, OUTPUT);

  // Inicializa o cartão SD
  if (!SD.begin(CS)) {
    Serial.println("ERROR - Não foi possível acessar o cartão");
    while (true);  // Loop infinito se não conseguir acessar o cartão
  }
  
  // Verifica se o arquivo existe
  if (SD.exists("dados.txt")) {
    cartao = SD.open("dados.txt", FILE_WRITE);
  } else {
    cartao = SD.open("dados.txt", FILE_WRITE);
    if (cartao) {
      cartao.println("Data, counter, RPM, Vel.Vento[m/s], Vel.Vento[km/h]");
      cartao.close();  // Fecha o arquivo após escrever o cabeçalho
    } else {
      Serial.println("Erro na escrita do cartão!");
      while (true);  // Loop infinito se não conseguir abrir o arquivo
    }
  }

  // Inicializa o RTC
  if (!rtc.begin()) {
    Serial.println("Erro ao inicializar o RTC");
    while (1);  // Loop infinito se o RTC não for inicializado
  }
  if(rtc.isrunning() == false){
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
}

}

// --- Loop Principal ---
void loop() {
  Sample++;
  
  // Leitura de dados do anemômetro
  windvelocity();
  
  // Calculando RPM e velocidades
  RPMcalc();
  WindSpeed();
  SpeedWind();

  // Exibe as informações no Serial Monitor
  Serial.print("Data: ");
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print("; Contador: ");
  Serial.print(counter);
  Serial.print("; RPM: ");
  Serial.print(RPM);
  Serial.print("; Vel. Vento: ");
  Serial.print(windspeed);
  Serial.print(" [m/s] ");
  Serial.print(speedwind);
  Serial.print(" [km/h] ");
  Serial.println();

  // Escreve as informações no cartão SD
  cartao = SD.open("dados.txt", FILE_WRITE);
  if (cartao) {
    cartao.print(now.year(), DEC);
    cartao.print('/');
    cartao.print(now.month(), DEC);
    cartao.print('/');
    cartao.print(now.day(), DEC);
    cartao.print(" ");
    cartao.print(now.hour(), DEC);
    cartao.print(':');
    cartao.print(now.minute(), DEC);
    cartao.print(':');
    cartao.print(now.second(), DEC);
    cartao.print(",");
    cartao.print(counter);
    cartao.print(",");
    cartao.print(RPM);
    cartao.print(",");
    cartao.print(windspeed);
    cartao.print(",");
    cartao.println(speedwind);
    cartao.close();  // Fecha o arquivo após escrever
  } else {
    Serial.println("Erro na gravação do arquivo!");
  }

  // Controle do LED
  if (speedwind <= 20.0) {
    digitalWrite(verde, HIGH);
    digitalWrite(azul, LOW);
    digitalWrite(vermelho, LOW);
  } else if (speedwind <= 40.0) {
    digitalWrite(verde, LOW);
    digitalWrite(azul, HIGH);
    digitalWrite(vermelho, LOW);
  } else {
    digitalWrite(verde, LOW);
    digitalWrite(azul, LOW);
    digitalWrite(vermelho, HIGH);
  }

  delay(delaytime);  // Taxa de atualização
}
