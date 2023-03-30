//V6
#include <Arduino.h>
#include <WiFi.h>             //Conectar na rede WiFi
#include <IRremoteESP8266.h>  //Enviar sinal infravermelho - https://github.com/crankyoldgit/IRremoteESP8266?utm_source=platformio&utm_medium=piohome
#include <IRsend.h>           //enviar sinal infravermelho - ^^
#include <ESP32_Servo.h>      //Movimentar servo motor - https://github.com/jkb-git/ESP32Servo
#include <FirebaseESP32.h>    //Conectar ao banco de dados - https://github.com/mobizt/Firebase-ESP32?utm_source=platformio&utm_medium=piohome
#include <EmonLib.h>          //Utilizado na leitura do TC - https://github.com/openenergymonitor/EmonLib?utm_source=platformio&utm_medium=piohome
#include <WebServer.h>        //Servidor Web local ( https://github.com/zhouhan0126/WebServer-esp32 ) - https://github.com/tzapu/WiFiManager?utm_source=platformio&utm_medium=piohome
#include <WiFiManager.h>      //Biblioteca para configuração da rede Wi-Fi - https://github.com/tzapu/WiFiManager?utm_source=platformio&utm_medium=piohome
#include <time.h>

//Dados do Banco de dados
#define FIREBASE_HOST "linc-iot.firebaseio.com"
#define FIREBASE_AUTH "MREMBRirFBAL1L4FN3ldAr8zjSRKDax81gpNMbE3"

#define FIREBASE_SERVER_HOST "https://linc-14f98-default-rtdb.firebaseio.com/" 
#define FIREBASE_SERVER_AUTH "LngdZmrf9h0Top5KcXfzXDVVv4WcsFCToUEpG8Xl"          

//Criando objeto da biblioteca
FirebaseData banco,meuBanco;

//Criando objeto do medidor de energia
EnergyMonitor TC;

//Pinos de saida
#define pinResetEN 19
#define ledInterrupt 2
#define lamp1 15
#define lamp2 23
#define lamp3 27
#define porta1 5
#define ar1 22 
#define ar2 4
#define persiana 32
#define resetEsp 34
#define analog 35   //ADC1_Ch7 - para a leitura do TC
#define ldr 33      //ADC1_CH5 - para leitura do LDR
                    //ADC2 não pode ser utulizada se estiver usando o WiFi

//Servomotor
Servo servoPe;
const int anguloMax = 120;
const int anguloMin = 0;

//configuração e dados IR
IRsend irsend1(ar1); //ar condicionado 1 linc
IRsend irsend2(ar2); //ar condicionado 2 linc
//AR2
uint16_t liga[259]                  = {3300, 1578,  440, 372,  440, 370,  440, 1186,  438, 372,  440, 1184,  440, 372,  440, 372,  440, 372,  440, 1184,  440, 1184,  440, 374,  440, 372,  440, 374,  436, 1186,  440, 1184,  440, 372,  440, 370,  440, 372,  438, 372,  440, 372,  440, 410,  402, 372,  440, 372,  442, 370,  440, 372,  440, 370,  440, 372,  440, 372,  438, 1186,  438, 372,  438, 374,  440, 372,  440, 372,  438, 372,  440, 370,  440, 374,  440, 1184,  440, 372,  440, 372,  440, 372,  440, 372,  412, 1248,  402, 1184,  440, 1184,  440, 1186,  440, 1182,  442, 1182,  442, 1184,  440, 1184,  440, 370,  442, 372,  440, 1186,  438, 374,  438, 372,  442, 372,  438, 372,  440, 372,  440, 372,  438, 372,  440, 410,  400, 1186,  442, 1184,  440, 370,  440, 372,  442, 1184,  440, 372,  440, 372,  440, 374,  438, 1186,  440, 1184,  440, 1184,  440, 372,  440, 372,  440, 372,  440, 374,  440, 372,  440, 372,  438, 372,  440, 372,  440, 372,  440, 372,  438, 372,  438, 374,  440, 372,  440, 372,  440, 372,  440, 372,  440, 372,  440, 372,  440, 372,  440, 372,  440, 374,  438, 372,  442, 372,  438, 412,  400, 372,  442, 370,  440, 372,  440, 372,  440, 372,  440, 372,  440, 372,  440, 372,  440, 374,  412, 398,  440, 372,  438, 374,  440, 372,  440, 372,  442, 372,  438, 372,  440, 372,  440, 372,  440, 374,  438, 374,  438, 372,  440, 372,  440, 1184,  440, 372,  438, 372,  440, 1184,  442, 1184,  440, 1184,  440, 1184,  440, 1186,  442, 1182,  440, 372,  440, 372,  438};  // FUJITSU_AC

