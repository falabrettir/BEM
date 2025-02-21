#include <WiFi.h>
#include <WiFiServer.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ACS712XX.h>
#include<ZMPT101B.h>
#include "time.h"

// Replace with your network credentials
const char* ssid = "Casa_5 2G";
const char* password = "Silvane1801";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

#define num_tomadas 4
#define tam_vetor 300

static int pos = 0;
static int flag = 0;
static int comeco = 0;
static int hora_atual;
static int min_atual;

bool dadosGravados = false;


const char * consumoEnergia = "/consumoEnergia.txt";

int pinos_tensao[num_tomadas] = {35, 34, 39, 36};
int pinos_corrente[num_tomadas] = {26, 27, 32, 33};
int pinos_rele[num_tomadas] = {15, 4, 16, 25};
int horas_ligar[num_tomadas] = {-1, -1, -1, -1};
int horas_desligar[num_tomadas] = {-1, -1, -1, -1}; 
int mins_ligar[num_tomadas] = {-1, -1, -1, -1};
int mins_desligar[num_tomadas] = {-1, -1, -1, -1};

float tensao[num_tomadas] = {-1, -1, -1, -1}; 
float corrente[num_tomadas] = {-1, -1, -1, -1}; 
float potencia[num_tomadas] = {-1, -1, -1, -1};


float vetor_energia_1[24] = {9.5, 9.0, 7.9, 6.3, 5.7, 8.2, 7.9, 9.1, 7.0, 9.5, 5.7, 6.6, 5.9, 9.8, 6.3, 7.8, 6.8, 5.4, 5.6, 7.9, 6.1, 9.6, 9.3, 5.9};
float vetor_energia_2[24] = {9.5, 9.0, 7.9, 6.3, 5.7, 8.2, 7.9, 9.1, 7.0, 9.5, 5.7, 6.6, 5.9, 9.8, 6.3, 7.8, 6.8, 5.4, 5.6, 7.9, 6.1, 9.6, 9.3, 5.9};
float vetor_energia_3[24] = {9.5, 9.0, 7.9, 6.3, 5.7, 8.2, 7.9, 9.1, 7.0, 9.5, 5.7, 6.6, 5.9, 9.8, 6.3, 7.8, 6.8, 5.4, 5.6, 7.9, 6.1, 9.6, 9.3, 5.9};
float vetor_energia_4[24] = {9.5, 9.0, 7.9, 6.3, 5.7, 8.2, 7.9, 9.1, 7.0, 9.5, 5.7, 6.6, 5.9, 9.8, 6.3, 7.8, 6.8, 5.4, 5.6, 7.9, 6.1, 9.6, 9.3, 5.9};

bool tomadaState[num_tomadas] = {false, false, false, false};

//Tomada 1;
ZMPT101B sensor_tensao_1(pinos_tensao[0]);
ACS712XX sensor_corrente_1(ACS712_30A,pinos_tensao[0]);

float vetor_tensao_1[tam_vetor];
float vetor_corrente_1[tam_vetor];
float vetor_potencia_1[tam_vetor];

const char * arquivo_tensao_1 = "/arquivo_tensao_1.txt";
const char * arquivo_corrente_1 = "/arquivo_corrente_1.txt";
const char * arquivo_potencia_1 = "/arquivo_potencia_1.txt";

//Tomada 2;
ZMPT101B sensor_tensao_2(pinos_tensao[1]);
ACS712XX sensor_corrente_2(ACS712_30A,pinos_tensao[1]);

float vetor_tensao_2[tam_vetor];
float vetor_corrente_2[tam_vetor];
float vetor_potencia_2[tam_vetor];

const char * arquivo_tensao_2 = "/arquivo_tensao_2.txt";
const char * arquivo_corrente_2 = "/arquivo_corrente_2.txt";
const char * arquivo_potencia_2 = "/arquivo_potencia_2.txt";

//Tomada 3;
ZMPT101B sensor_tensao_3(pinos_tensao[2]);
ACS712XX sensor_corrente_3(ACS712_30A,pinos_tensao[2]);

float vetor_tensao_3[tam_vetor];
float vetor_corrente_3[tam_vetor];
float vetor_potencia_3[tam_vetor];

