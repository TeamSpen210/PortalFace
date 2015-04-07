#include <pebble.h>
	
static Window *main_win;
static GFont *date_font;
static GFont *hour_font;
static BitmapLayer *box_blue;
static BitmapLayer *box_batt;
static BitmapLayer *ap_logo;
static TextLayer *box_date;
static TextLayer *hour_text;

static Layer *secs_layer;
static Layer *icon_bg;
static BitmapLayer *min_dig_ten;
static BitmapLayer *min_dig_one;

static GBitmap *res_digit_0;
static GBitmap *res_digit_1;
static GBitmap *res_digit_2;
static GBitmap *res_digit_3;
static GBitmap *res_digit_4;
static GBitmap *res_digit_5;
static GBitmap *res_digit_6;
static GBitmap *res_digit_7;
static GBitmap *res_digit_8;
static GBitmap *res_digit_9;

static GBitmap *res_batt_10;
static GBitmap *res_batt_20;
static GBitmap *res_batt_30;
static GBitmap *res_batt_40;
static GBitmap *res_batt_50;
static GBitmap *res_batt_60;
static GBitmap *res_batt_70;
static GBitmap *res_batt_80;
static GBitmap *res_batt_90;
static GBitmap *res_batt_100;

static GBitmap *res_bluetooth_on;
static GBitmap *res_bluetooth_off; 

static GBitmap *res_ap_logo;

static bool charge_vibe_done = 1;
static bool bluetooth_vibe_done = 1;

byte ICO_IDS[] = {
	RESOURCE_ID_TS_ICO_1,
	RESOURCE_ID_TS_ICO_2,
	RESOURCE_ID_TS_ICO_3,
	RESOURCE_ID_TS_ICO_4,
	RESOURCE_ID_TS_ICO_5,
	RESOURCE_ID_TS_ICO_6,
	RESOURCE_ID_TS_ICO_7,
	RESOURCE_ID_TS_ICO_8,
	RESOURCE_ID_TS_ICO_9,
	RESOURCE_ID_TS_ICO_10,
	RESOURCE_ID_TS_ICO_11,
	RESOURCE_ID_TS_ICO_12
}

static void initialise_ui(void) {
	main_win = window_create();
	window_set_fullscreen(main_win, true);

	// bluetooth bitmap
	box_blue = bitmap_layer_create(GRect(14, 110, 32, 32));
	bitmap_layer_set_bitmap(box_blue, res_bluetooth_off);
	layer_add_child(window_get_root_layer(main_win), (Layer *)box_blue);

	// battery bitmap
	box_batt = bitmap_layer_create(GRect(96, 110, 32, 32));
	bitmap_layer_set_bitmap(box_batt, res_batt_90);
	layer_add_child(window_get_root_layer(main_win), (Layer *)box_batt);
	
	// aperture logo
	ap_logo = bitmap_layer_create(GRect(10, 150, 65, 16));
	bitmap_layer_set_bitmap(ap_logo, res_ap_logo);
	layer_add_child(window_get_root_layer(main_win), (Layer *)ap_logo);

	// weekday textbox
	box_date = text_layer_create(GRect(54, 107, 36, 36));
	text_layer_set_background_color(box_date, GColorWhite);
	text_layer_set_text_color(box_date, GColorBlack);
	text_layer_set_text_alignment(box_date, GTextAlignmentCenter);
	text_layer_set_font(box_date, date_font);
	layer_add_child(window_get_root_layer(main_win), (Layer *)box_date);

	// seconds layer
	secs_layer = layer_create(GRect(10, 85, 123, 20));
	layer_add_child(window_get_root_layer(main_win), (Layer *)secs_layer);
	
	icon_bg = layer_create(GRect(160, 110, 123, 32));
	layer_add_child(window_get_root_layer(main_win), (Layer *)icon_bg);
		
	// minute tens digit
	min_dig_ten = bitmap_layer_create(GRect(71 - 32 - 2, 5, 32, 80));
	layer_add_child(window_get_root_layer(main_win), (Layer *)min_dig_ten);

	// minte ones digit
	min_dig_one = bitmap_layer_create(GRect(71 + 2, 5, 32, 80));
	layer_add_child(window_get_root_layer(main_win), (Layer *)min_dig_one);
	
	// hour text
	hour_text =  text_layer_create(GRect(8, 68, 32, 15));
	text_layer_set_background_color(hour_text, GColorWhite);
	text_layer_set_text_color(hour_text, GColorBlack);
	text_layer_set_text(hour_text, "??");
	text_layer_set_text_alignment(hour_text, GTextAlignmentRight);
	text_layer_set_font(hour_text, hour_font);
	layer_add_child(window_get_root_layer(main_win), (Layer *)hour_text);
}

