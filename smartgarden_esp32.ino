/*
 * ============================================================================
 * SmartGarden IoT - Firmware para ESP32
 * Ramo: Aplicaciones Móviles para IoT (TI3042)
 * Conexión Wi-Fi y Sincronización en Tiempo Real con Firebase Realtime Database
 * ============================================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>

// --- 1. CONFIGURACIÓN DE RED WI-FI ---
// Puedes usar la red de tu casa, del instituto o la zona Wi-Fi de tu celular
const char* WIFI_SSID     = "TU_RED_WIFI";
const char* WIFI_PASSWORD = "TU_PASSWORD_WIFI";

// --- 2. CONFIGURACIÓN DE FIREBASE REALTIME DATABASE ---
const char* FIREBASE_HOST = "https://smartgarden-web-react-iot-default-rtdb.firebaseio.com";
// Aislamiento Multi-Usuario: Configura aquí el nombre de usuario de tu cuenta de SmartGarden
const char* SMARTGARDEN_USUARIO = "sicosis3dfx";

// --- 3. PINES DE SENSORES Y ACTUADORES (ESP32 DevKit 3.3V) ---
const int PIN_SENSOR_DHT   = 4;   // Sensor de Temperatura/Humedad (DHT11 / DHT22)
const int PIN_SENSOR_LUZ   = 34;  // Fotoresistencia / LDR (Pin analógico ADC1)
const int PIN_RELE_BOMBA   = 23;  // Relé para la Bomba de Agua (Actuador)
const int PIN_LED_ESTADO   = 2;   // LED integrado para indicar estado de conexión

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_ENVIO_MS = 2000; // Enviar cada 2 segundos

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_RELE_BOMBA, OUTPUT);
  pinMode(PIN_LED_ESTADO, OUTPUT);
  digitalWrite(PIN_RELE_BOMBA, LOW); // Bomba apagada inicialmente
  digitalWrite(PIN_LED_ESTADO, LOW);

  Serial.println("\n[SmartGarden ESP32] Iniciando sistema...");
  conectarWiFi();
}

void loop() {
  // Verificar si la conexión Wi-Fi sigue activa
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(PIN_LED_ESTADO, LOW);
    conectarWiFi();
  }

  // Enviar telemetría periódica hacia Firebase
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO_MS) {
    ultimoEnvio = millis();

    // Lectura de sensores físicos (o valores de prueba)
    float temperatura = leerTemperatura();
    float humedad     = leerHumedad();
    float luz         = leerLuz();

    enviarTelemetriaFirebase(temperatura, humedad, luz);
    consultarActuadoresFirebase();
  }
}

// Conectar a la red Wi-Fi configurada
void conectarWiFi() {
  Serial.print("[Wi-Fi] Conectando a: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 25) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Wi-Fi] Conectado exitosamente.");
    Serial.print("[Wi-Fi] Dirección IP asignada: ");
    Serial.println(WiFi.localIP());
    digitalWrite(PIN_LED_ESTADO, HIGH); // LED encendido = conectado
  } else {
    Serial.println("\n[Wi-Fi] No se pudo conectar. Reintentando en el siguiente ciclo...");
  }
}

// Enviar datos en tiempo real a Firebase Realtime Database
void enviarTelemetriaFirebase(float temp, float hum, float luz) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(FIREBASE_HOST) + "/telemetria/" + String(SMARTGARDEN_USUARIO) + "/actual.json";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");

  // Construir el JSON de telemetría compatible con Web y Android
  String payload = "{";
  payload += "\"temperatura\":" + String(temp, 1) + ",";
  payload += "\"humedad\":" + String(hum, 1) + ",";
  payload += "\"luz\":" + String(luz, 1) + ",";
  payload += "\"nodo\":\"ESP32-SmartGarden-01\"";
  payload += "}";

  int httpCode = http.PUT(payload);

  if (httpCode > 0) {
    Serial.print("[Firebase RTDB] Telemetría enviada -> Temp: ");
    Serial.print(temp);
    Serial.print(" °C | Hum: ");
    Serial.print(hum);
    Serial.print(" % | Luz: ");
    Serial.print(luz);
    Serial.println(" %");
  } else {
    Serial.print("[Firebase Error] Código HTTP: ");
    Serial.println(httpCode);
  }

  http.end();
}

// Consultar si la App o la Web activaron la bomba de agua
void consultarActuadoresFirebase() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(FIREBASE_HOST) + "/telemetria/" + String(SMARTGARDEN_USUARIO) + "/actuadores/bombaAgua.json";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String respuesta = http.getString();
    bool encenderBomba = (respuesta == "true");

    digitalWrite(PIN_RELE_BOMBA, encenderBomba ? HIGH : LOW);
    if (encenderBomba) {
      Serial.println("[Actuador] ¡BOMBA DE AGUA ACTIVADA DESDE LA APP/WEB!");
    }
  }

  http.end();
}

// --- FUNCIONES DE LECTURA DE SENSORES ---
float leerTemperatura() {
  // Si tienes conectado un sensor DHT11/DHT22 real, usa: dht.readTemperature();
  // De lo contrario, genera un valor base:
  return 24.5 + (random(-15, 15) / 10.0);
}

float leerHumedad() {
  // Sensor de humedad de suelo capacitivo o DHT: dht.readHumidity();
  return 55.0 + (random(-20, 20) / 10.0);
}

float leerLuz() {
  // Lectura analógica de fotoresistencia LDR (0 a 4095 en ESP32):
  int rawLuz = analogRead(PIN_SENSOR_LUZ);
  float porcentajeLuz = map(rawLuz, 0, 4095, 0, 100);
  return porcentajeLuz > 0 ? porcentajeLuz : 75.0;
}