uint16_t desliga[115]               = {3274, 1642,  392, 402,  414, 426,  392, 1244,  392, 426,  392, 1244,  392, 426,  390, 428,  390, 434,  386, 1244,  392, 1246,  392, 426,  390, 426,  390, 406,  412, 1244,  392, 1244,  394, 424,  392, 426,  392, 426,  392, 426,  390, 404,  414, 428,  392, 426,  392, 430,  388, 424,  392, 426,  392, 426,  392, 426,  392, 426,  392, 1244,  392, 426,  392, 426,  392, 424,  392, 428,  390, 426,  392, 426,  392, 426,  392, 1244,  392, 426,  392, 426,  392, 426,  390, 428,  392, 1246,  392, 426,  388, 428,  392, 426,  394, 424,  390, 428,  390, 428,  392, 1246,  390, 406,  412, 1244,  392, 1244,  390, 1248,  390, 1244,  392, 1244,  394, 1244,  392};  // FUJITSU_AC


//AR1
uint16_t ligaAR1[]    =     {3078, 1582,  524, 1060,  524, 1058,  526, 290,  526, 292,  500, 316,  500, 1082,  500, 316,  500, 316,  524, 1056,  500, 1082,  524, 290,  502, 1082,  500, 316,  528, 288,  526, 1058,  524, 1056,  500, 316,  500, 1082,  526, 1056,  526, 290,  528, 288,  528, 1054,  526, 290,  528, 288,  528, 1054,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 288,  554, 256,  534, 288,  556, 254,  562, 254,  534, 288,  554, 1028,  528, 288,  548, 264,  532, 1054,  528, 288,  528, 288,  554, 256,  534, 288,  528, 288,  528, 1054,  528, 288,  528, 288,  554, 256,  562, 254,  560, 1028,  554, 256,  562, 254,  536, 1054,  526, 290,  528, 288,  552, 256,  560, 256,  536, 288,  554, 1028,  528, 288,  556, 1028,  528, 1054,  554, 1028,  528, 288,  552, 258,  560, 258,  558, 256,  534, 288,  554, 256,  534, 288,  552, 258,  562, 254,  534, 288,  526, 288,  530, 286,  554, 256,  562, 254,  534, 288,  554, 256,  534, 288,  552, 260,  532, 288,  552, 258,  532, 288,  526, 290,  554, 254,  560, 256,  564, 252,  536, 288,  528, 288,  556, 254,  534, 292,  528, 286,  528, 288,  556, 252,  560, 256,  560, 1032,  524, 288,  552, 256,  562, 1026,  552, 258,  560, 258,  562, 252,  560, 260,  558, 256,  560};  // TCL112AC

