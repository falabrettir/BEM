#include <WiFi.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ACS712XX.h>
#include <ZMPT101B.h>
#include "time.h"

const char* ssid = "xiaomiFelipe";
const char* password = "12345678";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

WifiServer server(80);

String header;
#define tam_vetor 300

static int pos = 0;
static int flag = 0;
static int comeco = 0;
static int hora_atual;
static int min_atual;

//Inicializacao de cada tomada

//Tomada 1;

int pino_tensao_1 = 0;
int pino_corrente_1 = 0;
int pino_rele_1 = 32;

int hora_ligar_1 = -1;
int min_ligar_1 = -1;
int hora_desligar_1 = -1;
int min_desligar_1 = -1;

ZMPT101B sensor_tensao_1(pino_tensao_1);
ACS712XX sensor_corrente_1(ACS712_30A,pino_corrente_1);

float tensao_1;
float corrente_1;
float potencia_1;

float vetor_tensao_1[tam_vetor];
float vetor_corrente_1[tam_vetor];
float vetor_potencia_1[tam_vetor];

const char * arquivo_tensao_1 = "/arquivo_tensao_1.txt";
const char * arquivo_corrente_1 = "/arquivo_corrente_1.txt";
const char * arquivo_potencia_1 = "/arquivo_potencia_1.txt";

//Tomada 2;

int pino_tensao_2 = 0;
int pino_corrente_2 = 0;
int pino_rele_2 = 33;

int hora_ligar_2 = -1;
int min_ligar_2 = -1;
int hora_desligar_2 = -1;
int min_desligar_2 = -1;

ZMPT101B sensor_tensao_2(pino_tensao_2);
ACS712XX sensor_corrente_2(ACS712_30A,pino_corrente_2);

float tensao_2;
float corrente_2;
float potencia_2;

float vetor_tensao_2[tam_vetor];
float vetor_corrente_2[tam_vetor];
float vetor_potencia_2[tam_vetor];

const char * arquivo_tensao_2 = "/arquivo_tensao_2.txt";
const char * arquivo_corrente_2 = "/arquivo_corrente_2.txt";
const char * arquivo_potencia_2 = "/arquivo_potencia_2.txt";

//Tomada 3;

int pino_tensao_3 = 0;
int pino_corrente_3 = 0;
int pino_rele_3 = 26;

int hora_ligar_3 = -1;
int min_ligar_3 = -1;
int hora_desligar_3 = -1;
int min_desligar_3 = -1;

ZMPT101B sensor_tensao_3(pino_tensao_3);
ACS712XX sensor_corrente_3(ACS712_30A,pino_corrente_3);

float tensao_3;
float corrente_3;
float potencia_3;

float vetor_tensao_3[tam_vetor];
float vetor_corrente_3[tam_vetor];
float vetor_potencia_3[tam_vetor];

const char * arquivo_tensao_3 = "/arquivo_tensao_3.txt";
const char * arquivo_corrente_3 = "/arquivo_corrente_3.txt";
const char * arquivo_potencia_3 = "/arquivo_potencia_3.txt";

//Tomada 4;

int pino_tensao_4 = 0;
int pino_corrente_4 = 0;
int pino_rele_4 = 27;

int hora_ligar_4 = -1;
int min_ligar_4 = -1;
int hora_desligar_4 = -1;
int min_desligar_4 = -1;

ZMPT101B sensor_tensao_4(pino_tensao_4);
ACS712XX sensor_corrente_4(ACS712_30A,pino_corrente_4);

float tensao_4;
float corrente_4;
float potencia_4;

float vetor_tensao_4[tam_vetor];
float vetor_corrente_4[tam_vetor];
float vetor_potencia_4[tam_vetor];

const char * arquivo_tensao_4 = "/arquivo_tensao_4.txt";
const char * arquivo_corrente_4 = "/arquivo_corrente_4.txt";
const char * arquivo_potencia_4 = "/arquivo_potencia_4.txt";

File file;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

