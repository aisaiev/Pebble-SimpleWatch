#include <pebble.h>
  
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
  
Window *MainWindow;
TextLayer *TimeText;
TextLayer *WeekText;
TextLayer *DateText;
TextLayer *WeatherText;
GFont TimeFont;
GFont WeekFont;
GFont DateFont;
GFont WeatherFont;
 
void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char time[10];
  static char week[12];
  static char date[12];
    
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 24h hour format
    strftime(time, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(time, sizeof("00:00"), "%I:%M", tick_time);
}
  
  // Write the current week into the buffer
strftime(week, sizeof(week), "%A", tick_time);
  
  // Write the current week into the buffer
strftime(date, sizeof(date), "%d.%m.%Y", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(TimeText, time);
    
  // Display this week on the TextLayer
  text_layer_set_text(WeekText, week);
  
  // Display this date on the TextLayer
  text_layer_set_text(DateText, date);
   
}
  
void main_window_load(Window *window) {
  window_set_background_color(MainWindow, GColorBlack);
  // Time Text
  TimeText = text_layer_create(GRect(0, 46, 145, 50));
  text_layer_set_background_color(TimeText, GColorClear);
  text_layer_set_text_color(TimeText, GColorClear);
  TimeFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_TIME_36));
  text_layer_set_font(TimeText, TimeFont);
  text_layer_set_text_alignment(TimeText, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(TimeText));

  // Week Text
  WeekText = text_layer_create(GRect(0, 34, 145, 30));
  text_layer_set_background_color(WeekText, GColorClear);
  text_layer_set_text_color(WeekText, GColorClear);
  WeekFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_TIME_17));
  text_layer_set_font(WeekText, WeekFont);
  text_layer_set_text_alignment(WeekText, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(WeekText));
  
  // Date Text
  DateText = text_layer_create(GRect(0, 88, 145, 25));
  text_layer_set_background_color(DateText, GColorClear);
  text_layer_set_text_color(DateText, GColorClear);
  DateFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_TIME_17));
  text_layer_set_font(DateText, DateFont);
  text_layer_set_text_alignment(DateText, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(DateText));
  
  // Weather Text
  WeatherText = text_layer_create(GRect(0, 147, 145, 25));
  text_layer_set_background_color(WeatherText, GColorClear);
  text_layer_set_text_color(WeatherText, GColorClear);
  WeatherFont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PERFECT_TIME_14));
  text_layer_set_font(WeatherText, WeatherFont);
  text_layer_set_text_alignment(WeatherText, GTextAlignmentCenter);
  text_layer_set_text(WeatherText, "Loading");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(WeatherText));
      
  // Make sure the time is displayed from the start
  update_time();
}

void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(TimeText);
  text_layer_destroy(WeekText);
  text_layer_destroy(DateText);
  text_layer_destroy(WeatherText);
  fonts_unload_custom_font(TimeFont);
  fonts_unload_custom_font(WeekFont);
  fonts_unload_custom_font(DateFont);
  fonts_unload_custom_font(WeatherFont);
}
  
void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 1 hour
  if(tick_time->tm_hour % 1 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
// Store incoming information
  static char temperature[8];
  static char conditions[32];
  static char weather[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature, sizeof(temperature), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions, sizeof(conditions), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
  }

    // Look for next item
    t = dict_read_next(iterator);
}
  
  // Assemble full string and display
  snprintf(weather, sizeof(weather), "%s, %s", temperature, conditions);
  text_layer_set_text(WeatherText, weather);  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void init() {
  // Create main Window element and assign to pointer
  MainWindow = window_create();
  window_set_background_color(MainWindow, GColorWhite);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(MainWindow, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(MainWindow, false);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void deinit() {
  // Destroy Window
  window_destroy(MainWindow);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}