static void handle_window_unload(Window* window) {
  window_destroy(main_win);
	
	bitmap_layer_destroy(box_blue);
	bitmap_layer_destroy(box_batt);
	
	text_layer_destroy(box_date);
	text_layer_destroy(hour_text);
	
	layer_destroy(secs_layer);

	bitmap_layer_destroy(min_dig_ten);
	bitmap_layer_destroy(min_dig_one);
	
	bitmap_layer_destroy(ap_logo);

	gbitmap_destroy(res_bluetooth_on);
	gbitmap_destroy(res_bluetooth_off);
	
	gbitmap_destroy(res_ap_logo);

	gbitmap_destroy(res_digit_0);
	gbitmap_destroy(res_digit_1);
	gbitmap_destroy(res_digit_2);
	gbitmap_destroy(res_digit_3);
	gbitmap_destroy(res_digit_4);
	gbitmap_destroy(res_digit_5);
	gbitmap_destroy(res_digit_6);
	gbitmap_destroy(res_digit_7);
	gbitmap_destroy(res_digit_8);
	gbitmap_destroy(res_digit_9);

	gbitmap_destroy(res_batt_10);
	gbitmap_destroy(res_batt_20);
	gbitmap_destroy(res_batt_30);
	gbitmap_destroy(res_batt_40);
	gbitmap_destroy(res_batt_50);
	gbitmap_destroy(res_batt_60);
	gbitmap_destroy(res_batt_70);
	gbitmap_destroy(res_batt_80);
	gbitmap_destroy(res_batt_90);
	gbitmap_destroy(res_batt_100);
}

static void draw_seconds(struct Layer *layer, GContext *ctx) {
	// Draw the seconds bar-graph.
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_line(ctx, GPoint(0,0), GPoint(122, 0));
	
	graphics_draw_line(ctx, GPoint(0, 20), GPoint(122, 20));
	
	
	time_t temp = time(NULL); 
	struct tm *cur_time = localtime(&temp);
	
	for (int i = 2; i <= (cur_time->tm_sec * 2); i += 2) {
		graphics_draw_line(ctx, GPoint(i, 4), GPoint(i, 12));
	}
}

static void draw_icon_bg(struct Layer *layer, GContext *ctx) {
	// Draw all the borders around the icons.
	for (int x = 0; x <= 4; x += 1) {
		for (int y = 0; y <= 1; x += 1) {
			graphics_draw_rect(ctx, GRect(x*18, y*18, 18, 18));
		}
	}
}

void show_main_window(void) {
	initialise_ui();
	window_set_window_handlers(main_win, (WindowHandlers) {
		.unload = handle_window_unload,
	});
	layer_set_update_proc(secs_layer, &draw_seconds);
	layer_set_update_proc(icon_bg, &draw_icon_bg);
	
	window_stack_push(main_win, true);
}

void hide_main_window(void) {
  window_stack_remove(main_win, true);
}

void bluetooth_check() {
	if (bluetooth_connection_service_peek()) {
		bitmap_layer_set_bitmap(box_blue, res_bluetooth_on);
		bluetooth_vibe_done = 0;
	} else {
		bitmap_layer_set_bitmap(box_blue, res_bluetooth_off);
		if (!bluetooth_vibe_done) {
			vibes_long_pulse();
			bluetooth_vibe_done = 1;
		}
	}
}

static void display_num(char num, BitmapLayer *bitmap) {
	// Display the number in the minute bitmaps.
	switch(num) {
		case '0':
		bitmap_layer_set_bitmap(bitmap, res_digit_0);
		break;
		case '1':
		bitmap_layer_set_bitmap(bitmap, res_digit_1);
		break;
		case '2':
		bitmap_layer_set_bitmap(bitmap, res_digit_2);
		break;
		case '3':
		bitmap_layer_set_bitmap(bitmap, res_digit_3);
		break;
		case '4':
		bitmap_layer_set_bitmap(bitmap, res_digit_4);
		break;
		case '5':
		bitmap_layer_set_bitmap(bitmap, res_digit_5);
		break;
		case '6':
		bitmap_layer_set_bitmap(bitmap, res_digit_6);
		break;
		case '7':
		bitmap_layer_set_bitmap(bitmap, res_digit_7);
		break;
		case '8':
		bitmap_layer_set_bitmap(bitmap, res_digit_8);
		break;
		case '9':
		bitmap_layer_set_bitmap(bitmap, res_digit_9);
		break;
	}
}

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *cur_time = localtime(&temp);
	
	char min_char[] = "0000";
	static char hour_char[] = "00/19";
	static char date_char[] = "22";
	
	strftime(min_char, sizeof("--"), "%M", cur_time);
	strftime(date_char, sizeof("--"), "%d", cur_time);
	if (clock_is_24h_style()) {
		strftime(hour_char, sizeof("-----"), "%H/34", cur_time);
	} else {
		strftime(hour_char, sizeof("-----"), "%I/19", cur_time);
	}
	display_num(min_char[0], min_dig_ten);
	display_num(min_char[1], min_dig_one);
	
	text_layer_set_text(box_date, date_char);
	text_layer_set_text(hour_text, hour_char);
	
	bluetooth_check();
}

