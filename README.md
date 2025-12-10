# Projeto: Sensor de Ré Automotivo com Alerta de Intrusão

Autores: Luana Karoline e Emilly Félix

##  Descrição do Projeto

Este projeto utiliza um ESP32 para monitorar a distância de obstáculos durante manobras de ré e detectar possíveis intrusões. Ele integra sensores ultrassônicos, sensores de vibração e inclinação, um display LCD 16x2, um botão de alternância de modo, buzzer e três LEDs indicadores de distância. Além disso, envia notificações para o Node-RED Dashboard via MQTT.

O sistema possui dois modos principais:

1. Modo Manobra – Monitoramento da distância do veículo com alertas visuais (LEDs) e sonoros (buzzer).
2. Modo Segurança – Monitoramento de intrusão utilizando sensores de vibração e inclinação, ativando alarme visual e sonoro.

##  Componentes Utilizados

| Componente                     | Função                                           |
| ------------------------------ | ------------------------------------------------ |
| ESP32                          | Controlador principal do sistema                 |
| Sensor Ultrassônico (HC-SR04)  | Mede a distância de obstáculos durante a ré      |
| Display LCD 16x2               | Exibe informações do modo atual e alertas        |
| Botão de Modo                  | Alterna entre Modo Manobra e Modo Segurança      |
| Sensor de Vibração (SW-420)    | Detecta movimentação suspeita / intrusão         |
| Sensor de Inclinação (SW-520D) | Detecta inclinação do veículo                    |
| Buzzer                         | Alerta sonoro em situações de perigo ou intrusão |
| LED Verde                      | Distância segura (> 300 cm)                      |
| LED Amarelo                    | Atenção (> 50 cm e ≤ 300 cm)                     |
| LED Vermelho                   | Perigo (≤ 50 cm)                                 |

##  Funcionalidades

### Modo Manobra

* Mostra no LCD a distância até o obstáculo.
* LEDs indicam a distância:

  * Verde → Distância > 300 cm (Livre)
  * Amarelo → Distância > 50 cm e ≤ 300 cm (Atenção)
  * Vermelho → Distância ≤ 50 cm (Perigo)
* Toca buzzer se o obstáculo estiver muito próximo.
* Envia valores da distância para o Node-RED via MQTT no tópico `/ESP32/DISTANCIA`.

### Modo Segurança

* Monitora sensores de vibração e inclinação.
* Aciona alarme sonoro e LEDs caso haja detecção de intrusão.
* Envia mensagem de alerta via MQTT no tópico `/ESP32/INTRUSAO` com "ALARME_DISPARADO".
* Notificação visual no Node-RED Dashboard com texto piscante.

### Botão de Modo

* Alterna entre Modo Manobra e Modo Segurança.
* Pode ser controlado fisicamente no ESP32 ou via Node-RED Dashboard através do tópico `/ESP32/BOTAO_MODO`.
* Publica o modo atual no tópico `/ESP32/MODO_ATUAL` para atualizar o dashboard.

### Node-RED Dashboard

* Mostra o modo atual (`MODO MANOBRA ATIVADO` ou `MODO SEGURANÇA ATIVADO`) via UI Text.
* Mostra alerta de intrusão piscante via UI Template.
* Mostra distância medida em tempo real via UI Gauge e UI Text.
* Permite alternar o modo diretamente pelo dashboard.

##  Fluxo de Dados

```
ESP32 → MQTT → Node-RED Dashboard
```

* `/ESP32/DISTANCIA` → valor do sensor ultrassônico
* `/ESP32/INTRUSAO` → alerta de intrusão
* `/ESP32/BOTAO_MODO` → comando para alternar o modo
* `/ESP32/MODO_ATUAL` → modo atual (Manobra ou Segurança)

##  Esquema de Conexão

* Sensor Ultrassônico

  * TRIG → pino 19
  * ECHO → pino 18
* LCD 16x2 I2C

  * SDA → pino 21
  * SCL → pino 22
* Botão de Modo

  * Pino → 27 (INPUT_PULLUP)
* Sensores de Intrusão

  * Vibração → 13 (INPUT_PULLUP)
  * Inclinação → 12 (INPUT_PULLUP)
* LEDs

  * Verde → 4
  * Amarelo → 2
  * Vermelho → 5
* Buzzer

  * Pino → 14

##  Instalação e Configuração

1. Configure seu ESP32 na Arduino IDE.
2. Instale as bibliotecas necessárias:

   * `WiFi.h`
   * `PubSubClient.h`
   * `LiquidCrystal_I2C.h`
   * `Wire.h`
     
3. Atualize suas credenciais de Wi-Fi e endereço do broker MQTT no código.
4. Faça o upload do código para o ESP32.
5. Configure o Node-RED Dashboard:

   * MQTT In nodes para os tópicos definidos.
   * UI Text para modo e distância.
   * UI Template para design e alternar o botão.
   * UI Gauge para mostrar distância de forma visual.

##  Simulador Wokwi

Você pode testar o projeto no Wokwi usando o link abaixo:

[Simulador Wokwi – Sensor de Ré Automotivo](https://wokwi.com/projects/449375005557960705)

* Simula o ESP32, sensor ultrassônico, LEDs, buzzer e LCD.
* Permite testar o código sem precisar do hardware físico.
