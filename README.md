# Sistema de Monitoreo Ambiental para Oficinas Inteligentes

![Sistema IoT](https://i.imgur.com/your-banner-image.jpg)

## 📝 Descripción del Proyecto

Este proyecto implementa un sistema IoT completo para el monitoreo y control ambiental en oficinas, integrando sensores de temperatura, humedad y calidad del aire con actuadores automatizados para mantener condiciones óptimas de trabajo. El sistema recoge datos en tiempo real, los almacena en una base de datos, los visualiza en un dashboard web y envía notificaciones cuando se detectan valores fuera de los rangos deseados.

## ⚙️ Componentes del Sistema

### Hardware
- **Microcontrolador**: ESP32 (Programado con Arduino IDE)
- **Sensores**:
  - DHT22 (Temperatura y Humedad)
  - MQ135 (Calidad del Aire/CO2)
  - LDR (Nivel de Luz)
- **Actuadores**:
  - Relé para control de ventilador
  - Servo para control de persianas
  - LED RGB para indicación visual de estado

### Software
- Base de datos: **MongoDB Atlas** (NoSQL)
- Comunicación: **MQTT** con broker Mosquitto
- Backend: **Node.js** con Express
- Frontend: **React.js** con Material-UI
- Sistema de Notificaciones: **Email** (nodemailer) y **Telegram Bot**

## 🏗️ Arquitectura del Sistema

```
┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│                 │      │                 │      │                 │
│  ESP32 + DHT22  │      │    MQTT         │      │   Node.js       │
│  + MQ135 + LDR  │─────▶│   Broker        │─────▶│   Backend       │
│  + Actuadores   │      │                 │      │                 │
└─────────────────┘      └─────────────────┘      └────────┬────────┘
                                                          │
                                                          │
┌─────────────────┐      ┌─────────────────┐      ┌───────▼────────┐
│                 │      │                 │      │                 │
│   Telegram Bot  │◀─────│   Sistema de    │◀─────│   MongoDB      │
│   & Email       │      │   Notificaciones│      │   Atlas        │
│                 │      │                 │      │                 │
└─────────────────┘      └─────────────────┘      └─────────────────┘
                                                          ▲
                                                          │
                                                  ┌───────┴────────┐
                                                  │                 │
                                                  │   React.js      │
                                                  │   Dashboard     │
                                                  │                 │
                                                  └─────────────────┘
```

## 🚀 Configuración y Uso

### Requisitos Previos
- Arduino IDE con soporte para ESP32
- Node.js v14+ y npm
- Cuenta en MongoDB Atlas
- Cuenta en Telegram para crear un bot
- Cliente MQTT (como Mosquitto)

### Instalación

#### 1. Configuración del Hardware
```bash
# Abrir el archivo Arduino (en la carpeta /arduino)
esp32_sensors_control.ino
```

Conectar los componentes según el siguiente esquema:

| Componente | Pin ESP32 |
|------------|-----------|
| DHT22      | D4        |
| MQ135      | A0        |
| LDR        | A3        |
| Relé       | D22       |
| Servo      | D21       |
| LED RGB    | D15,D16,D17|

#### 2. Configuración del Backend
```bash
# Clonar el repositorio
git clone https://github.com/usuario/proyecto-iot-oficina.git
cd proyecto-iot-oficina/backend

# Instalar dependencias
npm install

# Configurar variables de entorno
cp .env.example .env
# Editar .env con tus credenciales
```

#### 3. Configuración del Frontend
```bash
# Navegar a la carpeta del frontend
cd ../frontend

# Instalar dependencias
npm install

# Configurar variables de entorno
cp .env.example .env
# Editar .env con la URL de tu backend

# Iniciar la aplicación en modo desarrollo
npm start
```

#### 4. Configuración del Broker MQTT
```bash
# En sistemas basados en Debian/Ubuntu
sudo apt-get install mosquitto mosquitto-clients

# Iniciar el servicio
sudo systemctl start mosquitto

# Verificar estado
sudo systemctl status mosquitto
```

## 📊 Interfaz de Usuario

La interfaz web permite:
- Visualizar datos en tiempo real de temperatura, humedad y calidad del aire
- Ver gráficos históricos de las últimas 24 horas
- Controlar manualmente los actuadores (ventilador, persianas)
- Configurar umbrales para disparar alertas
- Revisar el historial de notificaciones

![Dashboard Screenshot](https://i.imgur.com/your-dashboard-image.jpg)

## 🔄 Comunicación MQTT

El sistema utiliza MQTT para la comunicación entre el ESP32 y el backend. Los tópicos principales son:

| Tópico | Descripción |
|--------|-------------|
| `oficina/sensores/temperatura` | Lecturas del sensor de temperatura |
| `oficina/sensores/humedad` | Lecturas del sensor de humedad |
| `oficina/sensores/aire` | Lecturas del sensor de calidad de aire |
| `oficina/sensores/luz` | Lecturas del sensor de luz |
| `oficina/actuadores/ventilador` | Control del ventilador (ON/OFF) |
| `oficina/actuadores/persiana` | Control de la posición de persianas (0-100%) |
| `oficina/actuadores/led` | Control del LED RGB de estado |

## 💾 Estructura de la Base de Datos

El sistema utiliza MongoDB Atlas para almacenar:

### Colección: `readings`
```json
{
  "_id": "ObjectId",
  "timestamp": "ISODate",
  "temperature": 23.5,
  "humidity": 45.2,
  "airQuality": 280,
  "lightLevel": 820
}
```

### Colección: `alerts`
```json
{
  "_id": "ObjectId",
  "timestamp": "ISODate",
  "type": "temperature|humidity|air|light",
  "value": 27.8,
  "threshold": 26.0,
  "message": "Temperatura elevada detectada",
  "notified": true
}
```

### Colección: `settings`
```json
{
  "_id": "ObjectId",
  "tempThresholdMin": 18.0,
  "tempThresholdMax": 26.0,
  "humThresholdMin": 30.0,
  "humThresholdMax": 60.0,
  "airQualityThreshold": 500,
  "fanAutoControl": true,
  "blindsAutoControl": true
}
```

## 📱 Sistema de Notificaciones

El sistema envía alertas por dos canales:

### Correo Electrónico
Utilizando Nodemailer, se envían correos con formato HTML que incluyen:
- Tipo de alerta detectada
- Valor medido y umbral configurado
- Timestamp
- Acción automática realizada (si aplica)

### Bot de Telegram
Un bot de Telegram envía mensajes instantáneos cuando:
- Los valores de los sensores superan los umbrales configurados
- Se detecta un fallo en algún componente del sistema
- El sistema realiza una acción automática correctiva

## 🔌 Integración de Sensores y Actuadores

### Sensores
1. **DHT22**: Mide temperatura (-40°C a 80°C) y humedad (0-100%), con precisión de ±0.5°C y ±2% respectivamente.
2. **MQ135**: Sensor de calidad del aire que detecta CO2, NH3, NOx, alcohol, benceno, humo, etc.
3. **LDR (Light Dependent Resistor)**: Mide el nivel de luz ambiental para controlar la iluminación y persianas.

### Actuadores
1. **Módulo Relé**: Control del ventilador o sistema de aire acondicionado basado en umbrales de temperatura.
2. **Servo Motor MG90S**: Ajusta automáticamente la posición de las persianas según el nivel de luz.
3. **LED RGB**: Proporciona indicación visual del estado del sistema:
   - Verde: Condiciones ambientales normales
   - Amarillo: Condiciones cerca de los umbrales
   - Rojo: Alerta, valores fuera de rango
   - Azul parpadeante: Transmitiendo datos

## 📄 Código ESP32

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Definiciones de pines
#define DHTPIN 4
#define DHTTYPE DHT22
#define MQ135_PIN 36
#define LDR_PIN 39
#define RELAY_PIN 22
#define SERVO_PIN 21
#define RGB_R 15
#define RGB_G 16
#define RGB_B 17

// Configuración de red
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BROKER";
const int mqtt_port = 1883;
const char* mqtt_user = "YOUR_MQTT_USER";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";

// Inicialización de objetos
DHT dht(DHTPIN, DHTTYPE);
Servo blindServo;
WiFiClient espClient;
PubSubClient client(espClient);

// Variables globales
unsigned long lastMsgTime = 0;
const long interval = 5000; // Intervalo de envío de datos (5 segundos)

float temperature = 0;
float humidity = 0;
int airQuality = 0;
int lightLevel = 0;
bool fanStatus = false;
int blindPosition = 0;

void setup() {
  Serial.begin(115200);
  
  // Configuración de pines
  pinMode(MQ135_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(RGB_B, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW); // Ventilador apagado inicialmente
  
  // Inicialización de componentes
  dht.begin();
  blindServo.attach(SERVO_PIN);
  blindServo.write(0); // Posición inicial de persiana
  
  // Indicador visual de inicio
  setRGBColor(0, 0, 255); // Azul: Iniciando
  
  // Conectar a WiFi
  setup_wifi();
  
  // Configurar servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println("Conectando a WiFi...");
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    blinkRGB(0, 0, 255); // Parpadeo azul durante conexión
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
  
  setRGBColor(0, 255, 0); // Verde: WiFi conectado
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convertir payload a string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.println("Mensaje recibido [" + String(topic) + "]: " + message);
  
  // Procesar comandos recibidos
  if (String(topic) == "oficina/actuadores/ventilador") {
    if (message == "ON") {
      digitalWrite(RELAY_PIN, HIGH);
      fanStatus = true;
    } else if (message == "OFF") {
      digitalWrite(RELAY_PIN, LOW);
      fanStatus = false;
    }
  }
  else if (String(topic) == "oficina/actuadores/persiana") {
    int position = message.toInt();
    if (position >= 0 && position <= 100) {
      blindPosition = position;
      blindServo.write(map(blindPosition, 0, 100, 0, 180));
    }
  }
}

void reconnect() {
  // Bucle hasta reconectar
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    
    // Crear ID de cliente aleatorio
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Intentar conectar
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("conectado");
      
      // Suscribirse a tópicos de control
      client.subscribe("oficina/actuadores/ventilador");
      client.subscribe("oficina/actuadores/persiana");
      client.subscribe("oficina/actuadores/led");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      
      // Parpadeo rojo durante problema de conexión
      blinkRGB(255, 0, 0);
      delay(5000);
    }
  }
}

void loop() {
  // Comprobar conexión MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Leer sensores y publicar cada 'interval' ms
  unsigned long now = millis();
  if (now - lastMsgTime > interval) {
    lastMsgTime = now;
    
    // Indicador de lectura de sensores
    blinkRGB(0, 0, 255); // Azul parpadeante: Leyendo sensores
    
    // Leer sensores
    readSensors();
    
    // Publicar datos
    publishData();
    
    // Verificar umbrales y actualizar indicadores
    checkThresholds();
  }
}

void readSensors() {
  // Leer DHT22
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  
  // Comprobar lecturas DHT22 válidas
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error en la lectura del sensor DHT22!");
    return;
  }
  
  // Leer MQ135 (calidad del aire)
  airQuality = analogRead(MQ135_PIN);
  
  // Leer LDR (nivel de luz)
  lightLevel = analogRead(LDR_PIN);
  
  Serial.println("Temperatura: " + String(temperature) + "°C");
  Serial.println("Humedad: " + String(humidity) + "%");
  Serial.println("Calidad del aire: " + String(airQuality));
  Serial.println("Nivel de luz: " + String(lightLevel));
}

