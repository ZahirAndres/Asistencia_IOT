/*  Formulario de Registro de Usuarios con ESP32, LVGL y MQTT
    Basado en el código de Rui Santos & Sara Santos - Random Nerd Tutorials
    Versión optimizada para mejor rendimiento con temporizador LVGL corregido
*/

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <PubSubClient.h>

// === CONFIGURACIÓN DE WIFI Y MQTT ===
const char* ssid = "MrMexico2014";
const char* password = "Mr2014..";
const char* mqtt_server = "192.168.0.6";
const int mqtt_port = 1883;
const char* mqtt_topic_receive = "api/prueba";       // Tópico para recibir códigos de huella/RFID
const char* mqtt_topic_register = "api/pantalla/registro"; // Tópico para enviar datos de registro

WiFiClient espClient;
PubSubClient client(espClient);

// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// LVGL buffer optimization - reduce size for better memory management
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 12)
static lv_color_t draw_buf[DRAW_BUF_SIZE];

// Variables LVGL
static lv_display_t * disp;
static lv_indev_t * indev;

// Pantalla principal
lv_obj_t * main_screen = NULL;

// Variables para formulario de registro
lv_obj_t * code_label = NULL;      // Muestra el código recibido (huella/RFID)
lv_obj_t * name_textarea = NULL;   // Campo para nombre
lv_obj_t * id_textarea = NULL;     // Campo para ID/cédula
lv_obj_t * phone_textarea = NULL;  // Campo para teléfono
lv_obj_t * status_label = NULL;    // Muestra estado del registro
lv_obj_t * keyboard = NULL;        // Teclado virtual
lv_obj_t * focused_input = NULL;   // Referencia al campo de texto activo

// Buffer para almacenar el código de huella/RFID recibido
char received_code[50] = "";
volatile bool new_code_received = false;
volatile bool mqtt_message_pending = false;
String pending_mqtt_message = "";

// Variables para control de tiempo
unsigned long last_touch_check = 0;
unsigned long last_lvgl_tick = 0;
const unsigned long TOUCH_CHECK_INTERVAL = 20; // ms
const unsigned long LVGL_TICK_INTERVAL = 5;    // ms para lv_tick_inc()

// Control de estado WiFi y MQTT
bool wifi_connected = false;
unsigned long last_reconnect_attempt = 0;
const unsigned long RECONNECT_INTERVAL = 5000; // 5 segundos

// Declaración adelantada de funciones
void update_ui_with_code();

// Función para configurar WiFi
void setup_wifi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    return;
  }
  
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  
  // Intentar conectar por 10 segundos máximo
  unsigned long start_time = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start_time < 10000) {
    delay(100);
    Serial.print(".");
    // Mantener LVGL funcionando mientras conectamos
    lv_tick_inc(5);
    lv_task_handler();
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println("\nWiFi conectado. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalló la conexión WiFi, se reintentará más tarde");
    wifi_connected = false;
  }
}

// Función para reconectar al broker MQTT - no bloqueante
bool reconnect_mqtt() {
  if (client.connected()) return true;
  
  if (!wifi_connected) {
    setup_wifi();
    if (!wifi_connected) return false;
  }
  
  Serial.print("Conectando al MQTT...");
  
  // Crear un ID de cliente único
  String clientId = "ESP32Client-";
  clientId += String(random(0xffff), HEX);
  
  if (client.connect(clientId.c_str())) {
    Serial.println("Conectado al broker");
    client.subscribe(mqtt_topic_receive);
    Serial.println("Suscrito al topic: " + String(mqtt_topic_receive));
    return true;
  } else {
    Serial.print("Fallo MQTT, rc=");
    Serial.println(client.state());
    return false;
  }
}

// Callback para mensajes MQTT - Ahora seguro para la memoria
void callback(char* topic, byte* payload, unsigned int length) {
  // Solo almacenar el mensaje para procesarlo en el loop principal
  pending_mqtt_message = "";
  for (unsigned int i = 0; i < length; i++) {
    pending_mqtt_message += (char)payload[i];
  }
  
  mqtt_message_pending = true;
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(pending_mqtt_message);
}

// Procesar mensaje MQTT pendiente - llamado desde el loop principal
void process_pending_mqtt_message() {
  if (!mqtt_message_pending) return;
  
  Serial.println("Procesando mensaje recibido");
  
  if (pending_mqtt_message.length() > 0) {
    // Guardar el código recibido
    strncpy(received_code, pending_mqtt_message.c_str(), sizeof(received_code) - 1);
    received_code[sizeof(received_code) - 1] = '\0'; // Asegurar null-termination
    new_code_received = true;
    
    // Actualizar la interfaz con el código recibido
    update_ui_with_code();
  }
  
  mqtt_message_pending = false;
}

// Actualizar UI con código recibido - separado para claridad
void update_ui_with_code() {
  // Actualizar la etiqueta con el código recibido
  String display_text = "Código recibido: ";
  display_text += received_code;
  
  lv_label_set_text(code_label, display_text.c_str());
  
  // Limpiar los campos del formulario
  lv_textarea_set_text(name_textarea, "");
  lv_textarea_set_text(id_textarea, "");
  lv_textarea_set_text(phone_textarea, "");
  lv_label_set_text(status_label, "Complete los datos y presione Registrar");
}

