# Sistema de Monitoreo Ambiental para Oficinas Inteligentes

![Sistema IoT](https://i.imgur.com/your-banner-image.jpg)

## 📝 Descripción del Proyecto

Este proyecto implementa un sistema IoT completo para el monitoreo y control de asistencia en oficinas o contexto necesario, integrando sensores automatizados para mantener acciones óptimas de trabajo. El sistema recoge datos en tiempo real, los almacena en una base de datos, los visualiza en una Web y envía notificaciones cuando se detectan estodos fuera de lo deseado.

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
┌─────────────────┐       ┌─────────────────┐      ┌─────────────────┐
│                 │       │                 │      │                 │
│  ESP32          │       │    MQTT         │      │   Node-red      │
│  + Sensores     │─────▶│   Broker        │─────▶│   Backend       │
│  + Actuadores   │       │                 │      │                 │
└─────────────────┘       └─────────────────┘      └─────────┬───────┘
                                                             │
                                                             │
                         ┌─────────────────┐       ┌─────────▼────────┐
                         │                 │       │                  │
                         │   Sistema de    │◀───── │   Postgres      │
                         │   Notificaciones│       │                  │
                         │                 │       │                  │
                         └─────────────────┘       └──────────────────┘
                                                          ▲
                                                          │
                                                  ┌───────┴─────────┐
                                                  │                 │
                                                  │   Node-red      │
                                                  │   Web           │
                                                  │                 │
                                                  └─────────────────┘
```

## 🚀 Configuración y Uso

### Requisitos Previos
- Arduino IDE con soporte para ESP32
- Node-red 
- Postgres
- Cuenta en Apps Scripts
- Cliente MQTT (como Mosquitto)


## 📊 Interfaz de Usuario

La interfaz web permite:
- Agregar **Usuarios, clases, asistencias, aulas, horarios**
- Ver históricos de *Asistencias por usuario*
- Controlar manualmente el agregar, eliminar o actualizar *credenciales* del **usuario**


![Web Screenshot](https://github.com/user-attachments/assets/0f37cad6-085f-47d8-9d1b-0b83ee4f6087)


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

El sistema utiliza Postgres para almacenar:

### Colección: `readings`
```json

                Listado de relaciones
 Esquema |        Nombre         |   Tipo    | 
---------+-----------------------+-----------+-
 public  | asistencias           | tabla     | 
 public  | asistencias_id_seq    | secuencia | 
 public  | aulas                 | tabla     | 
 public  | aulas_id_seq          | secuencia | 
 public  | clase_alumno          | tabla     | 
 public  | clase_alumno_id_seq   | secuencia | 
 public  | clase_profesor        | tabla     | 
 public  | clase_profesor_id_seq | secuencia | 
 public  | clases                | tabla     | 
 public  | clases_id_seq         | secuencia | 
 public  | horarios              | tabla     | 
 public  | horarios_id_seq       | secuencia | 
 public  | usuarios              | tabla     | 
 public  | usuarios_id_seq       | secuencia | 
 public  | v_id_horario          | tabla     | 

```

## 📱 Sistema de Notificaciones

El sistema envía alertas por dos canales:

### Correo Electrónico
Utilizando App Scripts, se envían correos con formato HTML que incluyen:
- Estado del sensor
- Fecha de suceso

### Hoja de calculo de Google
En la hoja se guarda en dos columnas:
- Fecha de cambio de *estado*
- Estado del sensor

## 🔌 Integración de Sensores y Actuadores

### Sensores
1. **Huella digital**: Detecta y maneja un CRUD básico de imagenes de huellas.
2. **RFID**: Hace lectura de chips RFC.
3. **HC-SR501**: Detecta la presencia termoestática en un rango de 3 mts cada 50 segundos aprox.

### Actuadores
1. **Reloj de siete segmentos**: Proporciona la hora de un servidor DNS de horas globales.
2. **Buzzer pasico**: Maneja advertencia en forma de frecuencia de las acciones realizadas.
3. **LED RGB**: Proporciona indicación visual del estado del sistema:
   - Primero: En conjunto de los demás forma un estado del sistema.
   - Segundo: En conjunto de los demás forma un estado del sistema.
   - Tercero: En conjunto de los demás forma un estado del sistema.

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

## 👥 Contribuciones

Las contribuciones son bienvenidas. Por favor, crea un fork del repositorio, realiza tus cambios y envía un pull request para su revisión.

## 📄 Licencia

Este proyecto está licenciado bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para más detalles.

---

## Autoevaluaciones

# Zahir Andrés Rodríguez Mora
- Email: rodriguez.mora.zahir.15@gmail.com
- GitHub: [ZahirAndres](https://github.com/ZahirAndres)

### ¿Qué hice bien?
- 


### ¿Qué hice mal?
- 

### ¿Qué puedo mejorar?
-

# Cesar Enrique Garay García
- Email: tu.email@ejemplo.com
- GitHub: [tu-usuario](https://github.com/tu-usuario)

### ¿Qué hice bien?
- 


### ¿Qué hice mal?
- 

### ¿Qué puedo mejorar?
- 