uint16_t desligaAR1[] =     {3132, 1532,  552, 1030,  552, 1030,  552, 258,  560, 260,  556, 258,  558, 1032,  552, 258,  558, 260,  558, 1030,  550, 1030,  552, 258,  558, 1030,  550, 262,  558, 256,  558, 1030,  552, 1030,  554, 256,  560, 1030,  550, 1030,  552, 260,  558, 258,  558, 1030,  550, 262,  556, 260,  532, 1056,  526, 290,  528, 290,  526, 292,  526, 290,  550, 260,  532, 292,  526, 290,  526, 290,  526, 290,  526, 290,  528, 290,  528, 290,  526, 288,  528, 288,  530, 288,  528, 288,  528, 288,  530, 288,  528, 288,  528, 288,  528, 1054,  526, 288,  528, 288,  528, 288,  528, 288,  528, 288,  528, 1054,  526, 290,  550, 262,  532, 288,  528, 290,  528, 1056,  526, 290,  526, 290,  526, 1056,  526, 290,  526, 288,  528, 290,  526, 290,  526, 290,  526, 1056,  526, 290,  526, 1056,  526, 1056,  524, 1058,  524, 292,  526, 292,  524, 292,  526, 294,  522, 290,  526, 292,  524, 292,  524, 292,  524, 292,  524, 292,  524, 292,  524, 294,  500, 316,  522, 294,  524, 292,  524, 294,  500, 316,  524, 292,  500, 316,  500, 316,  500, 316,  516, 300,  500, 318,  522, 292,  502, 314,  502, 314,  526, 290,  500, 316,  500, 316,  526, 290,  526, 290,  502, 316,  500, 316,  500, 1080,  502, 314,  524, 292,  526, 290,  526, 290,  526, 290,  526, 292,  526, 288,  528, 290,  526};  // TCL112AC


//Configuração do ESP para ser ponto de acesso e receber a dados da rede WiFi que ele vai se conectar
//Aqui são os dados de ponto de acesso quando o ESP inicia como servidor
const char* nome_AP = "LINC_ESP32";  //nome da rede que o ESP cria
const char* senha_AP = "LINC_IF-";   //senha da rede que o ESP cria

WiFiManager wifiManager; //cria uma instancia de controle da classe

//Diretórios do banco de dados do servidor Firebase
const char* lampadas1 = "Lâmpada 1/Status";
const char* lampadas2 = "Lâmpada 2/Status";
const char* lampadas3 = "Lâmpada 3/Status";
const char* porta = "Porta/Status";
const char* arc1 = "Ar-condicionado 1/Status";
const char* arc2 = "Ar-condicionado 2/Status";
const char* persi = "Persiana/Status";
const char* modo = "Modo/Status";       //Manual ou automático

String horaAnt= "/";
String horaAnt2= "/";

//Criando cliente para publicar e inscrever-se
WiFiClient espClient;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = -3600*3;

String hora;                    //variavel que captura tempo real
String datta;                   //variavel que captura a data atual
int erroTime=0;                 //reseta o esp32 quando tiver o valor 3 ou maior
int erroTimeFirebase=0;         //reseta o esp32 quando tiver o valor 3 ou maior

//variaveis de controle dos temporizadores
String hora_off1 = "99:99:00";
String hora_off2 = "99:99:00";
String hora_off3 = "99:99:00";
String hora_off4 = "99:99:00";
String hora_off5 = "99:99:00";
String hora_off6 = "99:99:00";
String hora_off7 = "99:99:00";
String hora_off8 = "99:99:00";
String hora_off9 = "99:99:00";
String hora_off10 = "99:99:00";

int modoAtual;  //Variavel para salvar o modo atual configurado (Manual - 0 / Automatico - 1)

double tensao = 220; //V
double potencia; //W
double Irms; //A

int leituraLDR;
void leitura_dados_server();
String consultaBD (String registro);                //função para capturar dados do firebase
void atualizaTemp();

//para controle do led interno
bool led_int = true;

//função de temporização watchdog
hw_timer_t *timer = NULL, *timer_atualiza = NULL; //faz o controle do temporizador (interrupção por tempo)
void IRAM_ATTR resetModule(){
    Serial.println("interrupção reiniciar........");
    ets_printf("(watchdog) reiniciar\n"); //imprime no log
    
    digitalWrite(pinResetEN,LOW);
    //ESP.restart();
}