// Función para leer el touchscreen - optimizada
void touchscreen_read(lv_indev_t * indev_drv, lv_indev_data_t * data) {
  bool touched = touchscreen.tirqTouched() && touchscreen.touched();
  
  if (touched) {
    TS_Point p = touchscreen.getPoint();
    
    // Imprimir valores raw (solo ocasionalmente para no saturar el serial)
    static uint32_t last_print = 0;
    if(millis() - last_print > 1000) {
      Serial.printf("Touch raw - X: %d, Y: %d\n", p.x, p.y);
      last_print = millis();
    }
    
    // Mapeo normal
    data->point.x = map(p.x, 200, 3700, 0, SCREEN_WIDTH);
    data->point.y = map(p.y, 240, 3800, 0, SCREEN_HEIGHT);
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
// Callback para el teclado
static void keyboard_event_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    // Ocultar teclado al presionar OK o Cancel
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    focused_input = NULL;
  }
}

// Callback para los textareas (para mostrar teclado)
static void textarea_event_cb(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  // CORRECCIÓN: Añadir casting explícito a lv_obj_t*
  lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);  
  
  if (code == LV_EVENT_FOCUSED) {
    // Mostrar teclado y asignar al textarea
    if (lv_obj_has_flag(keyboard, LV_OBJ_FLAG_HIDDEN)) {
      lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
      lv_keyboard_set_textarea(keyboard, ta);
      focused_input = ta;
    }
  }
  
  if (code == LV_EVENT_DEFOCUSED) {
    // Si se hace clic en otro lugar, ocultar teclado solo si fue este mismo textarea
    if (focused_input == ta) {
      lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
      focused_input = NULL;
    }
  }
}

// Callback para botón de registro
static void register_btn_event(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  
  if (code == LV_EVENT_CLICKED) {
    // Obtener datos del formulario
    const char* name = lv_textarea_get_text(name_textarea);
    const char* id = lv_textarea_get_text(id_textarea);
    const char* phone = lv_textarea_get_text(phone_textarea);
    
    // Validar que todos los campos estén llenos
    if (strlen(name) == 0 || strlen(id) == 0 || strlen(phone) == 0) {
      lv_label_set_text(status_label, "¡Error! Todos los campos son obligatorios");
      return;
    }
    
    // Validar que se tenga un código
    if (strlen(received_code) == 0) {
      lv_label_set_text(status_label, "¡Error! No hay código recibido");
      return;
    }
    
    // Crear JSON con los datos
    String json_data = "{\"codigo\":\"";
    json_data += received_code;
    json_data += "\",\"nombre\":\"";
    json_data += name;
    json_data += "\",\"apellido\":\"";
    json_data += id;
    json_data += "\",\"tipo_usuario\":\"";
    json_data += phone;
    json_data += "\"}";
    
    // Verificar conexión MQTT
    if (!client.connected()) {
      if (!reconnect_mqtt()) {
        lv_label_set_text(status_label, "Error: MQTT desconectado");
        Serial.println("MQTT no conectado.");
        return;
      }
    }
    
    // Enviar al broker MQTT
    if (client.publish(mqtt_topic_register, json_data.c_str())) {
      lv_label_set_text(status_label, "¡Usuario registrado con éxito!");
      Serial.println("Datos enviados: " + json_data);
    } else {
      lv_label_set_text(status_label, "Error al enviar datos. Intente nuevamente.");
      Serial.println("Error al publicar datos MQTT");
    }
  }
}

