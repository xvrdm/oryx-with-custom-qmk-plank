#pragma once
// Custom Planck EZ behaviors. Included from QJynB/keymap.c.
// APP_NEXT and APP_PREV are declared in keymap.c's custom-keycode enum.

static bool app_switch_active = false;
static bool app_switch_owns_gui = false;
static bool app_e_pressed = false;
static bool app_r_pressed = false;
static bool shifted_backspace_pressed = false;

static void app_switch_start(void) {
    if (!app_switch_active) {
        app_switch_owns_gui = !(get_mods() & MOD_BIT(KC_LGUI));
        if (app_switch_owns_gui) {
            register_code(KC_LGUI);
        }
        app_switch_active = true;
    }
}

static void app_switch_stop(void) {
    if (app_switch_active) {
        if (app_switch_owns_gui) {
            unregister_code(KC_LGUI);
        }
        app_switch_owns_gui = false;
        app_switch_active = false;
    }
}

// Return false to consume the event, true for normal Oryx handling.
static bool custom_process_record(uint16_t keycode, keyrecord_t *record) {


  // Shift + tap Backspace -> Forward Delete
  // Hold Backspace -> Cmd (unchanged)
  if (keycode == MT(MOD_LGUI, KC_BSPC)) {
      if (!record->event.pressed &&
          shifted_backspace_pressed) {
          shifted_backspace_pressed = false;
          return false;
      }

      if (record->event.pressed &&
          record->tap.count &&
          (get_mods() & MOD_MASK_SHIFT)) {

          // Temporarily remove Shift so macOS receives
          // an unmodified Forward Delete.
          uint8_t saved_mods = get_mods();
          del_mods(MOD_MASK_SHIFT);
          send_keyboard_report();

          tap_code(KC_DEL);

          // Restore the held modifiers.
          set_mods(saved_mods);
          send_keyboard_report();

          shifted_backspace_pressed = true;
          return false;
      }
  }


  // Layer 6: automatic Cmd + Tab / Shift+Tab
  if (keycode == APP_NEXT || keycode == APP_PREV) {
      if (record->event.pressed) {
          app_switch_start();

          if (keycode == APP_NEXT) {
              tap_code(KC_TAB);
          } else {
              tap_code16(S(KC_TAB));
          }
      }
      return false;
  }

  // Base layer: E/R while physically holding Cmd
  if (keycode == KC_E || keycode == KC_R) {
      bool *consumed = (keycode == KC_E)
                       ? &app_e_pressed
                       : &app_r_pressed;

      if (!record->event.pressed) {
          if (*consumed) {
              *consumed = false;
              return false;
          }
      } else if (get_mods() & MOD_MASK_GUI) {
          if (keycode == KC_E) {
              tap_code16(S(KC_TAB));
          } else {
              tap_code(KC_TAB);
          }
          *consumed = true;
          return false;
      }
  }

  // Cancel the layer-6 switcher before any other key.
  if (record->event.pressed) {
      app_switch_stop();
  }

    return true;
}

static void custom_layer_state_set(layer_state_t state) {
    if (!(state & (1UL << _LAYER6))) {
        app_switch_stop();
    }
}
