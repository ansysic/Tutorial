#include <pebble.h>

Window *window;
TextLayer *text_layer;
char buffer[] = "00:00";
InverterLayer *inv_layer;
TextLayer *bt_layer, *batt_layer, *emoji_layer;

static void batt_handler(BatteryChargeState charge_state) {
    static char buffer[] = "100%";
    snprintf(buffer, sizeof("100%"), "%d%%", charge_state.charge_percent);
    text_layer_set_text(batt_layer, buffer);
}

static void bt_handler(bool connected)
{
  if(connected == true)
  {
    text_layer_set_text(bt_layer, "BT");
  }
  else
  {
    text_layer_set_text(bt_layer, "");
  }
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  //Here we will update the watchface display
  //Format the buffer string using tick_time as the time source
  strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
 
  //Change the TextLayer text to show the new time!
  text_layer_set_text(text_layer, buffer);
}

void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
   //Create an array of ON-OFF-ON etc durations in milliseconds
   uint32_t segments[] = {50};
   //Create a VibePattern structure with the segments and length of the pattern as fields
   VibePattern pattern = {
     .durations = segments,
     .num_segments = ARRAY_LENGTH(segments),
   };
   //Trigger the custom pattern to be executed
   vibes_enqueue_custom_pattern(pattern);
}

void click_config_provider(void *context)
{
   window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

void window_load(Window *window) {
  //Layer Uhrzeit
    text_layer = text_layer_create(GRect(0, 53, 144, 168));
    text_layer_set_background_color(text_layer, GColorClear);
    text_layer_set_text_color(text_layer, GColorBlack);
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
    
  //Layer Invertierung
    inv_layer = inverter_layer_create(GRect(0, 50, 144, 62));
    layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
    
  //Uhrzeit
    struct tm *t;
    time_t temp;
    temp = time(NULL);
    t = localtime(&temp);
    //Manually call the tick handler when the window is loading
    tick_handler(t, MINUTE_UNIT);
    
  //Bluetooth
    bt_layer = text_layer_create(GRect(5,0,30,20));
    text_layer_set_font(bt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    //text_layer_set_text_color(bt_layer, GColorWhite);
    //text_layer_set_background_color(bt_layer, GColorBlack);
    bool is_connected = bluetooth_connection_service_peek();
    bt_handler(is_connected);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(bt_layer));
  
  //Batterie
    batt_layer = text_layer_create(GRect(100,0,39,20));
    text_layer_set_font(batt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    //text_layer_set_text_color(batt_layer, GColorWhite);
    //text_layer_set_background_color(batt_layer, GColorBlack);
    text_layer_set_text_alignment(batt_layer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(batt_layer));
    //Get info, copy to long-lived buffer and display
    BatteryChargeState state = battery_state_service_peek();
    batt_handler(state);
  
  //Emoji-Layer
    emoji_layer = text_layer_create(GRect(5,145,134,18));
    text_layer_set_font(emoji_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(emoji_layer, GTextAlignmentCenter);
    //text_layer_set_background_color(emoji_layer, GColorBlack);
    //text_layer_set_text_color(emoji_layer, GColorWhite);
    text_layer_set_text(emoji_layer, "P💛L");
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(emoji_layer));
}

void window_unload(Window *window) {
  //We will safely destroy the Window's elements here!
  text_layer_destroy(text_layer);
  inverter_layer_destroy(inv_layer);
  text_layer_destroy(bt_layer);
  text_layer_destroy(batt_layer);
  text_layer_destroy(emoji_layer);
}

void init() {
  //Initialize the app elements here!
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_click_config_provider(window, click_config_provider);
  tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
  bluetooth_connection_service_subscribe(bt_handler);
  battery_state_service_subscribe(batt_handler);
  window_stack_push(window, true);
} 

void deinit() {
  //De-initialize elements here to save memory!
  window_destroy(window);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
}
  
int main(void) {
  init();
  app_event_loop();
  deinit();
}

