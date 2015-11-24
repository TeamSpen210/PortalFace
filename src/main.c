#include <pebble.h>
	
static Window *main_win;
static BitmapLayer *box_blue;
static BitmapLayer *box_batt;
static BitmapLayer *box_apm;
static TextLayer *box_date;
static TextLayer *hour_text;

// The moving bars
static Layer *secs_layer;
// The line above and below the seconds display
static Layer *secs_line;
#ifndef PBL_ROUND
static Layer *icon_line; // There's only one on round displays
#endif

// The backround for icons
static Layer *icon_bg;

// Minute digits
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

const int MIN_PADDING = 4; // Distance beween minute digits
const int SECONDS_RADIAL_WIDTH = 10; // Length of seconds lines on round display
const int SECONDS_OUTER_PADDING = 4; // Distance from edge
const int SECONDS_RADIAL_COUNT = 120 ; // Number of radial seconds lines
const int SECONDS_INNER_PADDING = 4; // Distance between inner line and seconds bar ring

// Distance the round logo and hour text are inset from the top and bottom
const int ROUND_VERT_INSET = 24;
	
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

GRect box_pos(int off, bool second_row) {
	// Return the rect matching a specific box position.
	// If second_row is true, it's the right/bottom row.
	return GRect(
	#ifdef PBL_ROUND
		(second_row) ? 80+24+16 : 80-24-16, 
		(180 - 5*18)/2 + (off) * 18,
	#else
		27 + (off)*18,
		110 + ((second_row) ? 18: 0),
	#endif
	16, 16);
}

