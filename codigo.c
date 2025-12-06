/******************************************************************************/
/*     Projeto: Sensor de ré automotivo integrado com Alerta de Intrusão      */
/*                     Luana Karoline e Emilly Félix                          */
/******************************************************************************/

//Possível configuração de Distancia Mínima em centimetros para testes
//const int distancia_carro = 20; 

//Bibliotecas para o LCD
#include <WiFi.h>
//#include <PubSubClient.h>
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> 

/*const char* WIFI_SSID = "WIFI 2.4G";
const char* WIFI_SENHA = "";

const char* MQTT_SERVER_IP = "";
const int MQTT_PORT = 1883;

const char* MQTT_TOPIC = "/ESP32/SENSORES";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
int counter = 0;*/

// Função para conectar ao Wi-Fi
/*void setup_wifi() {
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

// Função para reconectar ao MQTT (caso a conexão caia)
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
      
      // (Opcional) Se inscrever em tópicos, se precisar
      // client.subscribe("topico/de/comando");

      } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Espera 5 segundos antes de tentar novamente
      delay(5000);
    }
  }
}*/

//Configurações de Portas do ESP32

// Pinos dos Sensores de Alerta
//const int SENSOR_VIBRACAO = 35;  // SW-420 - Entrada Digital
//const int SENSOR_INCLINACAO = 34; // SW-520D - Entrada Digital

//LCD
#define lin 2
#define col 16
#define index 0x27

LiquidCrystal_I2C display(index,col,lin);

//Sensor ultrassônico
const int TRIG = 19;
const int ECHO = 18;

//sensor para o alerta de intrusão 

// Leds
const int ledGreen = 4;
const int ledRed = 5;
const int ledYellow = 2;

//Botão
bool modo_seguranca = true; // true = SEGURANÇA, false = MANOBRA
const int botao_modo = 32;
const int botao_intrusao01 = 34;
const int botao_intrusao02 = 35;

//Buzzer
const int buzzer = 14;

// Variaveis para funcionamento do Buzzer
float seno;
int frequencia;

//funções
int pulso_distancia(int pinotrig,int pinoecho);
void tocaBuzzer();
bool alternarModo(bool modoAtual);

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando dispositivo.");

  //setup_wifi();
  //client.setServer(MQTT_SERVER_IP, MQTT_PORT);

  //inicialização do display
  display.init();
  display.backlight();
  display.clear();

  // Configuração dos Pinos
  //pinMode(SENSOR_VIBRACAO, INPUT); 
  //pinMode(SENSOR_INCLINACAO, INPUT); 
  
  
  // Inicialização do Sensor Ultrassônico
  pinMode(TRIG,OUTPUT);
  pinMode(ECHO,INPUT);
  
  // Inicialização dos LEDs
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);

  //Inicialização do Buzzer
  pinMode(buzzer, OUTPUT); 

  // Configuração do Botão com PULLUP
  pinMode(botao_modo, INPUT_PULLUP);
  pinMode(botao_intrusao01, INPUT_PULLUP);
  pinMode(botao_intrusao02, INPUT_PULLUP);
  
}