void publishData() {
  // Crear JSON para los datos de sensores
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["airQuality"] = airQuality;
  doc["lightLevel"] = lightLevel;
  doc["timestamp"] = millis();
  
  char buffer[200];
  serializeJson(doc, buffer);
  
  // Publicar en MQTT
  client.publish("oficina/sensores/datos", buffer);
  
  // Publicar valores individuales para compatibilidad
  client.publish("oficina/sensores/temperatura", String(temperature).c_str());
  client.publish("oficina/sensores/humedad", String(humidity).c_str());
  client.publish("oficina/sensores/aire", String(airQuality).c_str());
  client.publish("oficina/sensores/luz", String(lightLevel).c_str());
}

void checkThresholds() {
  // Ejemplo de lógica para verificar umbrales
  // En un sistema real, estos umbrales vendrían del servidor
  
  bool isAlertCondition = false;
  
  // Verificar temperatura
  if (temperature > 28) {
    // Encender ventilador automáticamente
    if (!fanStatus) {
      digitalWrite(RELAY_PIN, HIGH);
      fanStatus = true;
      client.publish("oficina/actuadores/ventilador", "ON");
    }
    isAlertCondition = true;
  } else if (temperature < 22) {
    // Apagar ventilador automáticamente
    if (fanStatus) {
      digitalWrite(RELAY_PIN, LOW);
      fanStatus = false;
      client.publish("oficina/actuadores/ventilador", "OFF");
    }
  }
  
  // Ajustar persianas según nivel de luz
  int newBlindPosition = map(lightLevel, 0, 4095, 100, 0);
  if (abs(newBlindPosition - blindPosition) > 10) { // Cambiar sólo si hay diferencia significativa
    blindPosition = newBlindPosition;
    blindServo.write(map(blindPosition, 0, 100, 0, 180));
    client.publish("oficina/actuadores/persiana", String(blindPosition).c_str());
  }
  
  // Actualizar LED RGB según condiciones
  if (isAlertCondition) {
    setRGBColor(255, 0, 0); // Rojo: Alerta
  } else if (temperature > 25 || humidity > 60 || airQuality > 1000) {
    setRGBColor(255, 255, 0); // Amarillo: Advertencia
  } else {
    setRGBColor(0, 255, 0); // Verde: Normal
  }
}

