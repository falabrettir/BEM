#include <WiFi.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ACS712XX.h>
#include<ZMPT101B.h>


const char* ssid = "xiaomiFelipe";
const char* password = "12345678";

WiFiServer server(80);

String header;
#define tam_vetor 300
const int volt_pino = 34;
const int corr_pino = 35;
int voltagem = 0;
int corrente = 0;
static int pos = 0;
int flag = 0;
static int comeco = 0;
float vetor_voltagem[tam_vetor];
float vetor_corrente[tam_vetor];
const char *  voltagem1 = "/voltagem1.txt";
const char *  corrente1 = "/corrente1.txt";

File file;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
ZMPT101B voltageSensor(39);
ACS712XX ACS712(ACS712_30A,A0);

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


void setup() {

  Serial.begin(9600);
  //pinMode(volt_pino, INPUT);
  //pinMode(corr_pino, INPUT);
  
  float _offset = ACS712.autoCalibrate();   // Must ensure there no current passing through the sensor

  voltageSensor.calibrate();

  //--------------------------------------------------Site--------------------------------------------------/
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

  //--------------------------------------------------MicroSD--------------------------------------------------/
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
    vetor_corrente[i] = -1;
    vetor_voltagem[i] = -1;
  }
    
    deleteFile(SD, voltagem1);
    deleteFile(SD, corrente1);
    writeFile(SD, voltagem1, -1);
    writeFile(SD, corrente1, -1);
    for(int i = 0; i < tam_vetor; i++){
      appendFile(SD, voltagem1, vetor_corrente[i]);
      appendFile(SD, corrente1, vetor_voltagem[i]);
  }
}

void loop(){
  float corr = ACS712.getAC(60.0, 0.5);
  corr /= 10;
  float volt = voltageSensor.getVoltageAC();
  volt *= 10;
  volt -= 2.5;

  vetor_voltagem[pos] = volt;
  vetor_corrente[pos] = corr;

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

            client.print("<body><h3>Tensao: </h3>");
            client.print(volt);
            client.print(" V");
            client.println("<body><h3>Corrente: </h3>");
            client.println(corr);
            client.print(" A");
            client.println("<body><h3>Potencia: </h3>");
            client.println(corr * volt);
            client.print(" W");
            client.println("<br><br>");

            client.println("<form action=\"/button-pressed\" method=\"GET\">");
            client.println("<button type=\"submit\" class=\"button\">Relatorio 5 minutos!</button>");
            client.println("</form>");
            
            client.println("</body></html>");

            if (header.indexOf("GET /button-pressed") >= 0) {
                 // Print to Serial Monitor when button is pressed
                deleteFile(SD, voltagem1);
                deleteFile(SD, corrente1);
                writeFile(SD, voltagem1, vetor_voltagem[comeco]);
                writeFile(SD, corrente1, vetor_corrente[comeco]);

                for(int i = comeco + 1; i < tam_vetor; i++){
                  if(vetor_voltagem[i] >= 0 && vetor_corrente[i] >= 0){
                    appendFile(SD, voltagem1, vetor_voltagem[i]);
                    appendFile(SD, corrente1, vetor_corrente[i]);
                  }
                  
                }
                for(int i = 0; i < comeco; i++){
                  if(vetor_voltagem[i] >= 0 && vetor_corrente[i] >= 0){
                    appendFile(SD, voltagem1, vetor_voltagem[i]);
                    appendFile(SD, corrente1, vetor_corrente[i]);
                  }
                }

                Serial.println("Leitura Voltagem");
                readFile(SD, voltagem1);
                Serial.println("Leitura Corrente");
                readFile(SD, corrente1);
                Serial.println("Button Pressed!"); 
                
            }

            client.println();
            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  //--------------------------------------------------Site--------------------------------------------------/

  //--------------------------------------------------MicroSD--------------------------------------------------/
  /*
  deleteFile(SD, voltagem1);
  deleteFile(SD, corrente1);
  writeFile(SD, voltagem1, vetor_voltagem[comeco]);
  writeFile(SD, corrente1, vetor_corrente[comeco]);

  for(int i = comeco + 1; i < tam_vetor; i++){
    appendFile(SD, voltagem1, vetor_voltagem[i]);
    appendFile(SD, corrente1, vetor_corrente[i]);
  }
  for(int i = 0; i < comeco; i++){
    appendFile(SD, voltagem1, vetor_voltagem[i]);
    appendFile(SD, corrente1, vetor_corrente[i]);
  }
  Quando clicar no botão*/
  //--------------------------------------------------MicroSD--------------------------------------------------/
  delay(1000);
  //Serial.println("Passou um segundo");
  Serial.print("Voltagem:");
  Serial.println(volt);
  delay(100);
  Serial.print("Corrente:");
  Serial.println(corr);
  delay(100);
  Serial.print("Potência:");
  Serial.println(corr*volt);
  delay(100);
}