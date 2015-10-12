#include <pebble.h>
	
static Window *main_win;
static GFont *small_font;
static BitmapLayer *box_blue;
static BitmapLayer *box_batt;
static BitmapLayer *box_apm;
static TextLayer *box_date;
static TextLayer *hour_text;

static Layer *secs_layer;
static Layer *secs_line;

static Layer *icon_bg;
static Layer *icon_line;
static BitmapLayer *min_dig_ten;
static BitmapLayer *min_dig_one;

static BitmapLayer *ap_logo;

static GBitmap *res_am;
static GBitmap *res_pm;

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

#define BOX_POS(x,y) GRect(27 + (x)*18, 110 + (y)*18, 16, 16)

int ICO_IDS[] = {
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
	RESOURCE_ID_TS_ICO_12,
	RESOURCE_ID_TS_ICO_13,
	RESOURCE_ID_TS_ICO_14,
	RESOURCE_ID_TS_ICO_15,
	RESOURCE_ID_TS_ICO_16,
	RESOURCE_ID_TS_ICO_17,
	RESOURCE_ID_TS_ICO_18,
	RESOURCE_ID_TS_ICO_19,
	RESOURCE_ID_TS_ICO_20,
	RESOURCE_ID_TS_ICO_21,
	RESOURCE_ID_TS_ICO_22,
	RESOURCE_ID_TS_ICO_23,
	RESOURCE_ID_TS_ICO_24,
	RESOURCE_ID_TS_ICO_25,
	RESOURCE_ID_TS_ICO_26
};
const int NUM_ICONS = 26;

GBitmap *ico_bitmap[6];

BitmapLayer *ico_layers[6];

static void initialise_ui(void) {
	main_win = window_create();
	
	#ifdef PBL_SDK_2
	window_set_fullscreen(main_win, true);
	#endif
	
	struct Layer *root_layer = window_get_root_layer(main_win);
	
	// bluetooth bitmap
	box_blue = bitmap_layer_create(BOX_POS(0,0));
	bitmap_layer_set_bitmap(box_blue, res_bluetooth_off);
	layer_add_child(root_layer, (Layer *)box_blue);

	// battery bitmap
	box_batt = bitmap_layer_create(BOX_POS(4,0));
	bitmap_layer_set_bitmap(box_batt, res_batt_90);
	layer_add_child(root_layer, (Layer *)box_batt);
	
	// aperture logo
	ap_logo = bitmap_layer_create(GRect(10, 150, 65, 16));
	bitmap_layer_set_bitmap(ap_logo, res_ap_logo);
	layer_add_child(root_layer, (Layer *)ap_logo);

	// weekday textbox
	box_date = text_layer_create(GRect(27 + 1*18, 110 - 1, 16, 16));
	text_layer_set_background_color(box_date, GColorWhite);
	text_layer_set_text_color(box_date, GColorBlack);
	text_layer_set_text_alignment(box_date, GTextAlignmentCenter);
	text_layer_set_font(box_date, small_font);
	layer_add_child(root_layer, (Layer *)box_date);
	
	// am/pm bitmap
	box_apm = bitmap_layer_create(BOX_POS(3,0));
	bitmap_layer_set_bitmap(box_apm, res_am);
	layer_add_child(root_layer, (Layer *)box_apm);
	
	// seconds line
	secs_line = layer_create(GRect(10, 85, 123, 1));
	layer_add_child(root_layer, (Layer *)secs_line);

	// seconds layer
	secs_layer = layer_create(GRect(10, 89, 123, 8));
	layer_add_child(root_layer, (Layer *)secs_layer);
		
	// minute tens digit
	min_dig_ten = bitmap_layer_create(GRect(71 - 32 - 2, 5, 32, 80));
	layer_add_child(root_layer, (Layer *)min_dig_ten);

	// minte ones digit
	min_dig_one = bitmap_layer_create(GRect(71 + 2, 5, 32, 80));
	layer_add_child(root_layer, (Layer *)min_dig_one);
	
	// hour text
	hour_text = text_layer_create(GRect(8, 68, 32, 15));
	text_layer_set_background_color(hour_text, GColorWhite);
	text_layer_set_text_color(hour_text, GColorBlack);
	text_layer_set_text(hour_text, "??");
	text_layer_set_text_alignment(hour_text, GTextAlignmentRight);
	text_layer_set_font(hour_text, small_font);
	layer_add_child(root_layer, (Layer *)hour_text);	
	
	// Icon line
	icon_line = layer_create(GRect(10, 105, 123, 1));
	layer_add_child(root_layer, (Layer *)icon_line);
	
	//Icon background
	icon_bg = layer_create(GRect(10, 109, 123, 40));
	layer_add_child(root_layer, (Layer *)icon_bg);
	
	// Init all the bitmap icons
	for (int i=0; i<6; i++) {
		if (i==0) {
			ico_layers[i] = bitmap_layer_create(BOX_POS(2, 0));
		} else {
			ico_layers[i] = bitmap_layer_create(BOX_POS(i-1, 1));
		}
		bitmap_layer_set_bitmap(ico_layers[i], ico_bitmap[i]);
		layer_add_child(root_layer, (Layer *)ico_layers[i]);
	}
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
	
	gbitmap_destroy(res_am);
	gbitmap_destroy(res_pm);

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
	
	for (int i=0; i<6; i++) {
		bitmap_layer_destroy(ico_layers[i]);
		gbitmap_destroy(ico_bitmap[i]);
	}
}

