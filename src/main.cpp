
//Bibliotecas
#include <stdio.h>
#include <Arduino.h>
#include <math.h>
#include <Wifi.h>
#include <HTTPClient.h>

//Nome das portas
#define led_ir 19
#define led_verde 21
#define led_amarelo 22
#define led_vermelho 23
#define botao 34
#define sensor 33

//Conexão com servidor
const char* ID = "ESP32_001";
const char* ssid = "Luan OI FIBRA 2G";
const char* password = "marcianepyetro";
const char* serverurl = "http://192.168.100.70:5000/dados";

//Declaração das funcões usadas
void f_leitura_sensor();
void f_botao_calibracao();
void f_calibracao_0();
void f_calibracao_50();
bool f_click_botao();
void f_pontos_carregamento();

/*
Parâmetros:
P = Parâmetro.
U = Usuário.

PU001 = Parâmetro de concentração de 0%. 430
PU002 = Parâmetro de concentração de 50%. 505
PU003 = Parâmetro de concentração de 100%. 335
Parâmetros */ 
float PU001 = 730;
float PU002 = 780;
float leitura = 0;
float concentracao = 0;
bool modo_calibracao = false;
unsigned long tempo = 0;



//Funções padrão
void setup(){
    pinMode(led_ir, OUTPUT);
    pinMode(led_verde, OUTPUT);
    pinMode(led_amarelo, OUTPUT);
    pinMode(led_vermelho, OUTPUT);
    pinMode(botao, INPUT);
    pinMode(sensor, INPUT);

    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
    }

    Serial.println("Wifi conectado");
}

void loop(){
    f_leitura_sensor();
    Serial.println(leitura);


    //Calibração 
    f_botao_calibracao();
    
    if(modo_calibracao == true){
        f_calibracao_0();
    }
    //Calibração

    //Resposta do ESP com o servidor
    if((millis() - tempo) >= 5000000){
        if(WiFi.status() == WL_CONNECTED){
            HTTPClient http;
            http.begin(serverurl);
            http.addHeader("Content-Type", "application/json");
            String json = "{\"ID\": \""+ String(ID) +"\", \"temperatura\": " + String(leitura) +"}";
            int httpResponseCode = http.POST(json);
            Serial.println("Codigo de resposta:" + String(httpResponseCode));
            http.end();

            tempo = millis();
        }
    }
    //Resposta do ESP com o servidor
    
    delay(200);
}

//Funcões especiais
void f_leitura_sensor(){
    digitalWrite(led_ir, HIGH);
    delay(1000);

    for(int i = 0;i < 100;i ++){
        leitura = (leitura + analogRead(sensor));
        delay(5);
    }

    leitura = (leitura / 100);
    concentracao = (leitura - PU001) / ((PU002 - PU001) / 50);

    //digitalWrite(led_ir, LOW);
}


//Calibração
void f_botao_calibracao(){
    static unsigned long tempo_inicio;
    static bool botao_pressionado = false;

    if(digitalRead(botao) == true){
        if(botao_pressionado == false){
            tempo_inicio = millis();

            botao_pressionado = true;
        }

        if((millis() - tempo_inicio) >= 2000){
            Serial.println("Modo de calibracao");
            digitalWrite(led_vermelho, HIGH);

            while(digitalRead(botao) == true){
                delay(200);
            }
            modo_calibracao = true;
        }
    }

    else{
        tempo_inicio = 0;
        botao_pressionado = false;
        modo_calibracao = false;
    }
}

void f_calibracao_0(){
    bool concluido = false;

    Serial.println("Modo calibracao da solucao em 0% de oleo");
    Serial.println("Ao inserir a solucao de amostra no sensor, pressionar o botao de calibracao");

    while(concluido == false){
        f_click_botao();

        if(f_click_botao()){
            f_pontos_carregamento();
            f_leitura_sensor();

            PU001 = leitura;
            leitura = 0;

            Serial.print("Valor medido pelo sensor (PU001): ");
            Serial.println(PU001);
            delay(2000);

            concluido = true;
            f_calibracao_50();
        }
    }
}

void f_calibracao_50(){
    bool concluido = false;

    Serial.println("Modo calibracao da solucao em 50% de oleo");
    Serial.println("Ao inserir a solucao de amostra no sensor, pressionar o botao de calibracao");
    
    while(concluido == false){
        f_click_botao();

        if(f_click_botao()){
            f_pontos_carregamento();
            f_leitura_sensor();

            PU002 = leitura;
            leitura = 0;
                
            Serial.print("Valor medido pelo sensor (PU002): ");
            Serial.println(PU002);
            delay(3000);
            Serial.println("Fim da calibracao");

            digitalWrite(led_vermelho, LOW);

            concluido = true;
            modo_calibracao = false;
        }
    }
}
//Calibração


bool f_click_botao(){
    if(digitalRead(botao) == true){
        while(digitalRead(botao) == true){
            delay(200);
        }
        return true;
    }
    return false;

}


void f_pontos_carregamento(){
    for(int i = 0; i < 80; i ++){
        Serial.print("|");
        delay(100);
    }
    Serial.println("");
}

/*void Pisca_Led(){
    int tempo_high = 1;
    int tempo_low = 1;

    digitalWrite(led, HIGH);
    delay(tempo_high * 1000);

    digitalWrite(led, LOW);
    delay(tempo_low * 1000);
}
*/
