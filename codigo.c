/*****************************************************************************/
/*     Projeto: Sensor de ré automotivo integrado com Alerta de Intrusão    */
/*                     Luana Karoline e Emilly Félix                         */
/*****************************************************************************/

// Possível configuração de Distancia Mínima em centimetros para testes
// const int distancia_carro = 20; 

// Bibliotecas para o LCD e (opcional) WiFi/MQTT
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

const char* WIFI_SSID = "brisa-1308851";
const char* WIFI_SENHA = "9ayccism";

const char* MQTT_SERVER_IP = "192.168.0.4";
const int MQTT_PORT = 1883;

const char* TOPICO_SENSORES = "/ESP32/SENSORES";
const char* TOPICO_BOTAO_MODO = "/ESP32/BOTAO_MODO";
const char* TOPICO_MODO = "/ESP32/MODO_ATUAL";
const char* TOPICO_DISTANCIA = "/ESP32/DISTANCIA";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
int counter = 0;

// Função para conectar ao Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se ao ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP do ESP32: ");
  Serial.println(WiFi.localIP());
}

/* Função para reconectar ao MQTT (caso a conexão caia)
void reconnect() {
  // Loop até reconectar
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");
    
    // Crie um ID de cliente único
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Tenta conectar
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado!");
  
      //client.subscribe("topico/de/comando");

    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Espera 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}*/

// ---------------------------------------------------------------------------
// Configurações de Portas do ESP32
// ---------------------------------------------------------------------------

// Pinos dos Sensores de Alerta
//const int SENSOR_VIBRACAO   = 13; // SW-420 - Entrada Digital
//const int SENSOR_INCLINACAO = 12; // SW-520D - Entrada Digital

// LCD
#define lin   2
#define col   16
#define index 0x27

LiquidCrystal_I2C display(index, col, lin);

// Sensor ultrassônico
const int TRIG = 19;
const int ECHO = 18;

// Leds
const int ledGreen  = 4;
const int ledRed    = 5;
const int ledYellow = 2;

// Botão
bool modo_seguranca = true; // true = SEGURANÇA, false = MANOBRA
const int botao_modo = 27;
const int botao_intrusao01 = 13;
const int botao_intrusao02 = 12;

// Buzzer
const int buzzer = 14;

// Variáveis para funcionamento do Buzzer
float seno;
int frequencia;

// ---------------------------------------------------------------------------
// Protótipos de funções
// ---------------------------------------------------------------------------
int  pulso_distancia(int pinotrig,int pinoecho);
void tocaBuzzer();
bool alternarModo(bool modoAtual);
void callbackMQTT(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void enviaMensagemIntrusao();

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando dispositivo.");

  setup_wifi();
  client.setServer(MQTT_SERVER_IP, MQTT_PORT);
  client.setCallback(callbackMQTT);

  // Inicialização do display
  display.init();
  display.backlight();
  display.clear();

  // Configuração dos Pinos dos sensores de intrusão
  //pinMode(SENSOR_VIBRACAO,   INPUT); 
  //pinMode(SENSOR_INCLINACAO, INPUT); 
  
  // Inicialização do Sensor Ultrassônico
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  
  // Inicialização dos LEDs
  pinMode(ledGreen,  OUTPUT);
  pinMode(ledRed,    OUTPUT);
  pinMode(ledYellow, OUTPUT);

  // Inicialização do Buzzer
  pinMode(buzzer, OUTPUT); 

  // Configuração do Botão com PULLUP
  pinMode(botao_modo, INPUT_PULLUP);
  pinMode(botao_intrusao01, INPUT_PULLUP);
  pinMode(botao_intrusao02, INPUT_PULLUP);
}