//Configurações
void setup() {
  analogReadResolution(10); //Resolução precisa ser alterada para se adequar a enomlib.h
  //Serial.begin(115200);
  Serial.begin(115200);
  pinMode(lamp1, OUTPUT);
  pinMode(lamp2, OUTPUT);
  pinMode(lamp3, OUTPUT);
  pinMode(porta1, OUTPUT);
  digitalWrite(pinResetEN,HIGH);
  pinMode(pinResetEN, OUTPUT);
  
  irsend1.begin();
  irsend2.begin();
  servoPe.attach(persiana);
  pinMode(resetEsp, INPUT);
  pinMode(ledInterrupt, OUTPUT);
  digitalWrite(ledInterrupt, led_int);

  //Aqui vai funcionar como servidor e esperar se conectar a rede
  wifiManager.setTimeout(180); // 3 minutos para confirmar os dado se não reseta
  if (!wifiManager.autoConnect(nome_AP, senha_AP)) {
    Serial.println("Falha ao conectar... Reiniciando...");
    delay(5000);
    digitalWrite(pinResetEN,LOW);
    //ESP.restart(); //reseta e tenta de novo
    //delay(5000);
  }
  Serial.println("Conectado a rede sem fio");

  //Consigurações da bilbioteca NTP
  //timeClient.begin();
  //timeClient.setTimeOffset(-10800); //GMT-3

  //Conexão com firebase
  Firebase.begin(FIREBASE_SERVER_HOST,FIREBASE_SERVER_AUTH);
  Firebase.reconnectWiFi(true);

  int estado=0;
  Firebase.getInt(meuBanco, "Lâmpada 1/Status",&estado);
  Serial.println("LAMP1: "+String(estado));
  estado= (estado==1)? 0 : 1;
  digitalWrite(lamp1,estado);
  
  Firebase.getInt(meuBanco, "Lâmpada 2/Status",&estado);
  Serial.println("LAMP2: "+String(estado));
  estado= (estado==1)? 0 : 1;
  digitalWrite(lamp2,estado);

  Firebase.getInt(meuBanco, "Persiana/Status",&estado);
  Serial.println("PERSIANA: "+String(estado));
  if(estado==0){ 
    servoPe.write(anguloMin);
  }
  if(estado==1){
    servoPe.write(anguloMax);
  }

  if (digitalRead(resetEsp) == HIGH){ 
      Serial.println("Resentando configurações salvas da rede WiFi");
      wifiManager.resetSettings();
      delay(200);
      Serial.println("Reniciando Esp... (botão)");
      delay(5000);
      digitalWrite(pinResetEN,LOW);
      //ESP.restart();
  }
  
  TC.current(analog, 6);  // N2 / R_carga = 6.0606

  //Em caso de reset do Esp, os temporizadores receberam a ultima informção salva no banco de dados
  hora_off1 = consultaBD("Temporizadores/Temp1");
  hora_off2 = consultaBD("Temporizadores/Temp2");
  hora_off3 = consultaBD("Temporizadores/Temp3");
  hora_off4 = consultaBD("Temporizadores/Temp4");
  hora_off5 = consultaBD("Temporizadores/Temp5");
  hora_off6 = consultaBD("Temporizadores/Temp6");
  hora_off7 = consultaBD("Temporizadores/Temp7");
  hora_off8 = consultaBD("Temporizadores/Temp8");
  hora_off9 = consultaBD("Temporizadores/Temp9");
  hora_off10 = consultaBD("Temporizadores/Temp10");

  //configuração timer para atualizar a hora e a data
  timer_atualiza= timerBegin(1, 80, true); //timerID 1, div 80
  
  //configuração timer para watchdog
  timer = timerBegin(0, 80, true); //timerID 0, div 80
  //timer, callback, interrupção de borda
  timerAttachInterrupt(timer, &resetModule, true);
  //timer, tempo (us), repetição
  timerAlarmWrite(timer, 40000000, true);
  timerAlarmEnable(timer); //habilita a interrupção

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  atualizaTemp();
  Serial.println("\n\n\t\t\tLINC v6\n\n");
}