void setRGBColor(int r, int g, int b) {
  digitalWrite(RGB_R, r > 0 ? HIGH : LOW);
  digitalWrite(RGB_G, g > 0 ? HIGH : LOW);
  digitalWrite(RGB_B, b > 0 ? HIGH : LOW);
}

void blinkRGB(int r, int g, int b) {
  setRGBColor(r, g, b);
  delay(100);
  setRGBColor(0, 0, 0);
  delay(100);
  setRGBColor(r, g, b);
}
```

## 📡 API Backend

El backend expone los siguientes endpoints RESTful:

| Método | Endpoint | Descripción |
|--------|----------|-------------|
| GET    | `/api/readings/latest` | Obtiene la última lectura de sensores |
| GET    | `/api/readings/history/:hours` | Historial de lecturas (horas) |
| GET    | `/api/alerts` | Lista de alertas |
| POST   | `/api/controls/fan` | Control del ventilador |
| POST   | `/api/controls/blinds` | Control de persianas |
| GET    | `/api/settings` | Obtiene configuración |
| PUT    | `/api/settings` | Actualiza configuración |

## 🔒 Seguridad

- Autenticación JWT para el acceso a la API
- Conexión segura a MongoDB Atlas con usuario y contraseña
- Las credenciales y tokens se almacenan en variables de entorno
- La comunicación MQTT puede configurarse con TLS y autenticación

## 🔄 Flujo de Alertas

1. El ESP32 lee los sensores y envía datos por MQTT al broker
2. El backend recibe los datos y los almacena en MongoDB
3. Si algún valor supera los umbrales configurados:
   - Se registra una alerta en la base de datos
   - Se envía una notificación por email
   - Se envía un mensaje a través del bot de Telegram
4. La interfaz web actualiza el dashboard con la nueva alerta

## 📈 Futuras Mejoras

- Añadir soporte para múltiples zonas o habitaciones
- Implementar machine learning para predicción de condiciones
- Integración con Google Home o Amazon Alexa
- Añadir autenticación en dos factores
- Implementar comunicación MQTT segura con certificados TLS/SSL
- Añadir más tipos de sensores (CO2, movimiento, ruido)

## 👥 Contribuciones

Las contribuciones son bienvenidas. Por favor, crea un fork del repositorio, realiza tus cambios y envía un pull request para su revisión.

## 📄 Licencia

Este proyecto está licenciado bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para más detalles.

## 📧 Contacto

Para preguntas o sugerencias, por favor contacta a:
- Email: tu.email@ejemplo.com
- GitHub: [tu-usuario](https://github.com/tu-usuario)

---

## Autoevaluación

### ¿Qué hice bien?
- Implementación completa de la comunicación MQTT entre dispositivos
- Diseño responsive y funcional de la interfaz de usuario
- Sistema de notificaciones robusto con múltiples canales

### ¿Qué hice mal?
- Falta de pruebas automatizadas para el backend
- La documentación de la API podría ser más detallada
- El código del ESP32 podría estar mejor modularizado

### ¿Qué puedo mejorar?
- Implementar seguridad MQTT con TLS
- Añadir más tipos de sensores y actuadores
- Mejorar la eficiencia energética del ESP32
- Crear un sistema de registro de usuarios con diferentes niveles de acceso