enum {
	POW_LINES,
	POW_LOGO, 
	POW_NUMS,
	POW_BOXES,
	POW_PROGRESS
};

// Macros to simplify these commands.
#define HIDE(lay) layer_set_hidden((Layer *) lay, true)
#define SHOW(lay) layer_set_hidden((Layer *) lay, false)
void powerdown() {
	// Hide all the icons.
	HIDE(icon_bg);
	HIDE(icon_line);
	
	HIDE(box_blue);
	HIDE(box_batt);
	HIDE(box_apm);
	HIDE(box_date);
	
	HIDE(secs_layer);
	HIDE(secs_line);
	HIDE(hour_text);
	
	HIDE(min_dig_ten);
	HIDE(min_dig_one);

	HIDE(ap_logo);
	for (int i=0; i<6; i++) {
		HIDE(ico_layers[i]);
	}
}

void powerup_lines(void *val) {
	SHOW(icon_line);
	SHOW(secs_line);
}	
void powerup_logo(void *val) {
	SHOW(ap_logo);
	
	// Hide everything from powerup_nums to allow switching between them
	HIDE(min_dig_ten);
	HIDE(min_dig_one);
	HIDE(hour_text);
	HIDE(icon_bg);
}		
void powerup_nums(void *val) {
	SHOW(min_dig_ten);
	SHOW(min_dig_one);
	SHOW(hour_text);
	SHOW(icon_bg);
}		
void powerup_boxes(void *val) {
	SHOW(box_blue);
	SHOW(box_batt);
	SHOW(box_apm);
	SHOW(box_date);
	for (int i=0; i<6; i++) {
		SHOW(ico_layers[i]);
	}
}

static bool playing_powerup = false;

void powerup_progress(void *val) {
	SHOW(secs_layer);
	
	light_enable_interaction(); // Return light to normal
	light_enable(false);
	playing_powerup = false;
}
	
static void powerup() {
	// Play the light flickering animation.
	if (playing_powerup){
		return; // Don't repeat
	}
	playing_powerup = true;
	
	powerdown();  // Hide everything first
	light_enable(true); // Keep the light on throughout the animation
	app_timer_register( 150, &powerup_lines, NULL);
	app_timer_register( 300, &powerup_nums, NULL);
	app_timer_register( 400, &powerup_logo, NULL);
	app_timer_register( 600, &powerup_nums, NULL);
	app_timer_register( 800, &powerup_logo, NULL);
	app_timer_register( 900, &powerup_nums, NULL);
	app_timer_register(1100, &powerup_boxes, NULL);
	app_timer_register(1500, &powerup_boxes, NULL);
	app_timer_register(1800, &powerup_progress, NULL);
}

static void draw_sep_line(struct Layer *layer, GContext *ctx) {
	// Draw the line that sepaarates seconds from the boxes or the testchamber number.
	graphics_draw_line(ctx, GPoint(0,0), GPoint(122, 0));
}

