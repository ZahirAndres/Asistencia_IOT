# Sistema de Monitoreo Ambiental para Oficinas Inteligentes

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
- Base de datos: **Postgres** (SQL)
- Comunicación: **MQTT** con broker Mosquitto
- Backend: **Node-Red**
- Frontend: **Node-Red** 
- Sistema de Notificaciones: **Email** y **Hoja de Cálculo** con Add Scripts

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

![circuit_image (25)](https://github.com/user-attachments/assets/9052da56-daf9-44a2-9079-e441b41bd1c7)


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


## 💾 Estructura de la Base de Datos

El sistema utiliza Postgres para almacenar:

### Estructura
```
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
- [Notificaciones](https://docs.google.com/spreadsheets/d/1Dc3OdmuSdST2gik-8zWmGDjFpg_3OnFmzgLU6YSqR9U/edit?usp=sharing)

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

## 👥 Videos e imágenes

Las archivos estan nombrados en orden cronológico [Multimedia](Pruebas/Evidencia_grafica/)

---

## 👥 Contribuciones

Las contribuciones son bienvenidas. Por favor, crea un fork del repositorio, realiza tus cambios y envía un pull request para su revisión.

---

## Autoevaluaciones

**Zahir Andrés Rodríguez Mora**  
_Email: rodriguez.mora.zahir.15@gmail.com_ • GitHub: [ZahirAndres](https://github.com/ZahirAndres)  

**¿Qué hice bien?**  
- Entregaste una integración limpia de los sensores (DHT22, MQ135 y LDR) en el ESP32, con lecturas estables y bien calibradas.  
- Configuraste correctamente el broker MQTT y los tópicos para que Node‑Red recibiera los datos en tiempo real.  
- Documentaste el diagrama de arquitectura de forma clara, mostrando el flujo hardware → MQTT → Node‑Red → Postgres y notificaciones.  

**¿Qué hice mal?**  
- La gestión de errores en el firmware del ESP32 es mínima: si un sensor falla o hay desconexión WiFi, no hay rutina de reconexión ni mensajes de diagnóstico.  
- No incluiste comentarios ni descripciones en el código de Arduino IDE, lo que dificulta su mantenimiento o la incorporación de más sensores.  
- No realizaste pruebas de campo (p. ej. variaciones de luz y temperatura) para comprobar la robustez de las lecturas.  

**¿Qué puedo mejorar?**  
- Añadir manejo de excepciones en el ESP32: reconexión automática al broker y mensajes de log cuando un sensor no responda.  
- Comentar y organizar tu sketch de Arduino, separando claramente las funciones de lectura, publicación MQTT y gestión de actuadores.  
- Realizar pruebas en diferentes condiciones ambientales y ajustar la frecuencia de muestreo o filtros de lectura para mayor estabilidad.  

---

**Cesar Enrique Garay García**  
_Email: cesarenriquegaraygarcia@gmial.com_ • GitHub: [Cesax69](https://github.com/Cesax69)  

**¿Qué hice bien?**  
- Diseñaste un esquema de base de datos Postgres muy completo, con tablas normalizadas para usuarios, aulas, asistencias y horarios.  
- Implementaste los flujos de Node‑Red para el backend y el frontend de manera modular, facilitando la reutilización de nodos.  
- Configuraste el envío de notificaciones por correo y la escritura en la hoja de cálculo de Google, cerrando el ciclo de monitoreo.  

**¿Qué hice mal?**  
- La interfaz web resultó algo rígida: los formularios no validan entradas (por ejemplo, fechas o IDs) y no hay mensajes de error claros.  
- No contemplaste ningún mecanismo de autenticación o control de acceso en Node‑Red; cualquiera que acceda al panel podría modificar datos críticos.  
- Falta de tests automáticos en los flujos de Node‑Red y en los scripts de Google Apps, lo que complica detectar fallos tras un cambio.  

**¿Qué puedo mejorar?**  
- Integrar un sistema de login (incluso básico con token JWT) para proteger el frontend y las APIs de Node‑Red.  
- Añadir validaciones en los formularios (requerir campos obligatorios, formatos de fecha, límites numéricos) y mostrar errores amigables al usuario.  
- Implementar pruebas de integración: por ejemplo, simular la llegada de mensajes MQTT y verificar que se almacenan y notifican correctamente.  

---

En conjunto, ambos aportaron piezas clave del sistema: Zahir en la capa de hardware y conexión IoT, y Cesar en la capa de datos, automatización y presentación. Si refinan la robustez del firmware, la seguridad y la usabilidad de la web, el proyecto dará un gran salto de calidad. ¡Excelente trabajo y a seguir mejorando!
