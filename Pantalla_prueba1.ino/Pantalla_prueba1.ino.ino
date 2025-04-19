/*  Formulario de Registro de Usuarios con ESP32, LVGL y MQTT
    Basado en el código de Rui Santos & Sara Santos - Random Nerd Tutorials
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
const char* mqtt_topic_register = "api/pruebaEnvio"; // Tópico para enviar datos de registro

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

// Touchscreen coordinates
int x, y, z;

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Definir pantallas y navbar
lv_obj_t * screens[3];        // Array para almacenar las pantallas
lv_obj_t * navbar[3];         // Navbar para cada pantalla
lv_obj_t * nav_buttons[3][3]; // Botones de navegación (3 botones en cada una de las 3 pantallas)
uint8_t current_screen = 0;   // Pantalla actual

// Variables para formulario de registro
lv_obj_t * code_label;         // Muestra el código recibido (huella/RFID)
lv_obj_t * name_textarea;      // Campo para nombre
lv_obj_t * id_textarea;        // Campo para ID/cédula
lv_obj_t * phone_textarea;     // Campo para teléfono
lv_obj_t * status_label;       // Muestra estado del registro
lv_obj_t * keyboard;           // Teclado virtual
lv_obj_t * focused_input;      // Referencia al campo de texto activo

// Buffer para almacenar el código de huella/RFID recibido
char received_code[50] = "";
bool new_code_received = false;

// Ajustes de la UI
#define NAVBAR_HEIGHT 50
#define CONTENT_HEIGHT (SCREEN_HEIGHT - NAVBAR_HEIGHT)

// Función para configurar WiFi
void setup_wifi() {
  delay(10);
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: ");
  Serial.println(WiFi.localIP());
}

// Función para reconectar al broker MQTT
void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Conectando al MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado al broker");
      client.subscribe(mqtt_topic_receive);
      Serial.println("Suscrito al topic: " + String(mqtt_topic_receive));
    } else {
      Serial.print("Fallo MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" intentando en 5 segundos...");
      delay(5000);
    }
  }
}

// Callback para mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  if (String(topic) == mqtt_topic_receive) {
    // Guardar el código recibido
    strncpy(received_code, message.c_str(), sizeof(received_code) - 1);
    received_code[sizeof(received_code) - 1] = '\0'; // Asegurar null-termination
    new_code_received = true;
    
    // Actualizar la interfaz con el código recibido
    lv_timer_t * timer = lv_timer_create([](lv_timer_t * timer) {
      // Cambiar a la pantalla de registro
      lv_screen_load(screens[1]);
      current_screen = 1;
      
      // Actualizar la etiqueta con el código recibido
      String display_text = "Código recibido: ";
      display_text += received_code;
      lv_label_set_text(code_label, display_text.c_str());
      
      // Limpiar los campos del formulario
      lv_textarea_set_text(name_textarea, "");
      lv_textarea_set_text(id_textarea, "");
      lv_textarea_set_text(phone_textarea, "");
      lv_label_set_text(status_label, "Complete los datos y presione Registrar");
      
      delete timer;
    }, 0, NULL);
  }
}

// Función para registro de logs
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Función para leer el touchscreen
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// Función para cambiar de pantalla
static void change_screen(lv_event_t * e) {
  uint32_t screen_index = (uint32_t)((uintptr_t)lv_event_get_user_data(e));
  current_screen = screen_index;
  lv_screen_load(screens[current_screen]);
  Serial.print("Cambiando a pantalla: ");
  Serial.println(current_screen);
}

// Callback para el teclado
static void keyboard_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
lv_obj_t * kb = (lv_obj_t *)lv_event_get_target(e);  
  if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
    // Ocultar teclado al presionar OK o Cancel
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    focused_input = NULL;
  }
}

// Callback para los textareas (para mostrar teclado)
static void textarea_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
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
    // Si se hace clic en otro lugar, ocultar teclado
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
    
    // Crear JSON con los datos
    String json_data = "{\"codigo\":\"";
    json_data += received_code;
    json_data += "\",\"nombre\":\"";
    json_data += name;
    json_data += "\",\"id\":\"";
    json_data += id;
    json_data += "\",\"telefono\":\"";
    json_data += phone;
    json_data += "\"}";
    
    // Enviar al broker MQTT
    if (client.connected()) {
      if (client.publish(mqtt_topic_register, json_data.c_str())) {
        lv_label_set_text(status_label, "¡Usuario registrado con éxito!");
        Serial.println("Datos enviados: " + json_data);
      } else {
        lv_label_set_text(status_label, "Error al enviar datos. Intente nuevamente.");
        Serial.println("Error al publicar datos MQTT");
      }
    } else {
      lv_label_set_text(status_label, "Error: MQTT desconectado");
      Serial.println("MQTT no conectado.");
    }
  }
}

// Crear navbar común para todas las pantallas
void create_navbar(int screen_index) {
  // Crear el contenedor de la navbar
  navbar[screen_index] = lv_obj_create(screens[screen_index]);
  lv_obj_set_size(navbar[screen_index], SCREEN_WIDTH, NAVBAR_HEIGHT);
  lv_obj_set_pos(navbar[screen_index], 0, 0); // Posición arriba
  lv_obj_set_style_radius(navbar[screen_index], 0, 0);
  lv_obj_set_style_bg_color(navbar[screen_index], lv_color_hex(0x2C3E50), 0);
  lv_obj_set_style_border_width(navbar[screen_index], 0, 0);
  lv_obj_set_flex_flow(navbar[screen_index], LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(navbar[screen_index], LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  
  // Crear los tres botones de navegación
  const char * btn_labels[] = {"Inicio", "Registro", "Estado"};
  
  for (int i = 0; i < 3; i++) {
    nav_buttons[screen_index][i] = lv_button_create(navbar[screen_index]);
    lv_obj_set_size(nav_buttons[screen_index][i], 65, 40);
    lv_obj_add_event_cb(nav_buttons[screen_index][i], change_screen, LV_EVENT_CLICKED, (void *)i);
    
    // Configurar estilo según esté seleccionado o no
    if (i == screen_index) {
      lv_obj_set_style_bg_color(nav_buttons[screen_index][i], lv_color_hex(0x3498DB), 0); // Azul para botón activo
    } else {
      lv_obj_set_style_bg_color(nav_buttons[screen_index][i], lv_color_hex(0x34495E), 0); // Gris oscuro para inactivos
    }
    
    lv_obj_t * label = lv_label_create(nav_buttons[screen_index][i]);
    lv_label_set_text(label, btn_labels[i]);
    lv_obj_center(label);
  }
}

// Crear pantalla de inicio (Pantalla 0)
void create_home_screen() {
  // Crear la navbar para esta pantalla
  create_navbar(0);

  // Título principal - ajustado para estar debajo de la navbar
  lv_obj_t * welcome_title = lv_label_create(screens[0]);
  lv_label_set_text(welcome_title, "Sistema de Registro");
  lv_obj_set_style_text_font(welcome_title, &lv_font_montserrat_20, 0);
  lv_obj_align(welcome_title, LV_ALIGN_TOP_MID, 0, NAVBAR_HEIGHT + 20);
  
  // Subtítulo - ajustado para estar debajo del título
  lv_obj_t * welcome_subtitle = lv_label_create(screens[0]);
  lv_label_set_text(welcome_subtitle, "Acerque su huella o tarjeta RFID al lector");
  lv_obj_set_width(welcome_subtitle, 200);
  lv_obj_set_style_text_align(welcome_subtitle, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(welcome_subtitle, LV_ALIGN_TOP_MID, 0, NAVBAR_HEIGHT + 50);
  
  // Imagen o icono representativo
  lv_obj_t * icon_container = lv_obj_create(screens[0]);
  lv_obj_set_size(icon_container, 100, 100);
  lv_obj_align(icon_container, LV_ALIGN_CENTER, 0, 10);
  lv_obj_set_style_bg_color(icon_container, lv_color_hex(0x3498DB), 0);
  lv_obj_set_style_radius(icon_container, LV_RADIUS_CIRCLE, 0);
  
  lv_obj_t * icon_label = lv_label_create(icon_container);
  lv_label_set_text(icon_label, "RFID");
  lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_20, 0);
  lv_obj_center(icon_label);
  
  // Texto informativo
  lv_obj_t * info_label = lv_label_create(screens[0]);
  lv_label_set_text(info_label, "El sistema registrará automáticamente\nsu código y lo llevará al formulario");
  lv_obj_set_width(info_label, 220);
  lv_obj_set_style_text_align(info_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(info_label, LV_ALIGN_BOTTOM_MID, 0, -40);
}

// Crear pantalla de registro (Pantalla 1)
void create_registration_screen() {
  // Crear la navbar para esta pantalla
  create_navbar(1);

  // Título - ajustado para estar debajo de la navbar
  lv_obj_t * title = lv_label_create(screens[1]);
  lv_label_set_text(title, "Registro de Usuario");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, NAVBAR_HEIGHT + 10);
  
  // Etiqueta para mostrar el código recibido
  code_label = lv_label_create(screens[1]);
  lv_label_set_text(code_label, "Código recibido: ");
  lv_obj_align(code_label, LV_ALIGN_TOP_LEFT, 20, NAVBAR_HEIGHT + 40);
  
  // Crear campos del formulario
  // 1. Campo Nombre
  lv_obj_t * name_label = lv_label_create(screens[1]);
  lv_label_set_text(name_label, "Nombre:");
  lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, 20, NAVBAR_HEIGHT + 70);
  
  name_textarea = lv_textarea_create(screens[1]);
  lv_obj_set_size(name_textarea, 200, 40);
  lv_obj_align(name_textarea, LV_ALIGN_TOP_LEFT, 20, NAVBAR_HEIGHT + 90);
  lv_textarea_set_placeholder_text(name_textarea, "Ingrese nombre completo");
  lv_obj_add_event_cb(name_textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
  
  // 2. Campo ID/Cédula
  lv_obj_t * id_label = lv_label_create(screens[1]);
  lv_label_set_text(id_label, "ID/Cédula:");
  lv_obj_align(id_label, LV_ALIGN_TOP_LEFT, 20, NAVBAR_HEIGHT + 140);
  
  id_textarea = lv_textarea_create(screens[1]);
  lv_obj_set_size(id_textarea, 200, 40);
  lv_obj_align(id_textarea, LV_ALIGN_TOP_LEFT, 20, NAVBAR_HEIGHT + 160);
  lv_textarea_set_placeholder_text(id_textarea, "Ingrese ID o cédula");
  lv_obj_add_event_cb(id_textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
  
  // 3. Campo Teléfono
  lv_obj_t * phone_label = lv_label_create(screens[1]);
  lv_label_set_text(phone_label, "Teléfono:");
  lv_obj_align(phone_label, LV_ALIGN_TOP_LEFT, 20, NAVBAR_HEIGHT + 210);
  
  phone_textarea = lv_textarea_create(screens[1]);
  lv_obj_set_size(phone_textarea, 200, 40);
  lv_obj_align(phone_textarea, LV_ALIGN_TOP_LEFT, 20, NAVBAR_HEIGHT + 230);
  lv_textarea_set_placeholder_text(phone_textarea, "Ingrese número telefónico");
  lv_obj_add_event_cb(phone_textarea, textarea_event_cb, LV_EVENT_ALL, NULL);
  
  // Botón de registro
  lv_obj_t * register_btn = lv_button_create(screens[1]);
  lv_obj_set_size(register_btn, 160, 50);
  lv_obj_align(register_btn, LV_ALIGN_BOTTOM_MID, 0, -60);
  lv_obj_set_style_bg_color(register_btn, lv_color_hex(0x16A085), 0); // Verde
  lv_obj_add_event_cb(register_btn, register_btn_event, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t * btn_label = lv_label_create(register_btn);
  lv_label_set_text(btn_label, "REGISTRAR");
  lv_obj_center(btn_label);
  
  // Etiqueta de estado del registro
  status_label = lv_label_create(screens[1]);
  lv_label_set_text(status_label, "Complete los datos y presione Registrar");
  lv_obj_set_width(status_label, 200);
  lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -20);
  
  // Crear teclado virtual (oculto inicialmente)
  keyboard = lv_keyboard_create(screens[1]);
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN); // Inicialmente oculto
  lv_obj_set_size(keyboard, SCREEN_WIDTH, SCREEN_HEIGHT / 2);
  lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, NULL);
  
  focused_input = NULL;
}

// Crear pantalla de estadísticas (Pantalla 2)
void create_stats_screen() {
  // Crear la navbar para esta pantalla
  create_navbar(2); 
  
  // Título
  lv_obj_t * title = lv_label_create(screens[2]);
  lv_label_set_text(title, "Estado del Sistema");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, NAVBAR_HEIGHT + 10);
  
  // Estadísticas de conexión
  lv_obj_t * stats_label = lv_label_create(screens[2]);
  
  // Crear texto con información del sistema
  char stats_text[256];
  snprintf(stats_text, sizeof(stats_text), 
           "WiFi: %s\n"
           "IP: %s\n"
           "MQTT: %s\n"
           "Tópico receptor: %s\n"
           "Tópico de envío: %s\n"
           "RAM libre: %d KB\n"
           "Tiempo activo: %d seg",
           WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado",
           WiFi.localIP().toString().c_str(),
           client.connected() ? "Conectado" : "Desconectado",
           mqtt_topic_receive,
           mqtt_topic_register,
           ESP.getFreeHeap() / 1024,
           millis() / 1000);
  
  lv_label_set_text(stats_label, stats_text);
  lv_obj_align(stats_label, LV_ALIGN_TOP_MID, 0, NAVBAR_HEIGHT + 60);
  
  // Botón para actualizar estadísticas
  lv_obj_t * btn_refresh = lv_button_create(screens[2]);
  lv_obj_align(btn_refresh, LV_ALIGN_BOTTOM_MID, 0, -40);
  lv_obj_set_size(btn_refresh, 120, 40);
  lv_obj_add_event_cb(btn_refresh, [](lv_event_t * e) {
    // Actualizar estadísticas
lv_obj_t * stats_label = (lv_obj_t *)lv_event_get_user_data(e);    
char stats_text[256];
    snprintf(stats_text, sizeof(stats_text), 
             "WiFi: %s\n"
             "IP: %s\n"
             "MQTT: %s\n"
             "Tópico receptor: %s\n"
             "Tópico de envío: %s\n"
             "RAM libre: %d KB\n"
             "Tiempo activo: %d seg",
             WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado",
             WiFi.localIP().toString().c_str(),
             client.connected() ? "Conectado" : "Desconectado",
             mqtt_topic_receive,
             mqtt_topic_register,
             ESP.getFreeHeap() / 1024,
             millis() / 1000);
    lv_label_set_text(stats_label, stats_text);
  }, LV_EVENT_CLICKED, stats_label);
  
  lv_obj_t * btn_label = lv_label_create(btn_refresh);
  lv_label_set_text(btn_label, "Actualizar");
  lv_obj_center(btn_label);
}

// Función para crear todas las pantallas
void create_all_screens() {
  // Crear las pantallas
  for (int i = 0; i < 3; i++) {
    screens[i] = lv_obj_create(NULL);
  }
  
  // Crear contenido para cada pantalla
  create_home_screen();          // Pantalla 0 - Inicio
  create_registration_screen();  // Pantalla 1 - Registro
  create_stats_screen();         // Pantalla 2 - Estado
  
  // Cargar la primera pantalla
  lv_screen_load(screens[0]);
}

void setup() {
  Serial.begin(115200);
  
  // Inicializar LVGL
  lv_init();
  lv_log_register_print_cb(log_print);
  
  // Inicializar touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);
  
  // Crear display
  lv_display_t * disp;
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);
  
  // Configurar touchscreen
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);
  
  // Crear todas las pantallas y sus elementos
  create_all_screens();
  
  // Configurar WiFi y MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  Serial.println("Configuración completada. Iniciando aplicación.");
}

void loop() {
  // Manejar conexión MQTT
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
  
  // Manejar GUI
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);
}