//Metodo para pegar os horarios de ligar/desligar
String getParam(String header, String param) {
  int index = header.indexOf(param + "=");
  if (index == -1) return "";
  int start = index + param.length() + 1;
  int end = header.indexOf("&", start);
  if (end == -1) end = header.indexOf(" ", start);
  return header.substring(start, end);
}

//Metodos para utilizar o SD Card
void writeFile(fs::FS &fs, const char * path, double message){
  //Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.println(message)){
    //Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, double message){
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    //Serial.println("Failed to open file for appending");
    return;
  }
  if(file.println(message)){
    //Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void deleteFile(fs::FS &fs, const char *path) {
  //Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    //Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}
//Fim dos Metodos para utilizar o SD Card

void setup() {

  Serial.begin(9600);

  sensor_tensao_1.calibrate();
  sensor_tensao_2.calibrate();
  sensor_tensao_3.calibrate();
  sensor_tensao_4.calibrate();

  pinMode(pino_rele_1, OUTPUT);
  pinMode(pino_rele_2, OUTPUT);
  pinMode(pino_rele_3, OUTPUT);
  pinMode(pino_rele_4, OUTPUT);

  hora_ligar_1 = -1;
  min_ligar_1 = -1;
  hora_desligar_1 = -1;
  min_desligar_1 = -1;

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }

  for(int i = 0; i < tam_vetor; i++){
    vetor_tensao_1[i] = -1;
    vetor_corrente_1[i] = -1;
    vetor_potencia_1[i] = -1;

    vetor_tensao_2[i] = -1;
    vetor_corrente_2[i] = -1;
    vetor_potencia_2[i] = -1;

    vetor_tensao_3[i] = -1;
    vetor_corrente_3[i] = -1;
    vetor_potencia_3[i] = -1;

    vetor_tensao_4[i] = -1;
    vetor_corrente_4[i] = -1;
    vetor_potencia_4[i] = -1;
  }
    
  deleteFile(SD, arquivo_tensao_1);
  deleteFile(SD, arquivo_corrente_1);
  deleteFile(SD, arquivo_potencia_1);

  writeFile(SD, arquivo_tensao_1, -1);
  writeFile(SD, arquivo_corrente_1, -1);
  writeFile(SD, arquivo_potencia_1, -1);

  deleteFile(SD, arquivo_tensao_2);
  deleteFile(SD, arquivo_corrente_2);
  deleteFile(SD, arquivo_potencia_2);

  writeFile(SD, arquivo_tensao_2, -1);
  writeFile(SD, arquivo_corrente_2, -1);
  writeFile(SD, arquivo_potencia_2, -1);

  deleteFile(SD, arquivo_tensao_3);
  deleteFile(SD, arquivo_corrente_3);
  deleteFile(SD, arquivo_potencia_3);

  writeFile(SD, arquivo_tensao_3, -1);
  writeFile(SD, arquivo_corrente_3, -1);
  writeFile(SD, arquivo_potencia_3, -1);

  deleteFile(SD, arquivo_tensao_4);
  deleteFile(SD, arquivo_corrente_4);
  deleteFile(SD, arquivo_potencia_4);

  writeFile(SD, arquivo_tensao_4, -1);
  writeFile(SD, arquivo_corrente_4, -1);
  writeFile(SD, arquivo_potencia_4, -1);

  for(int i = 0; i < tam_vetor; i++){
    appendFile(SD, arquivo_tensao_1, vetor_tensao_1[i]);
    appendFile(SD, arquivo_corrente_1, vetor_corrente_1[i]);
    appendFile(SD, arquivo_potencia_1, vetor_potencia_1[i]);

    appendFile(SD, arquivo_tensao_2, vetor_tensao_2[i]);
    appendFile(SD, arquivo_corrente_2, vetor_corrente_2[i]);
    appendFile(SD, arquivo_potencia_2, vetor_potencia_2[i]);

    appendFile(SD, arquivo_tensao_3, vetor_tensao_3[i]);
    appendFile(SD, arquivo_corrente_3, vetor_corrente_3[i]);
    appendFile(SD, arquivo_potencia_3, vetor_potencia_3[i]);

    appendFile(SD, arquivo_tensao_4, vetor_tensao_4[i]);
    appendFile(SD, arquivo_corrente_4, vetor_corrente_4[i]);
    appendFile(SD, arquivo_potencia_4, vetor_potencia_4[i]);
  }
  
}

