#include "screen_teclado.h"
#include "screen_manager.h"
#include "config.h"
#include <cstring>

lv_obj_t* keyboard;

static void teclado_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* obj = (lv_obj_t*) lv_event_get_target(e);  // cast explícito aquí

    if (code == LV_EVENT_VALUE_CHANGED) {
        const char* txt = lv_keyboard_get_button_text(obj, lv_keyboard_get_selected_btn(obj));
        if (txt != NULL && strcmp(txt, LV_SYMBOL_OK) == 0) {
            // Ocultar teclado
            lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void screen_teclado_create(lv_obj_t* parent) {
    keyboard = lv_keyboard_create(parent);
    
    // Set keyboard size based on screen dimensions
    lv_obj_set_size(keyboard, 320, 220 / 2);
    
    // Align to bottom of screen with no offset
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    // Set keyboard mode
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    
    // Hide initially
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    
    // Add event callback for keyboard
    lv_obj_add_event_cb(keyboard, teclado_event_cb, LV_EVENT_ALL, NULL);
}