static void initialise_ui(void) {
	#define ADD(child_layer) layer_add_child(root_layer, (Layer *)child_layer)
	main_win = window_create();
	
	#ifdef PBL_SDK_2
	window_set_fullscreen(main_win, true);
	#endif
	
	struct Layer *root_layer = window_get_root_layer(main_win);
	
	#ifdef PBL_ROUND
	GRect bounds = layer_get_bounds(root_layer);
	#endif

	// seconds bar layer. On rect displays it's constrained, but it covers everything on round.
	#ifdef PBL_RECT
		secs_layer = layer_create(GRect(10, 89, 123, 8));
	#else
		secs_layer = layer_create(grect_inset(
			bounds, 
			// Inset slightly
			GEdgeInsets(SECONDS_OUTER_PADDING)
		));
	#endif
	ADD(secs_layer);
	
	// bluetooth bitmap
	box_blue = bitmap_layer_create(box_pos(0, false));
	bitmap_layer_set_bitmap(box_blue, res_bluetooth_off);
	ADD(box_blue);

	// battery bitmap
	box_batt = bitmap_layer_create(box_pos(4, false));
	bitmap_layer_set_bitmap(box_batt, res_batt_90);
	ADD(box_batt);
	
	// aperture logo
	// On round displays it's just the aperture, and bottom-centered
	#ifdef PBL_RECT
		ap_logo = bitmap_layer_create(GRect(10, 150, 65, 16));
	#else
		GRect ap_logo_pos = (GRect){.size = GSize(16, 16)};
		grect_align(&ap_logo_pos, &bounds, GAlignBottom, false);
		ap_logo = bitmap_layer_create(grect_inset(
			ap_logo_pos,
			GEdgeInsets(-ROUND_VERT_INSET, 0, ROUND_VERT_INSET)
		));
	#endif
	bitmap_layer_set_bitmap(ap_logo, res_ap_logo);
	ADD(ap_logo);

	// weekday textbox
	GRect box_date_pos = box_pos(1, false);
	box_date_pos.origin.y -= 1; // It's not aligned perfectly, we need to adjust slightly.
	box_date = text_layer_create(box_date_pos);
	text_layer_set_background_color(box_date, GColorWhite);
	text_layer_set_text_color(box_date, GColorBlack);
	text_layer_set_text_alignment(box_date, GTextAlignmentCenter);
	text_layer_set_font(box_date, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	ADD(box_date);
	
	// am/pm bitmap
	box_apm = bitmap_layer_create(box_pos(3, false));
	bitmap_layer_set_bitmap(box_apm, res_am);
	ADD(box_apm);
	
	// seconds line
	#if defined(PBL_RECT)
		secs_line = layer_create(GRect(10, 85, 123, 1));
	#elif defined(PBL_ROUND)
		// On round watches, it's positioned inside the seconds ring.
		// We need to inset the width of the seconds ring, plus our paddings.
		secs_line = layer_create(grect_inset(bounds, 
			GEdgeInsets(SECONDS_OUTER_PADDING + SECONDS_RADIAL_WIDTH)
		));
	#endif
	ADD(secs_line);
	
	
	#if defined(PBL_RECT) // On rectangular displays, offset upwards
		min_dig_ten = bitmap_layer_create(GRect(71 - 32 - MIN_PADDING/2, 5, 32, 80));
		min_dig_one = bitmap_layer_create(GRect(71 + MIN_PADDING/2, 5, 32, 80));
	#elif defined(PBL_ROUND) // Round displays center the icons
		GRect min_pos = (GRect){.size = GSize(64 + MIN_PADDING, 80)};
		grect_align(&min_pos, &bounds, GAlignCenter, false);
		
		// minute tens digit
		min_dig_ten = bitmap_layer_create(grect_inset(
			min_pos, 
			GEdgeInsets(0, MIN_PADDING/2 + 32, 0, -MIN_PADDING/2)
		));

		// minte ones digit
		min_dig_one = bitmap_layer_create(grect_inset(
			min_pos, 
			GEdgeInsets(0, -MIN_PADDING/2, 0, MIN_PADDING/2 + 32)
		));
	#endif
	
	ADD(min_dig_ten);
	ADD(min_dig_one);
	
	// hour text
	#if defined(PBL_RECT)
		hour_text = text_layer_create(GRect(8, 68, 32, 15));
	#elif defined(PBL_ROUND)
		// This is positioned opposite to the aperture logo
		GRect hour_pos = (GRect){.size = GSize(32, 15)};
		grect_align(&hour_pos, &bounds, GAlignTop, false);
		hour_text = text_layer_create(grect_inset(
			ap_logo_pos,
			GEdgeInsets(ROUND_VERT_INSET, 0, -ROUND_VERT_INSET)
		));
	#endif
	
	text_layer_set_background_color(hour_text, GColorWhite);
	text_layer_set_text_color(hour_text, GColorBlack);
	text_layer_set_text(hour_text, "??");
	text_layer_set_text_alignment(hour_text, PBL_IF_ROUND_ELSE(
		GTextAlignmentCenter,
		GTextAlignmentRight
		));
	text_layer_set_font(hour_text, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	ADD(hour_text);	
	
	// Icon line - we only use 1 on round watches
	#ifndef PBL_ROUND
	icon_line = layer_create(GRect(10, 105, 123, 1));
	ADD(icon_line);
	#endif
	
	//Icon background - on round it's split up!
	#ifdef PBL_ROUND
		GRect first_rect = box_pos(0, false);
		GRect last_rect = box_pos(4, true);
		// The round icon backround needs to be the bounding box for each position, + 1 on each side.
		icon_bg = layer_create(GRect(
				first_rect.origin.x - 1, 
				first_rect.origin.y - 1, 
			    last_rect.origin.x+last_rect.size.w - first_rect.origin.x + 2,
			    last_rect.origin.y+last_rect.size.h - first_rect.origin.y + 2
			));
	#else
		icon_bg = layer_create(GRect(10, 109, 123, 40));
	#endif
		ADD(icon_bg);
	
	// Init all the bitmap icons
	for (int i=0; i<6; i++) {
		if (i==5) {
			// This one is in a different position
			ico_layers[i] = bitmap_layer_create(box_pos(2, false));
		} else {
			ico_layers[i] = bitmap_layer_create(box_pos(i, true));
		}
		bitmap_layer_set_bitmap(ico_layers[i], ico_bitmap[i]);
		ADD(ico_layers[i]);
	}
}

static void handle_window_unload(Window* window) {
	window_destroy(main_win);
	
	bitmap_layer_destroy(box_blue);
	bitmap_layer_destroy(box_batt);
	
	text_layer_destroy(box_date);
	text_layer_destroy(hour_text);
	
	layer_destroy(secs_layer);
	
	#ifndef PBL_ROUND
	layer_destroy(icon_line);
	#endif
	layer_destroy(secs_line);

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

// Macros to simplify these commands.
#define HIDE(lay) layer_set_hidden((Layer *) lay, true)
#define SHOW(lay) layer_set_hidden((Layer *) lay, false)

void powerdown() {
	// Hide all the icons.
	HIDE(icon_bg);
	#ifndef PBL_ROUND
	HIDE(icon_line);
	#endif
	
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
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Lines");
	#ifndef PBL_ROUND
	SHOW(icon_line);
	#endif
	SHOW(secs_line);
}

void powerup_logo(void *val) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Logo");
	SHOW(ap_logo);
	
	// Hide everything from powerup_nums to allow switching between them
	HIDE(min_dig_ten);
	HIDE(min_dig_one);
	HIDE(hour_text);
	HIDE(icon_bg);
}

void powerup_nums(void *val) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Nums");
	SHOW(min_dig_ten);
	SHOW(min_dig_one);
	SHOW(hour_text);
	SHOW(icon_bg);
}

void powerup_boxes(void *val) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Boxes");
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
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Complete");
	SHOW(secs_layer);
	
	light_enable_interaction(); // Return light to normal
	light_enable(false);
	playing_powerup = false;
}