void loop(){

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  hora_atual = timeinfo.tm_hour - 3;
  if(hora_atual < 0){
    hora_atual = 24 + timeinfo.tm_hour - 3;
  }
  min_atual = timeinfo.tm_min;

  //Tomada 1
  if (hora_atual == hora_ligar_1 && min_atual == min_ligar_1) {
    digitalWrite(pino_rele_1, HIGH);  
  }
  if (hora_atual == hora_desligar_1 && min_atual == min_desligar_1) {
    digitalWrite(pino_rele_1, LOW);  
  }

  //Tomada 2
  if (hora_atual == hora_ligar_2 && min_atual == min_ligar_2) {
    digitalWrite(pino_rele_2, HIGH);  
  }
  if (hora_atual == hora_desligar_2 && min_atual == min_desligar_2) {
    digitalWrite(pino_rele_2, LOW);  
  }

  //Tomada 3
  if (hora_atual == hora_ligar_3 && min_atual == min_ligar_3) {
    digitalWrite(pino_rele_3, HIGH);  
  }
  if (hora_atual == hora_desligar_3 && min_atual == min_desligar_3) {
    digitalWrite(pino_rele_3, LOW);  
  }

  // Tomada 4
  if (hora_atual == hora_ligar_4 && min_atual == min_ligar_4) {
    digitalWrite(pino_rele_4, HIGH);  
  }
  if (hora_atual == hora_desligar_4 && min_atual == min_desligar_4) {
    digitalWrite(pino_rele_4, LOW); 
  }

  //Tomada 1
  tensao_1 = sensor_tensao_1.getVoltageAC();
  tensao_1 *= 10;
  tensao_1 -= 2.5;
  tensao_1 = (float) ((int)(tensao_1 * 100)) /100;
  corrente_1 = sensor_corrente_1.getAC(60.0, 0.5);
  corrente_1 /= 10;
  corrente_1 = (float) ((int)(corrente_1 * 100)) /100;
  potencia_1 = tensao_1 * corrente_1;

  vetor_tensao_1[pos] = tensao_1;
  vetor_corrente_1[pos] = corrente_1;
  vetor_potencia_1[pos] = potencia_1;

  //Tomada 2
  tensao_2 = sensor_tensao_2.getVoltageAC();
  tensao_2 *= 10;
  tensao_2 -= 2.5;
  tensao_2 = (float) ((int)(tensao_2 * 100)) /100;
  corrente_2 = sensor_corrente_2.getAC(60.0, 0.5);
  corrente_2 /= 10;
  corrente_2 = (float) ((int)(corrente_2 * 100)) /100;
  potencia_2 = tensao_2 * corrente_2;

  vetor_tensao_2[pos] = tensao_2;
  vetor_corrente_2[pos] = corrente_2;
  vetor_potencia_2[pos] = potencia_2;

  //Tomada 3
  tensao_3 = sensor_tensao_3.getVoltageAC();
  tensao_3 *= 10;
  tensao_3 -= 2.5;
  tensao_3 = (float) ((int)(tensao_3 * 100)) /100;
  corrente_3 = sensor_corrente_3.getAC(60.0, 0.5);
  corrente_3 /= 10;
  corrente_3 = (float) ((int)(corrente_3 * 100)) /100;
  potencia_3 = tensao_3 * corrente_3;

  vetor_tensao_3[pos] = tensao_3;
  vetor_corrente_3[pos] = corrente_3;
  vetor_potencia_3[pos] = potencia_3;

  //Tomada 4
  tensao_4 = sensor_tensao_4.getVoltageAC();
  tensao_4 *= 10;
  tensao_4 -= 2.5;
  tensao_4 = (float) ((int)(tensao_4 * 100)) /100;
  corrente_4 = sensor_corrente_4.getAC(60.0, 0.5);
  corrente_4 /= 10;
  corrente_4 = (float) ((int)(corrente_4 * 100)) /100;
  potencia_4 = tensao_4 * corrente_4;

  vetor_tensao_4[pos] = tensao_4;
  vetor_corrente_4[pos] = corrente_4;
  vetor_potencia_4[pos] = potencia_4;

  pos++;
  if(pos == tam_vetor){
    flag = 1;
    pos = 0; 
    comeco = 1;
  }
  else if (flag){
    comeco++;
    if(comeco == tam_vetor){
      comeco = 0;
    }
  }

  //--------------------------------------------------Site--------------------------------------------------/
  WiFiClient client = server.available();   
  if (client) {                            
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");         
    String currentLine = "";               
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  
      currentTime = millis();
      if (client.available()) {            
        char c = client.read();           
        Serial.write(c);                   
        header += c;
        if (c == '\n') {                   
          if (currentLine.length() == 0) {
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:35%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:black; font-weight: bold; background-color: #e8e6a0; padding: 1px; }");
            
            // Web Page Headin
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
   
            client.println("<body><h1>B.E.M.</h1>");
            /*
            client.println("<h1>Tomada 1</h1>");
            client.println("<form action=\"/set-time\" method=\"GET\">");
            
            // Campos para os horários de ligar e desligar
            client.println("<h3>Tomada 1</h3>");
            client.println("Hora ligar: <input type=\"number\" name=\"hora_ligar_1\" min=\"0\" max=\"23\"><br>");
            client.println("Minuto ligar: <input type=\"number\" name=\"min_ligar_1\" min=\"0\" max=\"59\"><br>");
            client.println("Hora desligar: <input type=\"number\" name=\"hora_desligar_1\" min=\"0\" max=\"23\"><br>");
            client.println("Minuto desligar: <input type=\"number\" name=\"min_desligar_1\" min=\"0\" max=\"59\"><br><br>");

            client.println("<input type=\"submit\" value=\"Definir cronograma\">");
            client.println("</form>");

            client.println("<form action=\"/button-pressed-rele-1\" method=\"GET\"><button type=\"submit\" class=\"button\">Ligar/Desligar Tomada 1</button></form>");

            if (header.indexOf("GET /set-time") >= 0) {
              hora_ligar_1 = getParam(header, "hora_ligar_1").toInt();
              min_ligar_1 = getParam(header, "min_ligar_1").toInt();
              hora_desligar_1 = getParam(header, "hora_desligar_1").toInt();
              min_desligar_1 = getParam(header, "min_desligar_1").toInt();

              // Processar os outros horários (tomadas 2, 3 e 4) da mesma forma
              hora_ligar_2 = getParam(header, "hora_ligar_2").toInt();
              min_ligar_2 = getParam(header, "min_ligar_2").toInt();
              hora_desligar_2 = getParam(header, "hora_desligar_2").toInt();
              min_desligar_2 = getParam(header, "min_desligar_2").toInt();

              hora_ligar_3 = getParam(header, "hora_ligar_3").toInt();
              min_ligar_3 = getParam(header, "min_ligar_3").toInt();
              hora_desligar_3 = getParam(header, "hora_desligar_3").toInt();
              min_desligar_3 = getParam(header, "min_desligar_3").toInt();

              hora_ligar_4 = getParam(header, "hora_ligar_4").toInt();
              min_ligar_4 = getParam(header, "min_ligar_4").toInt();
              hora_desligar_4 = getParam(header, "hora_desligar_4").toInt();
              min_desligar_4 = getParam(header, "min_desligar_4").toInt();
            }

            if (header.indexOf("GET /button-pressed-rele-1") >= 0) {
              digitalWrite(pino_rele_1, !digitalRead(pino_rele_1));  // Alterna o estado
              Serial.println("Relé 1 Alterado");
            }

            client.println("</body></html>"); */

            client.println("<html><head><style>");
            client.println("body { font-family: Arial, sans-serif; margin: 20px; }");
            client.println(".form-container { display: flex; justify-content: space-between; flex-wrap: wrap; }");  // Flexbox com wrap para se ajustar à tela
            client.println(".form-item { border: 1px solid #ccc; padding: 10px; margin: 5px; width: 23%; box-sizing: border-box; }"); // Cada tomada ocupa 23% da largura
            client.println("h3 { color: #333; }");
            client.println("</style></head><body>");

            client.println("<h1>Controle de Tomadas</h1>");

            client.println("<form action=\"/set-time\" method=\"GET\">");  // Formulário para as 4 tomadas

            client.println("<div class=\"form-container\">");

            client.println("<div class=\"form-item\">");
            client.println("<h3>Tomada 1</h3>");
            client.print("<h2>Tensao: ");
            client.print(tensao_1);
            client.print(" V </h2>");
            client.print("<h2>Corrente: ");
            client.print(corrente_1);
            client.print(" A </h2>");
            client.print("<h2>Potencia: ");
            client.print(potencia_1);
            client.print(" W </h2>");
            client.println("<form action=\"/button-pressed-rele-1\" method=\"GET\"><button type=\"submit\" class=\"button\">Ligar/Desligar Tomada 1</button></form>");
            client.println("Hora ligar: <input type=\"number\" value=\"-1\" name=\"hora_ligar_1\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto ligar: <input type=\"number\" value=\"-1\" name=\"min_ligar_1\" min=\"-1\" max=\"59\"><br>");
            client.println("Hora desligar: <input type=\"number\" value=\"-1\" name=\"hora_desligar_1\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto desligar: <input type=\"number\" value=\"-1\" name=\"min_desligar_1\" min=\"-1\" max=\"59\"><br>");

            client.println("</div>");

            client.println("<div class=\"form-item\">");
            client.println("<h3>Tomada 2</h3>");
            client.print("<h2>Tensao: ");
            client.print(tensao_2);
            client.print(" V </h2>");
            client.print("<h2>Corrente: ");
            client.print(corrente_2);
            client.print(" A </h2>");
            client.print("<h2>Potencia: ");
            client.print(potencia_2);
            client.print(" W </h2>");
            client.println("<form action=\"/button-pressed-rele-2\" method=\"GET\"><button type=\"submit\" class=\"button\">Ligar/Desligar Tomada 2</button></form>");
            client.println("Hora ligar: <input type=\"number\" value=\"-1\" name=\"hora_ligar_2\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto ligar: <input type=\"number\" value=\"-1\" name=\"min_ligar_2\" min=\"-1\" max=\"59\"><br>");
            client.println("Hora desligar: <input type=\"number\" value=\"-1\" name=\"hora_desligar_2\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto desligar: <input type=\"number\" value=\"-1\" name=\"min_desligar_2\" min=\"-1\" max=\"59\"><br>");
            client.println("</div>");

            client.println("<div class=\"form-item\">");
            client.println("<h3>Tomada 3</h3>");
            client.print("<h2>Tensao: ");
            client.print(tensao_3);
            client.print(" V </h2>");
            client.print("<h2>Corrente: ");
            client.print(corrente_3);
            client.print(" A </h2>");
            client.print("<h2>Potencia: ");
            client.print(potencia_3);
            client.print(" W </h2>");
            client.println("<form action=\"/button-pressed-rele-3\" method=\"GET\"><button type=\"submit\" class=\"button\">Ligar/Desligar Tomada 3</button></form>");
            client.println("Hora ligar: <input type=\"number\" value=\"-1\" name=\"hora_ligar_3\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto ligar: <input type=\"number\" value=\"-1\" name=\"min_ligar_3\" min=\"-1\" max=\"59\"><br>");
            client.println("Hora desligar: <input type=\"number\" value=\"-1\" name=\"hora_desligar_3\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto desligar: <input type=\"number\" value=\"-1\" name=\"min_desligar_3\" min=\"-1\" max=\"59\"><br>");
            client.println("</div>");

            client.println("<div class=\"form-item\">");
            client.println("<h3>Tomada 4</h3>");
            client.print("<h2>Tensao: ");
            client.print(tensao_4);
            client.print(" V </h2>");
            client.print("<h2>Corrente: ");
            client.print(corrente_4);
            client.print(" A </h2>");
            client.print("<h2>Potencia: ");
            client.print(potencia_4);
            client.print(" W </h2>");
            client.println("<form action=\"/button-pressed-rele-4\" method=\"GET\"><button type=\"submit\" class=\"button\">Ligar/Desligar Tomada 4</button></form>");
            client.println("Hora ligar: <input type=\"number\" value=\"-1\" name=\"hora_ligar_4\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto ligar: <input type=\"number\" value=\"-1\" name=\"min_ligar_4\" min=\"-1\" max=\"59\"><br>");
            client.println("Hora desligar: <input type=\"number\" value=\"-1\" name=\"hora_desligar_4\" min=\"-1\" max=\"23\"><br>");
            client.println("Minuto desligar: <input type=\"number\" value=\"-1\" name=\"min_desligar_4\" min=\"-1\" max=\"59\"><br>");
            client.println("</div>");

              // Fecha o container das tomadas lado a lado

            client.println("<br><input type=\"submit\" value=\"Definir cronograma\">");  // Botão único para todas as tomadas
            client.println("</form>"); 
            
            
            client.println("</div>");

            if (header.indexOf("GET /set-time") >= 0) {
              hora_ligar_1 = getParam(header, "hora_ligar_1").toInt();
              min_ligar_1 = getParam(header, "min_ligar_1").toInt();
              hora_desligar_1 = getParam(header, "hora_desligar_1").toInt();
              min_desligar_1 = getParam(header, "min_desligar_1").toInt();

              // Processar os outros horários (tomadas 2, 3 e 4) da mesma forma
              hora_ligar_2 = getParam(header, "hora_ligar_2").toInt();
              min_ligar_2 = getParam(header, "min_ligar_2").toInt();
              hora_desligar_2 = getParam(header, "hora_desligar_2").toInt();
              min_desligar_2 = getParam(header, "min_desligar_2").toInt();

              hora_ligar_3 = getParam(header, "hora_ligar_3").toInt();
              min_ligar_3 = getParam(header, "min_ligar_3").toInt();
              hora_desligar_3 = getParam(header, "hora_desligar_3").toInt();
              min_desligar_3 = getParam(header, "min_desligar_3").toInt();

              hora_ligar_4 = getParam(header, "hora_ligar_4").toInt();
              min_ligar_4 = getParam(header, "min_ligar_4").toInt();
              hora_desligar_4 = getParam(header, "hora_desligar_4").toInt();
              min_desligar_4 = getParam(header, "min_desligar_4").toInt();
            }

            if (header.indexOf("GET /button-pressed-rele-1") >= 0) {
              digitalWrite(pino_rele_1, !digitalRead(pino_rele_1));  // Alterna o estado
              Serial.println("Relé 1 Alterado");
            }
            if (header.indexOf("GET /button-pressed-rele-2") >= 0) {
              digitalWrite(pino_rele_2, !digitalRead(pino_rele_2));  // Alterna o estado
              Serial.println("Relé 2 Alterado");
            }
            if (header.indexOf("GET /button-pressed-rele-3") >= 0) {
              digitalWrite(pino_rele_3, !digitalRead(pino_rele_3));  // Alterna o estado
              Serial.println("Relé 3 Alterado");
            }
            if (header.indexOf("GET /button-pressed-rele-4") >= 0) {
              digitalWrite(pino_rele_4, !digitalRead(pino_rele_4));  // Alterna o estado
              Serial.println("Relé 4 Alterado");
            }

            if(hora_ligar_1 != -1 && min_ligar_1 != -1){
              client.print("<h1>Tomada 1 - Hora Ligar: ");
              client.print(hora_ligar_1);
              client.print(":");
              client.print(min_ligar_1);
              client.print("<\h1>");
            }
            if(hora_desligar_1 != -1 && min_desligar_1 != -1){
              client.print("<h1>Tomada 1 - Hora Desligar: ");
              client.print(hora_desligar_1);
              client.print(":");
              client.print(min_desligar_1);
              client.print("<\h1>");
            }
            
            if(hora_ligar_2 != -1 && min_ligar_2 != -1){
              client.print("<h1>Tomada 2 - Hora Ligar: ");
              client.print(hora_ligar_2);
              client.print(":");
              client.print(min_ligar_2);
              client.print("<\h1>");
            }
            if(hora_desligar_2 != -1 && min_desligar_2 != -1){
              client.print("<h1>Tomada 2 - Hora Desligar: ");
              client.print(hora_desligar_2);
              client.print(":");
              client.print(min_desligar_2);
              client.print("<\h1>");
            }

            if(hora_ligar_3 != -1 && min_ligar_3 != -1){
              client.print("<h1>Tomada 3 - Hora Ligar: ");
              client.print(hora_ligar_3);
              client.print(":");
              client.print(min_ligar_3);
              client.print("<\h1>");
            }
            if(hora_desligar_3 != -1 && min_desligar_3 != -1){
              client.print("<h1>Tomada 3 - Hora Desligar: ");
              client.print(hora_desligar_3);
              client.print(":");
              client.print(min_desligar_3);
              client.print("<\h1>");
            }

            if(hora_ligar_4 != -1 && min_ligar_4 != -1){
              client.print("<h1>Tomada 4 - Hora Ligar: ");
              client.print(hora_ligar_4);
              client.print(":");
              client.print(min_ligar_4);
              client.print("<\h1>");
            }
            if(hora_desligar_4 != -1 && min_desligar_4 != -1){
              client.print("<h1>Tomada 4 - Hora Desligar: ");
              client.print(hora_desligar_4);
              client.print(":");
              client.print(min_desligar_4);
              client.print("<\h1>");
            }

            client.println("</body></html>");
            
            /*
            
            client.println("<form action=\"/button-pressed\" method=\"GET\">");
            client.println("<button type=\"submit\" class=\"button\">Relatorio 5 minutos!</button>");
            client.println("</form>");
            

            if (header.indexOf("GET /button-pressed") >= 0) {
                // Print to Serial Monitor when button is pressed
              deleteFile(SD, arquivo_tensao_1);
              deleteFile(SD, arquivo_corrente_1);
              deleteFile(SD, arquivo_potencia_1);

              deleteFile(SD, arquivo_tensao_2);
              deleteFile(SD, arquivo_corrente_2);
              deleteFile(SD, arquivo_potencia_2);

              deleteFile(SD, arquivo_tensao_3);
              deleteFile(SD, arquivo_corrente_3);
              deleteFile(SD, arquivo_potencia_3);

              deleteFile(SD, arquivo_tensao_4);
              deleteFile(SD, arquivo_corrente_4);
              deleteFile(SD, arquivo_potencia_4);

              writeFile(SD, arquivo_tensao_1, vetor_tensao_1[comeco]);
              writeFile(SD, arquivo_corrente_1, vetor_corrente_1[comeco]);
              writeFile(SD, arquivo_potencia_1, vetor_potencia_1[comeco]);
              
              writeFile(SD, arquivo_tensao_2, vetor_tensao_2[comeco]);
              writeFile(SD, arquivo_corrente_2, vetor_corrente_2[comeco]);
              writeFile(SD, arquivo_potencia_2, vetor_potencia_2[comeco]);

              writeFile(SD, arquivo_tensao_3, vetor_tensao_3[comeco]);
              writeFile(SD, arquivo_corrente_3, vetor_corrente_3[comeco]);
              writeFile(SD, arquivo_potencia_3, vetor_potencia_3[comeco]);

              writeFile(SD, arquivo_tensao_4, vetor_tensao_4[comeco]);
              writeFile(SD, arquivo_corrente_4, vetor_corrente_4[comeco]);
              writeFile(SD, arquivo_potencia_4, vetor_potencia_4[comeco]);

              for(int i = comeco + 1; i < tam_vetor; i++){
                if(vetor_tensao_1[i] >= 0 && vetor_corrente_1[i] >= 0 & vetor_potencia_1[i] >= 0){
                  appendFile(SD, arquivo_tensao_1, vetor_tensao_1[i]);
                  appendFile(SD, arquivo_corrente_1, vetor_corrente_1[i]);
                  appendFile(SD, arquivo_potencia_1, vetor_potencia_1[i]);
                }

                if(vetor_tensao_2[i] >= 0 && vetor_corrente_2[i] >= 0 & vetor_potencia_2[i] >= 0){
                  appendFile(SD, arquivo_tensao_2, vetor_tensao_2[i]);
                  appendFile(SD, arquivo_corrente_2, vetor_corrente_2[i]);
                  appendFile(SD, arquivo_potencia_2, vetor_potencia_2[i]);
                }

                if(vetor_tensao_3[i] >= 0 && vetor_corrente_3[i] >= 0 & vetor_potencia_3[i] >= 0){
                  appendFile(SD, arquivo_tensao_3, vetor_tensao_3[i]);
                  appendFile(SD, arquivo_corrente_3, vetor_corrente_3[i]);
                  appendFile(SD, arquivo_potencia_3, vetor_potencia_3[i]);
                }

                if(vetor_tensao_4[i] >= 0 && vetor_corrente_4[i] >= 0 & vetor_potencia_4[i] >= 0){
                  appendFile(SD, arquivo_tensao_4, vetor_tensao_4[i]);
                  appendFile(SD, arquivo_corrente_4, vetor_corrente_4[i]);
                  appendFile(SD, arquivo_potencia_4, vetor_potencia_4[i]);
                }
              }
              for(int i = 0; i < comeco; i++){
                if(vetor_tensao_1[i] >= 0 && vetor_corrente_1[i] >= 0 & vetor_potencia_1[i] >= 0){
                  appendFile(SD, arquivo_tensao_1, vetor_tensao_1[i]);
                  appendFile(SD, arquivo_corrente_1, vetor_corrente_1[i]);
                  appendFile(SD, arquivo_potencia_1, vetor_potencia_1[i]);
                }

                if(vetor_tensao_2[i] >= 0 && vetor_corrente_2[i] >= 0 & vetor_potencia_2[i] >= 0){
                  appendFile(SD, arquivo_tensao_2, vetor_tensao_2[i]);
                  appendFile(SD, arquivo_corrente_2, vetor_corrente_2[i]);
                  appendFile(SD, arquivo_potencia_2, vetor_potencia_2[i]);
                }

                if(vetor_tensao_3[i] >= 0 && vetor_corrente_3[i] >= 0 & vetor_potencia_3[i] >= 0){
                  appendFile(SD, arquivo_tensao_3, vetor_tensao_3[i]);
                  appendFile(SD, arquivo_corrente_3, vetor_corrente_3[i]);
                  appendFile(SD, arquivo_potencia_3, vetor_potencia_3[i]);
                }

                if(vetor_tensao_4[i] >= 0 && vetor_corrente_4[i] >= 0 & vetor_potencia_4[i] >= 0){
                  appendFile(SD, arquivo_tensao_4, vetor_tensao_4[i]);
                  appendFile(SD, arquivo_corrente_4, vetor_corrente_4[i]);
                  appendFile(SD, arquivo_potencia_4, vetor_potencia_4[i]);
                }
              }

              Serial.println("Leitura Tensao");
              readFile(SD, arquivo_tensao_1);
              Serial.println("Leitura Corrente");
              readFile(SD, arquivo_corrente_1);
              Serial.println("Leitura Potencia");
              readFile(SD, arquivo_potencia_1);
          
              Serial.println("Button Pressed!"); 
                
            }

            */

            client.println();
            break;
          } 
          else { 
            currentLine = "";
          }
        } 
        else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  delay(1000);
}