const char * arquivo_tensao_3 = "/arquivo_tensao_3.txt";
const char * arquivo_corrente_3 = "/arquivo_corrente_3.txt";
const char * arquivo_potencia_3 = "/arquivo_potencia_3.txt";

//Tomada 4;
ZMPT101B sensor_tensao_4(pinos_tensao[3]);
ACS712XX sensor_corrente_4(ACS712_30A,pinos_tensao[3]);

float vetor_tensao_4[tam_vetor];
float vetor_corrente_4[tam_vetor];
float vetor_potencia_4[tam_vetor];

const char * arquivo_tensao_4 = "/arquivo_tensao_4.txt";
const char * arquivo_corrente_4 = "/arquivo_corrente_4.txt";
const char * arquivo_potencia_4 = "/arquivo_potencia_4.txt";

File file;

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
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

// Create a WiFi server on port 80 (HTTP)
WiFiServer server(80);

String arrayToJSArray(float arr[], int length) {
  String result = "[";
  for (int i = 0; i < length; i++) {
    result += String(arr[i], 2);  // Convert to string with 2 decimal places
    if (i < length - 1) {
      result += ", ";
    }
  }
  result += "]";
  return result;
}


void setup() {
  // Start the serial monitor
  Serial.begin(115200);

  sensor_tensao_1.calibrate();
  sensor_tensao_2.calibrate();
  sensor_tensao_3.calibrate();
  sensor_tensao_4.calibrate();

  pinMode(pinos_rele[0], OUTPUT);
  pinMode(pinos_rele[1], OUTPUT);
  pinMode(pinos_rele[2], OUTPUT);
  pinMode(pinos_rele[3], OUTPUT);
  
  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());  // Print the IP address

  // Start the server
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

   
    deleteFile(SD, consumoEnergia);
    writeFile(SD, consumoEnergia, "teste");



}

void loop() {

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

  // Check if a client has connected
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New Client Connected");
    String request = "";
    
    // Read the request from the client
    while (client.available()) {
      char c = client.read();
      request += c;
    }
    
    Serial.println(request);  // For debugging, print the request
    
    
    if (request.indexOf("GET /ligarTime") >= 0 || request.indexOf("GET /desligarTime") >= 0) {
      String type = (request.indexOf("desligarTime") >= 0) ? "desligarTime" : "ligarTime";

      int indexStart = request.indexOf("index=") + 6;
      int indexEnd = request.indexOf("&", indexStart);
      int index = request.substring(indexStart, indexEnd).toInt();

      int timeStart = request.indexOf("time=") + 5;
      String timeValue = request.substring(timeStart, request.indexOf(" ", timeStart));

      int separatorIndex = timeValue.indexOf(':');
      int hora = timeValue.substring(0, separatorIndex).toInt();
      int min = timeValue.substring(separatorIndex + 1).toInt();

      if(type == "ligarTime"){
        horas_ligar[index] = hora;
        mins_ligar[index] = min;
      }
      else{
        horas_desligar[index] = hora;
        mins_desligar[index] = min;
      }
    } 

    if (request.indexOf("/toggleState") >= 0) {
      // Parse the index and state from the request
      int indexStart = request.indexOf("index=") + 6;
      int indexEnd = request.indexOf("&", indexStart);
      int index = request.substring(indexStart, indexEnd).toInt();

      String estado = (request.indexOf("Ligar") >= 0) ? "Ligar" : "Desligar";
      // Update the boolean state
      if (estado == "Ligar") {
        tomadaState[index] = true;
        digitalWrite(pinos_rele[index], HIGH);

      } else if (estado == "Desligar") {
        tomadaState[index] = false;
        digitalWrite(pinos_rele[index], LOW);
      }

      /*
      // Print the updated state
      Serial.print("Tomada ");
      Serial.print(index + 1);
      Serial.print(" State Updated: ");
      Serial.println(tomadaState[index] ? "Ligar (true)" : "Desligar (false)");
      */
    }

    // Serve the HTML page
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>B.E.M.</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
        }
        canvas {
            max-width: 100%;
            margin: 20px auto;
            display: block;
        }
        .input-section {
            text-align: center;
            margin-top: 20px;
        }
        .result {
            text-align: center;
            margin-top: 10px;
            font-size: 1.2em;
        }
        .result strong {
            display: block;
            margin-top: 10px;
        }
        .toggle-section {
            display: flex;
            justify-content: space-around;
            margin-top: 20px;
        }
        .toggle-section .section {
            text-align: center;
            width: 20%;
        }
        .toggle-section button {
            padding: 10px 20px;
            font-size: 1em;
            cursor: pointer;
        }
        .time-inputs {
            margin-top: 10px;
            width: 100%;
        }
        .time-input-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 5px;
        }
        .time-input-row label {
            text-align: left;
            flex: 1;
        }
        .time-input-row input {
            text-align: right;
            flex: 1;
        }
        .time-display {
            margin-top: 10px;
            font-size: 1.2em;
        }
        .time-display-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .time-display-row span:first-child {
            text-align: left;
        }
        .time-display-row span:last-child {
            text-align: right;
        }
    </style>