static void draw_batt(int perc) {
    if (perc <= 10) {
        bitmap_layer_set_bitmap(box_batt, res_batt_10);
    } else if (perc <= 20) {
        bitmap_layer_set_bitmap(box_batt, res_batt_20);
    } else if (perc <= 30) {
        bitmap_layer_set_bitmap(box_batt, res_batt_30);
    } else if (perc <= 40) {
        bitmap_layer_set_bitmap(box_batt, res_batt_40);
    } else if (perc <= 50) {
        bitmap_layer_set_bitmap(box_batt, res_batt_50);
    } else if (perc <= 60) {
        bitmap_layer_set_bitmap(box_batt, res_batt_60);
    } else if (perc <= 70) {
        bitmap_layer_set_bitmap(box_batt, res_batt_70);
    } else if (perc <= 80) {
        bitmap_layer_set_bitmap(box_batt, res_batt_80);
    } else if (perc <= 90) {
        bitmap_layer_set_bitmap(box_batt, res_batt_90);
    } else if (perc <= 110) {
		bitmap_layer_set_bitmap(box_batt, res_batt_100);
	}
}

static void battery_update(BatteryChargeState state) {
	//layer_set_hidden((Layer *)batt_charge, !(state.is_plugged || state.is_charging));
	// If charging, flash to the next percent icon if needed
	if (state.is_plugged || state.is_charging) {
		time_t temp = time(NULL);
		struct tm *cur_time = localtime(&temp);
		if (cur_time -> tm_sec % 2 == 0) {
			draw_batt(state.charge_percent + 10);
		} else {
			draw_batt(state.charge_percent);
		}
	} else {
		draw_batt(state.charge_percent);
	}
	
	if (state.charge_percent == 100) {
		bitmap_layer_set_bitmap(box_batt, res_batt_100);
		// Only trigger vibration if we just switched states.
		if (!charge_vibe_done) {
			vibes_double_pulse();
			charge_vibe_done = 1;
		}
	} else
		{
		charge_vibe_done = 0;
	}
}

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(secs_layer);
	update_time();
	BatteryChargeState st = battery_state_service_peek();
	if (st.is_plugged || st.is_charging) {
		battery_update(st);
	}
}

static void init() {
	res_bluetooth_on = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUE_ON);
	res_bluetooth_off = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUE_OFF);
	
	date_font = fonts_get_system_font(FONT_KEY_GOTHIC_28); //FONT_KEY_BITHAM_30_BLACK);
	hour_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
	
	res_ap_logo = gbitmap_create_with_resource(RESOURCE_ID_IMG_AP_LOGO);
	
	res_batt_10  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_1);
	res_batt_20  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_2);
	res_batt_30  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_3);
	res_batt_40  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_4);
	res_batt_50  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_5);
	res_batt_60  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_6);
	res_batt_70  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_7);
	res_batt_80  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_8);
	res_batt_90  = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_9);
	res_batt_100 = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_10);

	res_digit_0 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_0);
	res_digit_1 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_1);
	res_digit_2 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_2);
	res_digit_3 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_3);
	res_digit_4 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_4);
	res_digit_5 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_5);
	res_digit_6 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_6);
	res_digit_7 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_7);
	res_digit_8 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_8);
	res_digit_9 = gbitmap_create_with_resource(RESOURCE_ID_IMG_NUM_9);
	
}

int main() {
	init();
	show_main_window();

	update_time();
	bluetooth_check();
	battery_update(battery_state_service_peek());
	
	tick_timer_service_subscribe(SECOND_UNIT, time_handler);
	battery_state_service_subscribe(battery_update);
	
	app_event_loop();
	
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	
	return 0;
}