static void draw_seconds(struct Layer *layer, GContext *ctx) {
	// Draw the seconds bar-graph.
	
	time_t temp = time(NULL); 
	struct tm *cur_time = localtime(&temp);
	graphics_context_set_stroke_color(ctx, GColorBlack);
#ifdef PBL_COLOR
	// On color Pebbles, draw 'off' bars in grey
	for(int i = 2; i <= 60 * 2; i += 2) {
		if ((cur_time -> tm_sec *2) + 2 == i) {
			graphics_context_set_stroke_color(ctx, GColorLightGray);
		}
#else
	for (int i = 2; i <= (cur_time->tm_sec * 2); i += 2) {
#endif
		graphics_draw_line(ctx, GPoint(i, 0), GPoint(i, 8));
	}
}

static void draw_icon_bg(struct Layer *layer, GContext *ctx) {
	// Draw all the borders around the icons.
	graphics_context_set_stroke_color(ctx, GColorBlack);
	for (int x = 0; x <= 4; x += 1) {
		for (int y = 0; y <= 1; y += 1) {
			graphics_draw_rect(ctx, GRect(x*18 + 16, y*18, 18, 18));
		}
	}
}

void show_main_window() {
	initialise_ui();
	window_set_window_handlers(main_win, (WindowHandlers) {
		.unload = handle_window_unload,
	});
	layer_set_update_proc(secs_layer, &draw_seconds);
	layer_set_update_proc(secs_line, &draw_sep_line);
	
	layer_set_update_proc(icon_bg, &draw_icon_bg);
	layer_set_update_proc(icon_line, &draw_sep_line);
	
	window_stack_push(main_win, true);
}

void hide_main_window(void) {
  window_stack_remove(main_win, true);
}

void bluetooth_check(bool connected) {
	if (connected) {
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

static void shuffle_icons() {
	// Rearrange the icon array and apply it to the display.
	int i, j, tmp;
	for (i=(NUM_ICONS-1); i>0; i--){
		j = rand() % (i+1);
		tmp = ICO_IDS[i];
		ICO_IDS[i] = ICO_IDS[j];
		ICO_IDS[j] = tmp;
	}
	
	for (i=0; i<6; i++) {
		gbitmap_destroy(ico_bitmap[i]);
		ico_bitmap[i] =  gbitmap_create_with_resource(ICO_IDS[i]);
		bitmap_layer_set_bitmap(ico_layers[i], ico_bitmap[i]);
	}
	powerup(); // "Restart" the screen
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

static void shake_handler(AccelAxisType axis, int32_t dir) {
	// On shakes, shuffle the icons.
	shuffle_icons();
}

static void battery_update(BatteryChargeState state) {
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
	if ((units_changed & SECOND_UNIT) != 0) {
		layer_mark_dirty(secs_layer);
		BatteryChargeState st = battery_state_service_peek();
		if (st.is_plugged || st.is_charging) {
			battery_update(st);
		}
	}
	
	if ((units_changed & MINUTE_UNIT) != 0) {
		char min_char[] = "0000";
		strftime(min_char, sizeof("--"), "%M", tick_time);
		display_num(min_char[0], min_dig_ten);
		display_num(min_char[1], min_dig_one);
	}
	
	if ((units_changed & DAY_UNIT) != 0) {
		static char date_char[] = "22";
		strftime(date_char, sizeof("--"), "%d", tick_time);
		text_layer_set_text(box_date, date_char);
	}
	
	if ((units_changed & HOUR_UNIT) !=0) {
		shuffle_icons();
		
		static char hour_char[] = "00/19";
		if (clock_is_24h_style()) {
			strftime(hour_char, sizeof("-----"), "%H/34", tick_time);
		} else {
			strftime(hour_char, sizeof("-----"), "%I/19", tick_time);
		}
		text_layer_set_text(hour_text, hour_char);
		
		if ((tick_time -> tm_hour) < 12) {
			bitmap_layer_set_bitmap(box_apm, res_am);
		} else {
			bitmap_layer_set_bitmap(box_apm, res_pm);
		}
		
	}
}

static void init() {
	res_bluetooth_on = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUE_ON);
	res_bluetooth_off = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUE_OFF);
	
	small_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
	
	res_ap_logo = gbitmap_create_with_resource(RESOURCE_ID_IMG_AP_LOGO);
	
	res_am = gbitmap_create_with_resource(RESOURCE_ID_TS_ICO_AM);
	res_pm = gbitmap_create_with_resource(RESOURCE_ID_TS_ICO_PM);
	
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
	
	for (int i=0; i<6; i++) {
		ico_bitmap[i] = gbitmap_create_with_resource(ICO_IDS[i]);
	}
}

int main() {
	srand(time(NULL));
	
	init();
	show_main_window();
	
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *cur_time = localtime(&temp);
	
	// Run these the first time
	time_handler(cur_time, SECOND_UNIT | HOUR_UNIT | MINUTE_UNIT | DAY_UNIT);
	bluetooth_check(bluetooth_connection_service_peek());
	battery_update(battery_state_service_peek());
	
	tick_timer_service_subscribe(SECOND_UNIT | HOUR_UNIT | MINUTE_UNIT | DAY_UNIT, time_handler);
	
	battery_state_service_subscribe(battery_update);
	bluetooth_connection_service_subscribe(bluetooth_check);
	accel_tap_service_subscribe(shake_handler);
	
	shuffle_icons(); // also starts the powerup animation
	
	app_event_loop();
	
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	accel_tap_service_unsubscribe();
	
	return 0;
}