void loop(){
   //segurança para que não fique energizada a porta
   digitalWrite(porta1, LOW);

   //reseta o temporizador (alimenta o watchdog) zera o contador
   timerWrite(timer, 0); 
   
  //Reseta a rede WiFi quando se quiser mudar para outra rede ou mudar a senha
  if (digitalRead(resetEsp) == HIGH){ 
      Serial.println("Resentando configurações salvas da rede WiFi...");
      Firebase.setString(meuBanco,"LINC Esp32/Status","RESET");  
      wifiManager.resetSettings();
      delay(200);
      Serial.println("Reniciando Esp... (botão)");
      delay(5000);
      digitalWrite(pinResetEN,LOW);
      //ESP.restart();
  }
    
  if(timerReadSeconds(timer_atualiza)>=15){
    atualizaTemp();
    timerWrite(timer_atualiza, 0); 
  }
  //Faz a leitura
  Irms = TC.calcIrms(1480);   // Calcula o valor da Corrente com 1480 amostras
  potencia = Irms * tensao;   // Calcula o valor da Potencia Instantanea 

  //quando a hora é XX:Y0:00, isto é atualiza a cada 10 minutos
  if (hora[3]!=horaAnt[0]) {
    horaAnt=hora[3];
    //Serial.println("hora irms: "+hora);//mudar if do irms - com a att a cada 15 seg, n é legal essa ideia especifica de a cada 10mim estritamente.
    Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
    Firebase.pushFloat(banco,"TC/Irms", Irms);
    Firebase.pushFloat(banco,"TC/potencia",potencia);
    Firebase.pushString(banco,"TC/data",datta+" - "+hora.substring(0,5));
    Serial.print("Publicado: ");
    Serial.print(Irms);
    Serial.print(" A, ");
    Serial.print(potencia);
    Serial.println(" W. ");
    Firebase.begin(FIREBASE_SERVER_HOST,FIREBASE_SERVER_AUTH);

  } 
  
  //Verifica se a hora atual é igual aos temporizadores para desligar os aparelhos de ar
  if ((hora.substring(0,5) == hora_off1.substring(0,5)) || (hora.substring(0,5) == hora_off2.substring(0,5)) || (hora.substring(0,5) == hora_off3.substring(0,5)) || (hora.substring(0,5) == hora_off4.substring(0,5)) || (hora.substring(0,5) == hora_off5.substring(0,5)) || (hora.substring(0,5) == hora_off6.substring(0,5)) || (hora.substring(0,5) == hora_off7.substring(0,5)) || (hora.substring(0,5) == hora_off8.substring(0,5)) || (hora.substring(0,5) == hora_off9.substring(0,5)) || (hora.substring(0,5) == hora_off10.substring(0,5))){
    //aparelhos de ar
    if(hora.substring(0,5) != horaAnt2){
      irsend1.sendRaw(desligaAR1, sizeof(desligaAR1) / sizeof(desligaAR1[0]), 38);//Manda o vetor de ligar 38kHz.
      delay(500);
      irsend1.sendRaw(desligaAR1, sizeof(desligaAR1) / sizeof(desligaAR1[0]), 38);//Manda o vetor de ligar 38kHz.
      delay(500);
      irsend1.sendRaw(desligaAR1, sizeof(desligaAR1) / sizeof(desligaAR1[0]), 38);//Manda o vetor de ligar 38kHz.
      Firebase.setInt(meuBanco,arc1,0);
  
      irsend2.sendRaw(desliga, 115, 38);//Manda o vetor de ligar 38kHz.
      delay(500);
      irsend2.sendRaw(desliga, 115, 38);//Manda o vetor de ligar 38kHz.
      delay(500);
      irsend2.sendRaw(desliga, 115, 38);//Manda o vetor de ligar 38kHz.
      Firebase.setInt(meuBanco,arc2,0);
      Serial.println("Aparelhos de ar desligados no horário: "+hora.substring(0,5));
      horaAnt2=hora.substring(0,5);
    }
  }

  if (modoAtual){
      //Faz a leitura do ldr e calcula angulo do motor
      leituraLDR = analogRead(ldr);  //varia de 0 - 1024
      int angulo_motor = (((anguloMax - anguloMin) * leituraLDR) / 1023) + anguloMin; 
      servoPe.write(angulo_motor);
  }
  //Verificador de travamentos
  led_int = !led_int;
  digitalWrite(ledInterrupt, led_int);
  delay(100);
  leitura_dados_server();
}