// ---------------------------------------------------------------------------
// LOOP
// ---------------------------------------------------------------------------
void loop() {

  if (!client.connected()) reconnectMQTT();
  client.loop();

  // Atualiza modo conforme botão (detecta clique)
  modo_seguranca = alternarModo(modo_seguranca);

  // Lê sensores
  int distancia = pulso_distancia(TRIG, ECHO);
  //int vibracao_status  = digitalRead(SENSOR_VIBRACAO);
  //int inclinacao_status = digitalRead(SENSOR_INCLINACAO);

  // Publica no MQTT
char buffer[10];
sprintf(buffer, "%d", distancia);  // converte int para string
client.publish("/ESP32/DISTANCIA", buffer);

  // ---------- MODO MANOBRA ----------
  if (modo_seguranca == false) {

    display.clear();
    display.setCursor(0,0);
    display.print("Modo: MANOBRA");
    
    // ---- Lógica do sensor de ré ----

    if (distancia > 300) {
      
      Serial.printf("LIVRE: %d cm\n", distancia);

      digitalWrite(ledGreen,  HIGH);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed,    LOW);
      noTone(buzzer);

      display.setCursor(0,1);
      display.print("Livre: ");
      display.print(distancia);
      display.print("cm");

    } else if (distancia > 50) {
      
      Serial.printf("ATENCAO: %d cm\n", distancia);

      digitalWrite(ledGreen,  LOW);
      digitalWrite(ledYellow, HIGH);
      digitalWrite(ledRed,    LOW);
      noTone(buzzer);

      display.setCursor(0,1);
      display.print("Atencao: ");
      display.print(distancia);
      display.print("cm");

    } else {
      
      Serial.printf("PERIGO: %d cm\n", distancia);

      digitalWrite(ledGreen,  LOW);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed,    HIGH);
      tocaBuzzer();

      display.setCursor(0,1);
      display.print("PERIGO: ");
      display.print(distancia);
      display.print("cm");
    }

    delay(100);

  // ---------- MODO SEGURANÇA ----------
  } else {

    // Usa leituras já feitas dos sensores de intrusão

    display.clear();

    if (digitalRead(botao_intrusao01) == LOW || digitalRead(botao_intrusao02) == LOW) {

      // ALERTA DE INTRUSÃO
      Serial.println("INTRUSAO DETECTADA");

      enviaMensagemIntrusao(); // publica no MQTT

      display.setCursor(0,0); 
      display.print("ALERTA"); 
      display.setCursor(0,1); 
      display.print("!INTRUSAO!");

      //client.publish("VERIFIQUE O CARRO", "!!ALARME DISPARADO!!");

      // Ativa o alarme
      digitalWrite(ledGreen,  HIGH);
      digitalWrite(ledYellow, HIGH);
      digitalWrite(ledRed,    HIGH); // LED vermelho de alerta
      tocaBuzzer();

    } else {
      // MODO SEGURANÇA ATIVO (SEM INTRUSÃO)
      display.setCursor(0,0); 
      display.print("Modo Seguranca"); 
      display.setCursor(0,1); 
      display.print("Ativo");

      // Mantém atuadores desligados e LED Verde ligado
      digitalWrite(ledGreen,  HIGH);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed,    LOW);
      noTone(buzzer);
    }
  }

  if (modo_seguranca) {
    client.publish("/ESP32/MODO_ATUAL", "MODO SEGURANCA ATIVADO");
} else {
    client.publish("/ESP32/MODO_ATUAL", "MODO MANOBRA ATIVADO");
}

  delay(100);
}

// ---------------------------------------------------------------------------
// FUNÇÕES AUXILIARES
// ---------------------------------------------------------------------------

// Função que envia alerta de intrusão
void enviaMensagemIntrusao() {
  if (client.connected()) {
    client.publish(TOPICO_SENSORES, "ALARME DISPARADO");
  }
}

// Função de callback MQTT (recebe comando do dashboard)
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  if (String(topic) == TOPICO_BOTAO_MODO) {
    if (msg == "TOGGLE") {
      modo_seguranca = !modo_seguranca;
      Serial.println("Modo alternado via MQTT!");
    }
  }
}

// Reconecta MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado!");
      client.subscribe(TOPICO_BOTAO_MODO); // escuta comandos do dashboard
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
    }
  }
}

// Função para calcular a distância do ultrassônico
int pulso_distancia(int pinotrig,int pinoecho){
  digitalWrite(pinotrig, LOW);
  delayMicroseconds(2);  
  digitalWrite(pinotrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinotrig, LOW);

  // Converte a duração do pulso para distância em cm (Duração / 58)
  return pulseIn(pinoecho, HIGH) / 58;
}

// Função para execução do Alarme Sonoro (tom senoidal crescente/decrescente)
void tocaBuzzer(){
  for(int x = 0; x < 180; x++){
    seno = (sin(x * 3.1416 / 180));
    frequencia = 2000 + (int(seno * 1000));
    tone(buzzer, frequencia);
    delay(2);
  }
  // Garante que o tom termine após o ciclo
  noTone(buzzer);
}

bool alternarModo(bool modoAtual) {
  static bool estadoAnterior = HIGH;
  bool estadoAtual = digitalRead(botao_modo);

  // DETECTA APENAS O APERTO (flanco de descida)
  if (estadoAnterior == HIGH && estadoAtual == LOW) {

    // alterna o modo apenas aqui
    modoAtual = !modoAtual;

    display.clear();
    if (modoAtual) {
      Serial.println("Modo SEGURANCA");
      display.setCursor(0,0);
      display.print("Modo:");
      display.setCursor(0,1);
      display.print("Seguranca");
      //client.publish(TOPICO_MODO, "MODO SEGURANCA ATIVADO");
    } else {
      Serial.println("Modo MANOBRA");
      display.setCursor(0,0);
      display.print("Modo:");
      display.setCursor(0,1);
      display.print("Manobra");
      //client.publish(TOPICO_MODO, "MODO MANOBRA ATIVADO");
    }
    

    delay(200);
  }

  // atualiza estado do botão
  estadoAnterior = estadoAtual;

  // devolve o modo ATUAL sem alteração
  return modoAtual;
}
