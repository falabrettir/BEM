
#include <WiFi.h>

const char* ssid = "xiaomiFelipe"; // Replace with your Wi-Fi SSID
const char* password = "12345678"; // Replace with your Wi-Fi password

WiFiServer server(80);

// Simulated power data arrays
float vetor_voltagem[5] = {230.0, 231.0, 229.5, 230.5, 232.0}; // Replace with actual voltage data
float vetor_corrente[5] = {2.0, 2.2, 2.1, 2.3, 2.4}; // Replace with actual current data

// Function to serve the main HTML page
void handleClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  if (request.indexOf("GET / ") >= 0) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Power Usage Graph</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
  <h1>Power Usage</h1>
  <canvas id="powerChart" width="400" height="200"></canvas>
  <script>
    const ctx = document.getElementById('powerChart').getContext('2d');
    const powerChart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [], // Time labels
        datasets: [{
          label: 'Power (W)',
          data: [], // Power data
          borderColor: 'rgba(75, 192, 192, 1)',
          backgroundColor: 'rgba(75, 192, 192, 0.2)',
          borderWidth: 1
        }]
      },
      options: {
        scales: {
          x: { title: { display: true, text: 'Time (s)' } },
          y: { title: { display: true, text: 'Power (W)' }, beginAtZero: true }
        }
      }
    });

    async function fetchData() {
      const response = await fetch('/data');
      const data = await response.json();
      powerChart.data.labels = data.labels;
      powerChart.data.datasets[0].data = data.power;
      powerChart.update();
    }

    setInterval(fetchData, 5000); // Update every 5 seconds
  </script>
</body>
</html>
)rawliteral");
  }
}

// Function to serve power data as JSON
void servePowerData(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();

  // Calculate power and prepare JSON data
  String jsonData = "{\"labels\": [1, 2, 3, 4, 5], \"power\": [";
  for (int i = 0; i < 5; i++) {
    float power = vetor_voltagem[i] * vetor_corrente[i];
    jsonData += String(power);
    if (i < 4) jsonData += ",";
  }
  jsonData += "]}";
  client.println(jsonData);
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("GET /data") >= 0) {
      servePowerData(client);
    } else {
      handleClient(client);
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}
