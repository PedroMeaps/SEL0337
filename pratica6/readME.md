**Autor:** 
* Pedro Pontes - 13864627
* Ruan Victor - 14691533

# Projeto Final - Parte 1: Caracterização de Sistemas Embarcados

## 1. Seleção do Sistema Embarcado

**Produto Escolhido:** Aspirador Robô iRobot Roomba (Série 600)

**Descrição Geral:**
O Roomba (Série 600) é um robô aspirador doméstico autônomo desenvolvido pela iRobot. Sua principal função é realizar a limpeza de pisos secos (madeira, piso frio e tapetes) de forma automatizada. Ele utiliza um sistema de navegação baseado em comportamentos (iAdapt 1.0) para cobrir a área de limpeza, detectando sujeira e evitando obstáculos ou quedas. É amplamente utilizado no segmento de eletrodomésticos e automação residencial (Smart Home).

---

## 2. Características Técnicas

**Referência Técnica Principal:**
* *iRobot® Create® 2 Open Interface (OI) Specification based on the iRobot® Roomba® 600*. Disponível em: [iRobot Education](https://edu.irobot.com/learning-library/create-2).

### Unidade de Processamento
* **Microcontrolador (MCU):** O sistema central geralmente utiliza um microcontrolador da família **STMicroelectronics STM32F103** (ARM Cortex-M3, 32-bit) ou similar proprietário, operando em frequências típicas de 72 MHz.
* **Arquitetura:** ARMv7-M (RISC).
* **Plataforma:** Placa dedicada (PCB customizada pela iRobot).

### Memória
* **Flash:** Interna ao MCU (tipicamente 128KB a 512KB para armazenar o firmware).
* **RAM:** Interna ao MCU (tipicamente 20KB a 64KB para variáveis de controle e estados dos sensores).
* **EEPROM:** Utilizada para armazenar configurações do usuário (agendamentos) e logs de erro.

### Sistema Operacional
* **Tipo:** **Bare Metal** ou **RTOS Proprietário**. O Roomba 600 não roda sistemas complexos como Linux. Ele opera com um firmware de loop de controle rígido que gerencia os estados dos sensores e atuadores em tempo real (ciclos de 15ms segundo a especificação OI).

### 🔌 Interfaces de Comunicação
* **Com Fio (Interna/Debug):**
    * **UART (Serial):** Disponível através da porta Mini-DIN externa (para diagnósticos e hackers, seguindo o protocolo *iRobot Open Interface*).
    * **SPI/I2C:** Utilizados internamente na PCB para comunicação entre o MCU e sensores periféricos (giroscópio, drivers de motor).
* **Sem Fio:**
    * **RF (Rádio Frequência):** Modelos mais antigos usavam RF proprietário para comunicar com "Paredes Virtuais".
    * **Wi-Fi (ESP8266/ESP32):** Modelos "conectados" da série 600 (ex: 690) possuem um módulo Wi-Fi (muitas vezes um SoC separado) para conexão com o App iRobot Home e Nuvem.

###  Entradas e Saídas (I/O) e Sensores/Atuadores

**Sensores (Inputs):**
* **Sensores de Abismo (Cliff Sensors):** 4 sensores IR (Infravermelho) na parte inferior para evitar quedas em escadas.
* **Sensor de Colisão (Bumper):** Sensores táteis/ópticos no para-choque frontal.
* **Sensor de Sujeira (Dirt Detect):** Sensor piezoelétrico (acústico) que detecta o impacto de grãos de areia/sujeira nas escovas.
* **Sensores de Roda (Wheel Drop):** Micro-switches que detectam se o robô foi levantado do chão.
* **Encoder de Rodas:** Ópticos, para odometria básica e controle de velocidade.
* **Receptor IR Omnidirecional:** No topo, para localizar a base de carregamento (Dock).

**Atuadores (Outputs):**
* **Motores DC com Caixa de Redução:** 2 motores para as rodas motrizes (tração diferencial).
* **Motor da Escova Lateral:** 1 motor DC pequeno.
* **Motor das Escovas Principais:** 1 motor DC de maior potência.
* **Motor de Aspiração (Impeller):** 1 motor de alta rotação no compartimento de lixo.
* **LEDs e Display:** LEDs de status (Bateria, Erro, Dirt Detect) e Display de 7 segmentos (em alguns modelos para agendamento).
* **Alto-falante (Buzzer):** Para feedback sonoro e códigos de erro.

###  Energia
* **Alimentação:** Bateria de **NiMH** (modelos antigos) ou **Li-Ion** (modelos novos), tipicamente 14.4V, ~3000mAh.
* **Gerenciamento:** Circuito PMIC interno para controle de carga e monitoramento de temperatura da bateria. O robô retorna autonomamente à base quando a tensão está crítica.

---

## 3. Rerefencias

> **Referência:**
> G. Alric, D. Lebraly, C. L. R. G. de L. e Silva, and L. Adouane, "Evaluation of the Roomba: A low-cost, ubiquitous platform for robotics research and education," *2015 IEEE/RSJ International Conference on Intelligent Robots and Systems (IROS)*, Hamburg, Germany, 2015.
> *Alternativa focada em algoritmo:* Z. J. Haas and B. Liang, "Ad hoc mobility management with uniform quorum systems," .
> *Referência Escolhida:* Y. Gabriely and E. Rimon, "Spanning-tree based coverage of continuous areas by a mobile robot," *Annals of Mathematics and Artificial Intelligence*, vol. 31, pp. 77–98, 2001.


---
# Diferencial Eletronico com ESP32 e FreeRTOS


> ** Parte 3: Projeto Final - SEL0337 - Projetos em Sistemas Embarcados** > Escola de Engenharia de Sao Carlos (EESC-USP)

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
```
## imagem da simulação no wokwi
* ![Demonstração Inicializar][Captura de tela 2025-12-20 085613.png]
