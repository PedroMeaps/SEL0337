#include <Arduino.h>
#include <Wire.h>  // utilizada para a comunicação serial
#include <LiquidCrystal_I2C.h> // LCD no modo serial I2C
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//parametros PWM:
const uint32_t frequenciaPWM = 5000; //frequencia de 5k Hz
const uint8_t resolucaoPWM = 8; //resolução de 8 bits (0-255)
const uint8_t pwm1_Pin = 0, pwm2_Pin = 2; //pinos de saida PWM

//ADC:
const uint8_t pedal_Pin = 34, volante_Pin = 35; //pinos de leitura ADC

//botao e buzzer
const uint8_t botaoPin = 13, buzzerPin = 14; //pinos

//display i2c:
// Endereço I2C do display LCD
const int lcdAddress1 = 0x27;
const int lcdAddress2 = 0x28;
// Número de colunas e linhas do display LCD
const int lcdColumns = 16;
const int lcdRows = 2;
// Inicialização do display LCD
LiquidCrystal_I2C lcd1(lcdAddress1, lcdColumns, lcdRows);
LiquidCrystal_I2C lcd2(lcdAddress2, lcdColumns, lcdRows);

// Constantes fisicas
const uint16_t pwmMax = 255;
const float D = 1.25; // Comprimento entre as rodas traseiras (m)
const float L = 1.55; // Comprimento entre eixos (m)

// Variaveis Globais
volatile uint16_t leituraPedal = 0;
volatile uint16_t leituraVolante = 0;
volatile uint8_t valorPWM_L = 0;
volatile uint8_t valorPWM_R = 0;

volatile bool protocoloPartida = false;
volatile bool botaoFlag = false;
volatile bool alrPressed = false;

// Handles e Timer
TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;
hw_timer_t *timer = NULL; 

//interrupções
void IRAM_ATTR onButtonPress() {  // Função de interrupção externa de GPIO para lidar com o pressionamento do botão
  botaoFlag = true;
}

void IRAM_ATTR onTimer() {  // Função de interrupção para lidar com o temporizador
  digitalWrite(buzzerPin, LOW);  // desliga o buzzer
  protocoloPartida = true; //da o aval pro PWM começar a funcionar
}

void calcularDiferencial() {
  //converte a escala das leiturad de 0-4095 para uma escala usada nos calculos do diferencial
  //pedal 0-1000:
  float pedalFisico = ((float)leituraPedal / 4095.0) * 1000.0;
  //volante 0-1800 (0 full esquerda e 1800 full direita)
  float volanteFisico = ((float)leituraVolante / 4095.0) * 1800.0;

  float vc = (volanteFisico / 10.0f) - 90.0f; //degress
  float w = (5000.0 * 2.0 * 3.1415 / 60.0) * (pedalFisico / 1000.0); //(rad/s)
  float vm = vc * 39.0 * 3.1415 / (180.0 * 162.0); 

  float pwmL_calc, pwmR_calc;

  if (vm > 0) {
      // Curva para direita
    pwmL_calc = w * (1.0 + D * tan(vm) / (2.0 * L));
    pwmR_calc = w * (1.0 - D * tan(vm) / (2.0 * L));
  } else if (vm < 0) {
       // Curva para esquerda
    pwmL_calc = w * (1.0 + D * tan(vm) / (2.0 * L));
    pwmR_calc = w * (1.0 - D * tan(vm) / (2.0 * L));
  } else {
       // Caso de equilibrio zero
    pwmL_calc = w;
    pwmR_calc = w;
  }
  
  // Limites necessarios pra evitar overflow
  if (pwmL_calc > 523) pwmL_calc = 523;
  if (pwmR_calc > 523) pwmR_calc = 523;
  if (pwmL_calc < 0) pwmL_calc = 0;
  if (pwmR_calc < 0) pwmR_calc = 0;

  //converte pra 0 a 255 pro duty cyle de resolução de 8 bits:
  valorPWM_L = map((long)pwmL_calc, 0, 523, 0, 255);
  valorPWM_R = map((long)pwmR_calc, 0, 523, 0, 255);
}