//Função de leitura de dados do Firebase
void leitura_dados_server(){
  String comando="";
  
  Firebase.getString(meuBanco,"LINC");
  comando=meuBanco.to<const char *>();

  //trava o ESP e reseta ele ↓
  if(strcmp(comando.c_str(),"trava")==0){
    Serial.println("vai travar- ");  
    Firebase.set(meuBanco,"LINC","0");
    while (true){
      Serial.println("travando");
      delay(500);
    }
  }
  //trava o ESP e reseta ele ↑

  //reseta ESP ↓
  if(strcmp(comando.c_str(),"resetNTP")==0){
    Serial.println("vai resetar NTP");  
    Firebase.set(meuBanco,"LINC","0");
    delay(2000);
    digitalWrite(pinResetEN,LOW);
  }
  //reseta ESP ↑
  
  //LAMPADA 1
  if(strcmp(comando.c_str(),"LAMPADA 1_1")==0){
    Serial.print("LÂMPADA 1 - ");  
    //digitalWrite(lamp1, HIGH);
    digitalWrite(lamp1, LOW);           //relé ligação normalmente fechada
    Serial.println("LIGADO");       
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,lampadas1,1); 
  }
  if(strcmp(comando.c_str(),"LAMPADA 1_0")==0){
    Serial.print("LÂMPADA 1 - ");
    digitalWrite(lamp1, HIGH); 
    //digitalWrite(lamp1, LOW);         //relé ligação normalmente fechada
    Serial.println("DESLIGADO");
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,lampadas1,0); 
  }
  
  //LAMPADA 2
  if(strcmp(comando.c_str(),"LAMPADA 2_1")==0){
    Serial.print("LÂMPADA 2 - ");  
    //digitalWrite(lamp2, HIGH);
    digitalWrite(lamp2, LOW);           //relé ligação normalmente fechada
    Serial.println("LIGADO");   
    Firebase.set(meuBanco,"LINC","0"); 
    Firebase.setInt(meuBanco,lampadas2,1);    
  }
  if(strcmp(comando.c_str(),"LAMPADA 2_0")==0){
    Serial.print("LÂMPADA 2 - ");
    digitalWrite(lamp2, HIGH);          //relé ligação normalmente fechada
    //digitalWrite(lamp2, LOW);
    Serial.println("DESLIGADO");
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,lampadas2,0); 
  }
  
  //LAMPADA 3
  if(strcmp(comando.c_str(),"LAMPADA 3_1")==0){
    Serial.print("LÂMPADA 3 - ");  
    //digitalWrite(lamp3, HIGH);
    digitalWrite(lamp3, LOW);
    Serial.println("LIGADO");   
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,lampadas3,1);     
  }
  if(strcmp(comando.c_str(),"LAMPADA 3_0")==0){
    Serial.print("LÂMPADA 3 - ");
    digitalWrite(lamp3, HIGH); 
    //digitalWrite(lamp3, LOW);
    Serial.println("DESLIGADO");
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,lampadas3,0); 
  }
  
  //MODO MANUAL OU AUTOMÁTICO
  if(strcmp(comando.c_str(),"MODO_1")==0){
    Serial.print("MODO - "); 
    modoAtual = 1;
    Serial.println("LIGADO");  
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,modo,1); 
  }
  if(strcmp(comando.c_str(),"MODO_0")==0){
    Serial.print("MODO - "); 
    modoAtual = 0;
    Serial.println("DESLIGADO");  
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,modo,0); 
  }
  
  //PERSIANA
  if(!modoAtual){
    //Movimento da persiana
    if(strcmp(comando.c_str(),"PERSIANA_0")==0){
      Serial.print("PERSIANA - ");  
      servoPe.write(anguloMin);
      Serial.println("ÂNGULO MÍNIMO IMPLEMENTADO"); 
      Firebase.set(meuBanco,"LINC","0");
      Firebase.setInt(meuBanco,persi,0); 
    }
  }
  if(!modoAtual){
    //Movimento da persiana
    if(strcmp(comando.c_str(),"PERSIANA_1")==0){
      Serial.print("PERSIANA - ");  
      servoPe.write(anguloMax);
      Serial.println("ÂNGULO MÁXIMO IMPLEMENTADO"); 
      Firebase.set(meuBanco,"LINC","0");
      Firebase.setInt(meuBanco,persi,1); 
    }
  }

  //PORTA
  if(strcmp(comando.c_str(),"PORTA")==0){
    Serial.println("PORTA - ");
    Firebase.setInt(meuBanco,porta,1);
    Firebase.set(meuBanco,"LINC","0");   
    digitalWrite(porta1, HIGH);
    delay(1000);
    digitalWrite(porta1, LOW); 
    Firebase.setInt(meuBanco,porta,0); 
  }

  //AR-CONDICIONADO 1
  if(strcmp(comando.c_str(),"AR_1_1")==0){
    Serial.print("AR 1 - ");  
    irsend1.sendRaw(ligaAR1, sizeof(ligaAR1) / sizeof(ligaAR1[0]), 38);//Manda o vetor de ligar 38kHz.
    delay(500);
    irsend1.sendRaw(ligaAR1, sizeof(ligaAR1) / sizeof(ligaAR1[0]), 38);//Manda o vetor de ligar 38kHz.
    delay(500);
    irsend1.sendRaw(ligaAR1, sizeof(ligaAR1) / sizeof(ligaAR1[0]), 38);//Manda o vetor de ligar 38kHz.
    Serial.println("LIGADO");  
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,arc1,1); 
  }
  if(strcmp(comando.c_str(),"AR_1_0")==0){
    Serial.print("AR 1 - ");  
    irsend1.sendRaw(desligaAR1, sizeof(desligaAR1) / sizeof(desligaAR1[0]), 38);//Manda o vetor de desliga 38kHz.
    delay(500);
    irsend1.sendRaw(desligaAR1, sizeof(desligaAR1) / sizeof(desligaAR1[0]), 38);//Manda o vetor de desliga 38kHz.
    delay(500);
    irsend1.sendRaw(desligaAR1, sizeof(desligaAR1) / sizeof(desligaAR1[0]), 38);//Manda o vetor de desliga 38kHz.
    Serial.println("DESLIGADO"); 
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,arc1,0);  
  }

  //AR-CONDICIONADO 2
  if(strcmp(comando.c_str(),"AR_2_1")==0){
    Serial.print("AR 2 - "); 
    irsend2.sendRaw(liga, 259, 38);//Manda o vetor de ligar 38kHz.
    delay(500);
    irsend2.sendRaw(liga, 259, 38);//Manda o vetor de ligar 38kHz.
    delay(500);
    irsend2.sendRaw(liga, 259, 38);//Manda o vetor de ligar 38kHz.
    delay(500);
    irsend2.sendRaw(liga, 259, 38);//Manda o vetor de ligar 38kHz.
    delay(500);
    irsend2.sendRaw(liga, 259, 38);//Manda o vetor de ligar 38kHz.
    delay(500);
    irsend2.sendRaw(liga, 259, 38);//Manda o vetor de ligar 38kHz.
    Serial.println("LIGADO"); 
    Firebase.set(meuBanco,"LINC","0");
    Firebase.setInt(meuBanco,arc2,1);  
  }
  if(strcmp(comando.c_str(),"AR_2_0")==0){
    Serial.print("AR 2 - "); 
    irsend2.sendRaw(desliga, 115, 38);//Manda o vetor de desliga 38kHz.
    delay(500);
    irsend2.sendRaw(desliga, 115, 38);//Manda o vetor de desliga 38kHz.
    delay(500);
    irsend2.sendRaw(desliga, 115, 38);//Manda o vetor de desliga 38kHz.
    Serial.println("DESLIGADO"); 
    Firebase.set(meuBanco,"LINC","0"); 
    Firebase.setInt(meuBanco,arc2,0); 
  }
    
  //Temporizadores de desligue de aparelhos de ar
  if(((comando.indexOf("TEMPO_1"))!=-1)&&((comando.indexOf("TEMPO_10"))==-1)){
    hora_off1 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 1 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp1",hora_off1);  
  }
  if((comando.indexOf("TEMPO_2"))!=-1){
    hora_off2 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 2 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp2",hora_off2);  
  }
  if((comando.indexOf("TEMPO_3"))!=-1){
    hora_off3 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 3 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp3",hora_off3);  
  }
  if((comando.indexOf("TEMPO_4"))!=-1){
    hora_off4 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 4 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp4",hora_off4);  
  }
  if((comando.indexOf("TEMPO_5"))!=-1){
    hora_off5 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 5 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp5",hora_off5);  
  }
  if((comando.indexOf("TEMPO_6"))!=-1){
    hora_off6 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 6 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp6",hora_off6);  
  }
  if((comando.indexOf("TEMPO_7"))!=-1){
    hora_off7 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 7 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp7",hora_off7);  
  }
  if((comando.indexOf("TEMPO_8"))!=-1){
    hora_off8 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0"); 
    Serial.println("Tempo 8 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp8",hora_off8);  
  }
  if((comando.indexOf("TEMPO_9"))!=-1){
    hora_off9 = comando.substring(8);
    Firebase.set(meuBanco,"LINC","0");
    Serial.println("Tempo 9 alterado para: "+ comando.substring(8)); 
    Firebase.setString(meuBanco,"Temporizadores/Temp9",hora_off9);  
  }
  if((comando.indexOf("TEMPO_10"))!=-1){
    hora_off10 = comando.substring(9);
    Firebase.set(meuBanco,"LINC","0");
    Serial.println("Tempo 10 alterado para: "+ comando.substring(9));
    Firebase.setString(meuBanco,"Temporizadores/Temp10",hora_off10);  
  }
}