void loop() {
  // 1. Verifica e alterna o modo (se o botão for pressionado)
  modo_seguranca = alternarModo(modo_seguranca);

  // 2. Lógica de funcionamento baseada no modo
  int distancia = pulso_distancia(TRIG, ECHO);

  //int vibracao_status = digitalRead(SENSOR_VIBRACAO);
  //int inclinacao_status = digitalRead(SENSOR_INCLINACAO);

  // Se estiver no modo MANOBRA (modo_seguranca == false)
  if (!modo_seguranca) {
    // Escreve o modo atual (linha 0)
    display.setCursor(0,0); 
    display.print("Modo: MANOBRA"); 
    display.setCursor(0,1); 
    display.print("Distancia"); display.print(distancia); display.print("cm");

  // Lógica do Sensor de Ré
  if (distancia > 300) {
    Serial.print("Livre: "); Serial.print(distancia); Serial.println("cm");
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledRed, LOW);
    noTone(buzzer);
    // Exibe no display em texto pequeno na linha inferior
    display.setCursor(0,0); 
    display.print("Livre"); 
    display.setCursor(0,1); 
    display.print("Distancia"); display.print(distancia); display.print("cm");

  } else if (distancia > 50) {
    Serial.print("Atenção: "); Serial.print(distancia); Serial.println("cm");
    digitalWrite(ledYellow, HIGH);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledRed, LOW);
    noTone(buzzer);
    // Exibe no display em texto pequeno na linha inferior
    display.setCursor(0,0); 
    display.print("Atençao!"); 
    display.setCursor(0,1); 
    display.print("Distancia"); display.print(distancia); display.print("cm");

  }else {
    Serial.print("Perigo: "); Serial.print(distancia); Serial.println("cm");
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledRed, HIGH);
    tocaBuzzer();

    display.setCursor(0,0); 
    display.print("PERIGO"); 
    display.setCursor(0,1); 
    display.print("Distancia"); display.print(distancia); display.print("cm");
  }

  // Se estiver no modo SEGURANÇA (modo_seguranca == true)
  }else {
    // Checa os sensores de intrusão
    bool intrusaoDetectada = (digitalRead(botao_intrusao01) == LOW || digitalRead(botao_intrusao02) == LOW);

    if (intrusaoDetectada) {
      // ALERTA DE INTRUSÃO
      Serial.println("INTRUSÃO DETECTADA");

      display.setCursor(0,0); 
      display.print("ALERTA"); 
      display.setCursor(0,1); 
      display.print("!!INTRUSÃO!!");

      // Ativa o alarme
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed, HIGH); // LED vermelho de alerta
      tocaBuzzer();

    } else {
      // MODO SEGURANÇA ATIVO (SEM INTRUSÃO)
      display.setCursor(0,0); 
      display.print("Modo Seguranca"); 
      display.setCursor(0,1); 
      display.print("Ativo");

      // Mantém atuadores desligados e LED Verde ligado
      digitalWrite(ledGreen, HIGH);
      digitalWrite(ledYellow, LOW);
      digitalWrite(ledRed, LOW);
      noTone(buzzer);
    }
  }

  delay(100);
}

//  FUNÇÕES AUXILIARES

  // Função para calcular a distância do ultrassônico
  int pulso_distancia(int pinotrig,int pinoecho){
    digitalWrite(pinotrig,LOW);
    delayMicroseconds(2);  
    digitalWrite(pinotrig,HIGH);
    delayMicroseconds(10);
    digitalWrite(pinotrig,LOW);

    // Converte a duração do pulso para distância em cm (Duração / 58)
    return pulseIn(pinoecho,HIGH)/58;
  }

  // Função para execução do Alarme Sonoro (tom senoidal crescente/decrescente)
  void tocaBuzzer(){
    for(int x=0;x<180;x++){
      seno=(sin(x*3.1416/180));
      frequencia = 2000+(int(seno*1000));
      tone(buzzer,frequencia);
      delay(2);
    }
  // Garante que o tom termine após o ciclo
  noTone(buzzer);
  }

  // Função para alternar entre os modos
  bool alternarModo(bool modoAtual) {
  // Usa static bool para rastrear o estado anterior do botão (necessário para debounce)
  static unsigned long lastButtonPress = 0;
  const long debounceDelay = 200; // Aumentei o debounce para evitar falhas

  // O botão é ativo-baixo (LOW quando pressionado)
  if (digitalRead(botao_modo) == LOW) {
 
  // Checagem de Debounce: Verifica se passou tempo suficiente desde o último clique
  if ((millis() - lastButtonPress) > debounceDelay) {

  // Inverte o modo
  bool novoModo = !modoAtual;

  if (novoModo) {
    Serial.println("\n--- MODO: SEGURANÇA (Carro Parado) ---");
    display.setCursor(0,0); 
    display.print("Modo:"); 
    display.setCursor(0,1); 
    display.print("Segurança");
  } else {
    Serial.println("\n--- MODO: MANOBRA (Sensor de Ré) ---");
    display.setCursor(0,0); 
    display.print("Modo:"); 
    display.setCursor(0,1); 
    display.print("Manobra");
  }

  // Envia a mensagem de mudança imediatamente

  // Desliga todos os atuadores na transição (Estado neutro)
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledRed, LOW);
  noTone(buzzer); // Usa noTone para desligar o buzzer

  delay(1500); // Pausa para o usuário ver o modo

  lastButtonPress = millis(); // Atualiza o tempo do último clique
  return novoModo;
  }
 }
 
 // Retorna o modo atual se o botão não foi pressionado ou está em debounce
 return modoAtual;
}