// TASK 1: Controle (Core 1 - Alta Prioridade)
void Task1_Control(void *pvParameters) {
  for (;;) {
    
    if (botaoFlag && !alrPressed) {
      digitalWrite(buzzerPin, HIGH); //ativa o buzzer
      // Configura timer para 1 segundo (1000000 us)
      timerAlarm(timer, 1000000, false, 0); 
      alrPressed = true; //pra n ativar o timer denovo
      botaoFlag = false;
    }

    leituraPedal = analogRead(pedal_Pin);
    leituraVolante = analogRead(volante_Pin);

    if (protocoloPartida) {
      calcularDiferencial();
      ledcWrite(pwm1_Pin, valorPWM_L);
      ledcWrite(pwm2_Pin, valorPWM_R);
    } else {
      ledcWrite(pwm1_Pin, 0);
      ledcWrite(pwm2_Pin, 0);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// TASK 2: Display (Core 0 - Baixa Prioridade)
void Task2_Display(void *pvParameters) {
  uint16_t lastPedal = 9999, lastVolante = 9999;
  uint8_t lastPwmL = 255, lastPwmR = 255;

  for (;;) {
    //so atualiza cada display se os valores mudarem
    if (abs(leituraPedal - lastPedal) > 10 || abs(leituraVolante - lastVolante) > 10) {
      // Atualiza display
      lcd1.setCursor(0, 0); lcd1.print("Pedal: "); lcd1.print(leituraPedal); lcd1.print("    "); // limpa valor anterior
      lcd1.setCursor(0, 1); lcd1.print("Volante: "); lcd1.print(leituraVolante); lcd1.print("    ");
      lastPedal = leituraPedal;
      lastVolante = leituraVolante;
    }

    if (valorPWM_L != lastPwmL || valorPWM_R != lastPwmR) {
       // Atualiza display
      lcd2.setCursor(0, 0); lcd2.print("Motor L: "); lcd2.print(valorPWM_L); lcd2.print("   ");
      lcd2.setCursor(0, 1); lcd2.print("Motor R: "); lcd2.print(valorPWM_R); lcd2.print("   ");
      lastPwmL = valorPWM_L;
      lastPwmR = valorPWM_R;
    }

    vTaskDelay(pdMS_TO_TICKS(250)); // delay pro diplay
  }
}

void setup() {
  Serial.begin(115200);

  //inicializa os display i2c
  lcd1.init(); lcd1.backlight();
  lcd2.init(); lcd2.backlight();
  lcd1.setCursor(0,0); lcd1.print("Sistema Iniciando");

  pinMode(buzzerPin, OUTPUT); // Configura o pino do LED/Buzzer como saída
  pinMode(botaoPin, INPUT_PULLUP); // Configura o pino do botão como entrada com pull-up interno
  
  // Configuração da interrupção externa GPIO (botão)
  attachInterrupt(digitalPinToInterrupt(botaoPin), onButtonPress, FALLING);

  // Inicia o timer com uma frequência de 1MHz
  timer = timerBegin(1000000); 
  timerAttachInterrupt(timer, &onTimer); // Associa a função de interrupção ao timer

  //pwm 
  ledcAttach(pwm1_Pin, frequenciaPWM, resolucaoPWM);
  ledcAttach(pwm2_Pin, frequenciaPWM, resolucaoPWM);

  // Criação das Tasks (Multicore)
  xTaskCreatePinnedToCore(Task1_Control, "ControleMotor", 4096, NULL, 2, &Task1Handle, 1);
  xTaskCreatePinnedToCore(Task2_Display, "InterfaceLCD", 4096, NULL, 1, &Task2Handle, 0);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}