#undef SHOW
#undef HIDE

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
	app_timer_register(1800, &powerup_progress, NULL);
}

static void draw_sep_line(struct Layer *layer, GContext *ctx) {
	#ifdef PBL_ROUND
	// Draw the circle inside the seconds lines.
	// We inset here a bit so the very edges don't get clipped.
	GRect bounds = grect_inset(layer_get_bounds(layer), 
		GEdgeInsets(SECONDS_INNER_PADDING)
	);
	// Find the center and radius (average of two diameters).
	graphics_draw_circle(ctx, 
		grect_center_point(&bounds), 
		(bounds.size.w + bounds.size.h) / 4
	);
	#else
	// Draw the line that sepaarates seconds from the boxes or the testchamber number.
	graphics_draw_line(ctx, GPoint(0,0), GPoint(122, 0));
	#endif
}

static void draw_seconds(struct Layer *layer, GContext *ctx) {
	// Draw the seconds bar-graph.
	// Horizontal on rectangular displays, radial on round ones.
	time_t temp = time(NULL); 
	struct tm *cur_time = localtime(&temp);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	
#if defined(PBL_RECT) // Bar-graph display
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
#elif defined(PBL_ROUND) // Radial
	GRect bounds = layer_get_bounds(layer);
	GRect inner = grect_inset(bounds, GEdgeInsets(SECONDS_RADIAL_WIDTH));
		
	graphics_context_set_antialiased(ctx, true);
		
	for (int i = 0; i <= SECONDS_RADIAL_COUNT; i += 1) {
		if ((cur_time -> tm_sec * 60 / SECONDS_RADIAL_COUNT ) + 1 == i) {
			graphics_context_set_stroke_color(ctx, GColorLightGray);
		}
		int angle = TRIG_MAX_ANGLE * i / SECONDS_RADIAL_COUNT;
		graphics_draw_line(ctx,
			gpoint_from_polar(inner, GOvalScaleModeFitCircle, angle),
			gpoint_from_polar(bounds, GOvalScaleModeFitCircle, angle)
		);
	}

#endif
}

static void draw_icon_bg(struct Layer *layer, GContext *ctx) {
	// Draw all the borders around the icons.
	graphics_context_set_stroke_color(ctx, GColorBlack);
	#if defined(PBL_RECT)
	for (int x = 0; x <= 4; x += 1) {
		for (int y = 0; y <= 1; y += 1) {
			graphics_draw_rect(ctx, GRect(x*18 + 16, y*18, 18, 18));
		}
	}
	#elif defined(PBL_ROUND)
	GRect bounds = layer_get_bounds(layer);
	int right_offset = bounds.size.w - 18;
	for (int y = 0; y <= 4; y += 1) {
		// Left
		graphics_draw_rect(ctx, GRect(0, 18*y, 18, 18));
		// Right
		graphics_draw_rect(ctx, GRect(right_offset, 18*y, 18, 18));
	}
	#endif
}

void show_main_window() {
	initialise_ui();
	window_set_window_handlers(main_win, (WindowHandlers) {
		.unload = handle_window_unload,
	});
	layer_set_update_proc(secs_layer, &draw_seconds);
	layer_set_update_proc(secs_line, &draw_sep_line);
	
	layer_set_update_proc(icon_bg, &draw_icon_bg);
	#ifndef PBL_ROUND
	layer_set_update_proc(icon_line, &draw_sep_line);
	#endif
	
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
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Starting up");
	
	init();
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "initialised");
	show_main_window();
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Shown window");
	
	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *cur_time = localtime(&temp);
	
	// Run these the first time
	time_handler(cur_time, SECOND_UNIT | HOUR_UNIT | MINUTE_UNIT | DAY_UNIT);
	bluetooth_check(bluetooth_connection_service_peek());
	battery_update(battery_state_service_peek());
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done checks");
	
	tick_timer_service_subscribe(SECOND_UNIT | HOUR_UNIT | MINUTE_UNIT | DAY_UNIT, time_handler);
	
	battery_state_service_subscribe(battery_update);
	bluetooth_connection_service_subscribe(bluetooth_check);
	accel_tap_service_subscribe(shake_handler);
	
	shuffle_icons(); // also starts the powerup animation
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Shuffled icons");
	
	app_event_loop();
	
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	accel_tap_service_unsubscribe();
	
	return 0;
}