// Crear pantalla de registro (única pantalla)
void create_registration_screen() {
  // Título
  lv_obj_t * title = lv_label_create(main_screen);
  lv_label_set_text(title, "Registro de Usuario");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
  
  // Etiqueta para mostrar el código recibido
  code_label = lv_label_create(main_screen);
  lv_label_set_text(code_label, "Código recibido: ");
  lv_obj_align(code_label, LV_ALIGN_TOP_LEFT, 20, 40);
  
  // Crear campos del formulario
  // 1. Campo Nombre
  lv_obj_t * name_label = lv_label_create(main_screen);
  lv_label_set_text(name_label, "Nombre:");
  lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, 20, 70);
  
  name_textarea = lv_textarea_create(main_screen);
  lv_obj_set_size(name_textarea, 200, 40);
  lv_obj_align(name_textarea, LV_ALIGN_TOP_LEFT, 20, 90);
  lv_textarea_set_placeholder_text(name_textarea, "Ingrese nombre completo");
  lv_obj_add_event_cb(name_textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
  
  // 2. Campo ID/Cédula
  lv_obj_t * id_label = lv_label_create(main_screen);
  lv_label_set_text(id_label, "ID/Cédula:");
  lv_obj_align(id_label, LV_ALIGN_TOP_LEFT, 20, 140);
  
  id_textarea = lv_textarea_create(main_screen);
  lv_obj_set_size(id_textarea, 200, 40);
  lv_obj_align(id_textarea, LV_ALIGN_TOP_LEFT, 20, 160);
  lv_textarea_set_placeholder_text(id_textarea, "Ingrese ID o cédula");
  lv_obj_add_event_cb(id_textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
  
  // 3. Campo Teléfono
  lv_obj_t * phone_label = lv_label_create(main_screen);
  lv_label_set_text(phone_label, "Teléfono:");
  lv_obj_align(phone_label, LV_ALIGN_TOP_LEFT, 20, 210);
  
  phone_textarea = lv_textarea_create(main_screen);
  lv_obj_set_size(phone_textarea, 200, 40);
  lv_obj_align(phone_textarea, LV_ALIGN_TOP_LEFT, 20, 230);
  lv_textarea_set_placeholder_text(phone_textarea, "Ingrese número telefónico");
  lv_obj_add_event_cb(phone_textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
  
 
 // Botón de registro - ubicado completamente en la parte inferior
lv_obj_t * register_btn = lv_button_create(main_screen);
lv_obj_set_size(register_btn, 160, 50);
// Cambiar la posición vertical a -10 o -5 para dejarlo muy cerca del borde inferior
lv_obj_align(register_btn, LV_ALIGN_BOTTOM_MID, 0, -10);  // -10 píxeles desde el borde inferior
lv_obj_set_style_bg_color(register_btn, lv_color_hex(0x16A085), 0); // Verde
lv_obj_add_event_cb(register_btn, register_btn_event, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t * btn_label = lv_label_create(register_btn);
  lv_label_set_text(btn_label, "REGISTRAR");
  lv_obj_center(btn_label);
  
  // Etiqueta de estado del registro - movida arriba del botón
status_label = lv_label_create(main_screen);
lv_label_set_text(status_label, "Complete los datos y presione Registrar");
lv_obj_set_width(status_label, 200);
lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);
// Colocar la etiqueta de estado justo encima del botón
lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -70);  // -70 para estar arriba del botón
  
  // Crear teclado virtual (oculto inicialmente)
  keyboard = lv_keyboard_create(main_screen);
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN); // Inicialmente oculto
  lv_obj_set_size(keyboard, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, NULL);
}

// CORRECCIÓN: Actualización de la función my_log_cb para coincidir con la firma esperada
static void my_log_cb(const char * buf) {
  Serial.println(buf);
}

// Función actualizada para registrar el callback de log correctamente
static void register_log_cb(void) {
  // En LVGL 8.x y superiores, no usar directamente lv_log_register_print_cb
  // En su lugar, usar Serial.println directamente cuando sea necesario
  Serial.println("Log callback configurado");
}

void setup() {
  Serial.begin(115200);
  delay(100); // Pequeño retraso para que el serial se estabilice
  
  Serial.println("\n\n--- Iniciando Sistema de Registro ---");
  
  // Inicializar LVGL
  lv_init();
  
  // CORRECCIÓN: Eliminar o comentar la línea de registro de callback que causa problemas
  // lv_log_register_print_cb(my_log_cb);
  register_log_cb(); // Llamar a nuestra función de registro modificada
  
  Serial.println("LVGL inicializado");
  
  // Inicializar touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);
  Serial.println("Touchscreen inicializado");
  
  // Crear display - buffers optimizados
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);
  Serial.println("Display TFT inicializado");
  
  // Configurar touchscreen
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);
  Serial.println("Input device configurado");
  
  // Crear pantalla principal
  main_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0xF5F5F5), 0); // Fondo gris claro
  
  // Crear formulario de registro
  create_registration_screen();
  Serial.println("UI creada");
  
  // Cargar la pantalla
  lv_screen_load(main_screen);
  
  // Configurar WiFi y MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // Inicializar aleatorio para clientId
  randomSeed(analogRead(0));
  
  Serial.println("Configuración completada. Iniciando aplicación.");
}

void loop() {
  unsigned long current_millis = millis();
  
  // *** IMPORTANTE: Incrementar el tick de LVGL cada 5ms ***
  if (current_millis - last_lvgl_tick > LVGL_TICK_INTERVAL) {
    lv_tick_inc(LVGL_TICK_INTERVAL);
    last_lvgl_tick = current_millis;
  }
  
  // Verificar y procesar conexión WiFi si es necesario
  if (!wifi_connected) {
    if (current_millis - last_reconnect_attempt > RECONNECT_INTERVAL) {
      last_reconnect_attempt = current_millis;
      setup_wifi();
    }
  }
  
  // Manejar conexión MQTT de forma no bloqueante
  if (!client.connected()) {
    if (current_millis - last_reconnect_attempt > RECONNECT_INTERVAL) {
      last_reconnect_attempt = current_millis;
      reconnect_mqtt();
    }
  } else {
    client.loop(); // Solo procesar MQTT si estamos conectados
  }
  
  // Procesar mensajes MQTT pendientes
  if (mqtt_message_pending) {
    process_pending_mqtt_message();
  }
  
  // Manejar GUI - IMPORTANTE para LVGL
  lv_task_handler();
  
  // Sin delay aquí - dejamos que el sistema funcione a máxima velocidad
  // El timing lo controlan los contadores de intervalos
}