**Autor:** Pedro Pontes - 13864627

           Ruan Victor - 14691533
# Diferencial Eletronico com ESP32 e FreeRTOS


> **Projeto Final - SEL0337 - Projetos em Sistemas Embarcados** > Escola de Engenharia de Sao Carlos (EESC-USP)

## Sobre o Projeto

Este projeto implementa um **Sistema de Diferencial Eletrônico** para veículos elétricos utilizando um microcontrolador **ESP32**. O sistema simula o controle de tração independente de duas rodas traseiras com base no ângulo do volante e na aceleração (pedal), garantindo que, em curvas, a roda externa gire mais rápido que a interna.

O diferencial do projeto é a utilização do **FreeRTOS** para dividir o processamento em dois núcleos (**Dual-Core**), garantindo que o cálculo crítico dos motores não seja afetado pela atualização lenta do display.

---

##  Funcionalidades e Arquitetura RTOS

O sistema foi projetado para explorar a capacidade **Multicore** da ESP32, dividindo a carga de trabalho em duas **Tasks** principais com prioridades distintas:

###  Task 1: Controle Crítico (Core 1)
- **Prioridade:** Alta (2)
- **Função:** Realiza a leitura dos sensores analógicos (Pedal e Volante), executa os cálculos matemáticos do modelo físico do veículo e atualiza o PWM dos motores.
- **Por que Core 1?** Dedicado ao processamento em tempo real para garantir estabilidade na tração, sem interrupções.

###  Task 2: Interface Humano-Máquina (Core 0)
- **Prioridade:** Baixa (1)
- **Função:** Gerencia dois Displays LCD via comunicação I2C. Exibe dados de entrada (sensores) e saída (PWM).
- **Por que Core 0?** A comunicação I2C e a escrita no LCD são processos lentos. Separar esta tarefa evita que o "lag" do display trave o controle dos motores.

---

##  Hardware Utilizado

| Componente | Função | Pino (GPIO) |
| :--- | :--- | :--- |
| **ESP32 DevKit** | Microcontrolador Principal | - |
| **Potenciômetro 1** | Simulador de Pedal (Aceleração) | `GPIO 34` |
| **Potenciômetro 2** | Simulador de Volante (Direção) | `GPIO 35` |
| **Motor DC (Esq.)** | Saída PWM Motor Esquerdo | `GPIO 0` |
| **Motor DC (Dir.)** | Saída PWM Motor Direito | `GPIO 2` |
| **Botão (Push)** | Sistema de Partida (Ignição) | `GPIO 13` |
| **Buzzer** | Alarme Sonoro de Partida | `GPIO 14` |
| **LCD 16x2 (x2)** | Interface Visual (I2C) | `SDA/SCL Padrão` |

---

##  Modelo Matemático

O código implementa equações físicas para calcular a velocidade angular necessária para cada roda baseada na geometria de Ackermann simplificada:

```cpp
// Exemplo da lógica de cálculo
if (curva_direita) {
    pwm_Esquerda = w * (1.0 + D * tan(vm) / (2.0 * L));
    pwm_Direita  = w * (1.0 - D * tan(vm) / (2.0 * L));
}

## imagem da simulação no wokwi
* ![Demonstração Inicializar][Captura de tela 2025-12-20 085613.png]