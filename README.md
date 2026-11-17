# SmartGarden IoT — Firmware ESP32

Firmware desarrollado en C/C++ (Arduino IDE) para la placa **ESP32 DevKit**, encargado de la lectura física de sensores ambientales, control de actuadores (relés) y transmisión de telemetría en tiempo real hacia **Firebase Realtime Database**.

---

## 🔌 Diagrama de Conexiones y Pines (Pinout)

| Componente | Tipo | Pin ESP32 | Descripción |
| :--- | :--- | :--- | :--- |
| **DHT11 / DHT22** | Sensor Digital | **GPIO 4 (D4)** | Sensor de Temperatura y Humedad Ambiental |
| **Fotoresistencia (LDR)** | Sensor Analógico | **GPIO 34 (ADC1)** | Medición de luminosidad / radiación solar (0 - 3.3V) |
| **Módulo Relé (Bomba)** | Actuador Digital | **GPIO 23 (D23)** | Control de encendido/apagado de bomba de agua |
| **LED de Estado** | Indicador Visual | **GPIO 2** | LED integrado (Encendido = WiFi conectado) |

---

## ⚙️ Configuración del Firmware

En el archivo `smartgarden_esp32.ino`, edita las siguientes líneas según tu entorno:

```cpp
// 1. Red Wi-Fi
const char* WIFI_SSID     = "TU_RED_WIFI";
const char* WIFI_PASSWORD = "TU_PASSWORD_WIFI";


// 2. Conexión Firebase Realtime Database
const char* FIREBASE_HOST = "https://smartgarden-web-react-iot-default-rtdb.firebaseio.com";


// 3. Aislamiento Multi-Usuario
// Configura aquí tu nombre de usuario de SmartGarden (ej. "sicosis3dfx")
const char* SMARTGARDEN_USUARIO = "sicosis3dfx";
```

---

## 📡 Protocolo de Comunicación en Tiempo Real

El firmware realiza peticiones HTTP con payload JSON hacia Firebase cada 2 segundos:

### Envío de Telemetría (PUT)
```text
PUT https://smartgarden-web-react-iot-default-rtdb.firebaseio.com/telemetria/<SMARTGARDEN_USUARIO>/actual.json
```
**Payload enviado:**
```json
{
  "temperatura": 24.5,
  "humedad": 55.0,
  "luz": 72.0,
  "nodo": "ESP32-SmartGarden-01"
}
```

### Consulta de Actuadores (GET)
El firmware consulta el estado de la bomba de agua y nebulizador fijados desde la aplicación Android o el panel Web para conmutar el relé físico correspondiente.

---

## 🛠️ Instrucciones de Flasheo (Arduino IDE)

1. Abre **Arduino IDE** (versión 2.x recomendada).
2. Ve a **Archivo** > **Preferencias** y en *Gestor de URLs Adicionales de Tarjetas* añade:
   ```text
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. En **Herramientas** > **Placa**, selecciona **ESP32 Dev Module**.
4. Conecta el ESP32 por USB y selecciona el puerto COM correspondiente.
5. Haz clic en **Subir (Upload)**.
6. Abre el **Monitor Serie** a `115200 baudios` para verificar el estado de conexión Wi-Fi y los envíos de telemetría.