//Função usada para capturar os ultimo valor dos temporizadores salvo no banco de dados
String consultaBD (String registro){
  if (Firebase.getString(banco, registro)){
    Serial.print("Dado capturado: ");
    Serial.println(banco.stringData());
    return banco.stringData();
  }
  else{
    Serial.println ("Não foi possível capturar a informação do BD");
    Serial.println(banco.errorReason());
    return "";
  }
}

//Função para atualizar a hora e a data
void atualizaTemp(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Falha ao obter a hora");
    erroTime++;
    Serial.print("erroTime: ");
    Serial.println(erroTime);
    if(erroTime>=3){
      Serial.println("RESET");
      delay(1000);
      digitalWrite(pinResetEN,LOW);
      //ESP.restart();
    }
  }
  
  else{
    erroTime=0;
    //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    //Serial.println("printLocalTime");
    
    String dia= (String(timeinfo.tm_mday).length()>1) ? timeinfo.tm_mday : "0"+String(timeinfo.tm_mday);
    String mes= (String(timeinfo.tm_mon).length()>1) ? timeinfo.tm_mon+1 : "0"+String(timeinfo.tm_mon+1);
    datta= dia+"/"+mes+"/"+String(timeinfo.tm_year+1900);
    String seg= (String(timeinfo.tm_sec).length()>1) ? timeinfo.tm_sec : "0"+String(timeinfo.tm_sec);
    String min_= (String(timeinfo.tm_min).length()>1) ? timeinfo.tm_min : "0"+String(timeinfo.tm_min);
    int horaNum=timeinfo.tm_hour;
    switch(horaNum){
      case 0: 
        horaNum=21;
        break;
      case 1:
        horaNum=22;
        break;
      case 2:
        horaNum=23;
        break;
      default:
        horaNum=horaNum-3;
        break;
    }
    String hour_= (String(horaNum).length()>1) ? horaNum : "0"+String(horaNum);
    
    //Serial.println("hora: "+hour_);
    hora= hour_+":"+min_+":"+seg;
    Serial.println("Data: "+datta+"\t Hora: "+hora);
    if(Firebase.setString(meuBanco,"LINC Esp32/Status",datta+" - "+hora)){
      Serial.println("Sucesso ao atualizar a hora e data");
      erroTimeFirebase=0;
    } 
    else{
      erroTimeFirebase++;
      Serial.println("Erro ao tentar atualizar a hora e data");
      Serial.print("erroTimeFirebase: ");
      Serial.println(erroTimeFirebase);
      if(erroTimeFirebase>=3){
        Serial.println("RESET");
        delay(1000);
        digitalWrite(pinResetEN,LOW);
        //ESP.restart();
      }
    }
  }
}