</head>
<body>

    <h1 style="text-align: center;">Better Energy Management - B.E.M</h1>
    <canvas id="wattageChart" width="800" height="400"></canvas>

    <div class="input-section">
        <label for="multiplier">Calculo de gastos, insira sua tarifa (R$/kWh): </label>
        <input type="number" id="multiplier" step="0.01" placeholder="Utilize '.' como decimal">
        <button onclick="calculateSum()">Calcular</button>
    </div>

    <div id="result" class="result"></div>

    <div class="toggle-section" id="toggle-section">
        <!-- Each section will be dynamically inserted here for each Tomada -->
    </div>

    <div id="time-display" class="time-display"></div>

    <script>
        let toggleState = [false, false, false, false]; // Array to store the state of each button (4 buttons)
        let ligarTime = [null, null, null, null]; // Array to store ligar time for each outlet
        let desligarTime = [null, null, null, null]; // Array to store desligar time for each outlet

        // Function to toggle the variable
        function toggleVariable(index) {
            toggleState[index] = !toggleState[index]; // Toggle the state of the specific button
            const button = document.getElementById(`toggle-button-${index}`);
            button.textContent = toggleState[index] ? 'Ligar' : 'Desligar';

            const toggleValue = toggleState[index] ? 'Ligar' : 'Desligar';

            // Send the toggle state to the server
            fetch(`/toggleState?index=${index}&state=${toggleValue}`).then(response => {
              console.log(`Server response for toggleState:`, response.status);
            });
        }

        // Function to update time values and display the chosen time
        function updateTime(type, index) {
          const timeInput = document.getElementById(`${type}-time-${index}`).value;
          const [hours, minutes] = timeInput.split(':').map(Number);

          if (type === 'ligar') {
              ligarTime[index] = { hours, minutes };
          } else if (type === 'desligar') {
              desligarTime[index] = { hours, minutes };
          }

          updateTimeDisplay(index);

          // Send the updated time to the server
          const timeValue = `${hours}:${minutes.toString().padStart(2, '0')}`;
          fetch(`/${type}Time?index=${index}&time=${timeValue}`).then(response => {
              console.log(`Server response for ${type}Time:`, response.status);
          });
        }   

        // Function to update the time display for a specific outlet
        function updateTimeDisplay(index) {
            const timeDisplay = document.getElementById(`time-display-${index}`);
            timeDisplay.innerHTML = '';  // Clear previous content
            
            if (ligarTime[index]) {
                const formattedTime = `${ligarTime[index].hours}:${ligarTime[index].minutes.toString().padStart(2, '0')}`;
                const row = document.createElement('div');
                row.className = 'time-display-row';
                
                const label = document.createElement('span');
                label.textContent = 'Desligar:';
                row.appendChild(label);
                
                const time = document.createElement('span');
                time.textContent = formattedTime;
                row.appendChild(time);
                
                timeDisplay.appendChild(row);
            } else {
                const row = document.createElement('div');
                row.className = 'time-display-row';
                
                const label = document.createElement('span');
                label.textContent = 'Desligar:';
                row.appendChild(label);
                
                const time = document.createElement('span');
                time.textContent = 'Não selecionado';
                row.appendChild(time);
                
                timeDisplay.appendChild(row);
            }
            
            if (desligarTime[index]) {
                const formattedTime = `${desligarTime[index].hours}:${desligarTime[index].minutes.toString().padStart(2, '0')}`;
                const row = document.createElement('div');
                row.className = 'time-display-row';
                
                const label = document.createElement('span');
                label.textContent = 'Ligar:';
                row.appendChild(label);
                
                const time = document.createElement('span');
                time.textContent = formattedTime;
                row.appendChild(time);
                
                timeDisplay.appendChild(row);
            } else {
                const row = document.createElement('div');
                row.className = 'time-display-row';
                
                const label = document.createElement('span');
                label.textContent = 'Ligar:';
                row.appendChild(label);
                
                const time = document.createElement('span');
                time.textContent = 'Não selecionado';
                row.appendChild(time);
                
                timeDisplay.appendChild(row);
            }
        }

        // Create toggle buttons and time input sections for each outlet (Tomada 1 to 4)
        window.onload = function() {
            const toggleSection = document.getElementById('toggle-section');
            const outlets = ['Tomada 1', 'Tomada 2', 'Tomada 3', 'Tomada 4'];
            
            outlets.forEach((outlet, index) => {
                const div = document.createElement('div');
                div.classList.add('section');
                
                // Create the label for each Tomada
                const label = document.createElement('h3');
                label.textContent = `${outlet}`;
                div.appendChild(label);
                
                // Create the toggle button for each Tomada
                const button = document.createElement('button');
                button.id = `toggle-button-${index}`;
                button.textContent = 'Desligar';
                button.onclick = () => toggleVariable(index);
                div.appendChild(button);

                // Create time input section for ligar and desligar
                const timeInputs = document.createElement('div');
                timeInputs.classList.add('time-inputs');

                // Row for Ligar time
                const ligarRow = document.createElement('div');
                ligarRow.className = 'time-input-row';
                
                const ligarLabel = document.createElement('label');
                ligarLabel.textContent = 'Hora de Desligar:';
                ligarRow.appendChild(ligarLabel);
                
                const ligarInput = document.createElement('input');
                ligarInput.type = 'time';
                ligarInput.id = `ligar-time-${index}`;
                ligarInput.onchange = () => updateTime('ligar', index);
                ligarRow.appendChild(ligarInput);
                
                timeInputs.appendChild(ligarRow);

                // Row for Desligar time
                const desligarRow = document.createElement('div');
                desligarRow.className = 'time-input-row';
                
                const desligarLabel = document.createElement('label');
                desligarLabel.textContent = 'Hora de Ligar:';
                desligarRow.appendChild(desligarLabel);
                
                const desligarInput = document.createElement('input');
                desligarInput.type = 'time';
                desligarInput.id = `desligar-time-${index}`;
                desligarInput.onchange = () => updateTime('desligar', index);
                desligarRow.appendChild(desligarInput);
                
                timeInputs.appendChild(desligarRow);

                div.appendChild(timeInputs);

                // Create a section to display selected times
                const timeDisplay = document.createElement('div');
                timeDisplay.id = `time-display-${index}`;
                timeDisplay.classList.add('time-display');
                div.appendChild(timeDisplay);

                toggleSection.appendChild(div);
                
                // Initialize time display
                updateTimeDisplay(index);
            });
        }

        // Use the data from the C++ arrays
        const tomada1 = )rawliteral" + arrayToJSArray(vetor_energia_1, 24) + R"rawliteral(;
        const tomada2 = )rawliteral" + arrayToJSArray(vetor_energia_2, 24) + R"rawliteral(;
        const tomada3 = )rawliteral" + arrayToJSArray(vetor_energia_3, 24) + R"rawliteral(;
        const tomada4 = )rawliteral" + arrayToJSArray(vetor_energia_4, 24) + R"rawliteral(;

        const labels = ['00','01','02','03','04','05','06','07','08','09','10','11','12','13','14','15','16','17','18','19','20','21','22','23'];

        const ctx = document.getElementById('wattageChart').getContext('2d');
        const wattageChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [{
                    label: 'Tomada 1',
                    data: tomada1,
                    borderColor: 'rgba(255, 99, 132, 1)',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
             