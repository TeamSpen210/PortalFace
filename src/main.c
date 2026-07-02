#include <pebble.h>
#ifdef TSPEN_DISPLAY_HIGHRES

#ifdef PBL_RECT
#define HAS_OBSTRUCTION
#endif

static Window *main_win;
static BitmapLayer *box_blue;
static BitmapLayer *box_batt;
static BitmapLayer *box_apm;
static BitmapLayer *box_quiet_time;
static TextLayer *box_date;
static TextLayer *hour_text;

// Main window layer
static Layer *root_layer;

#ifdef HAS_OBSTRUCTION
// Slides up for quickview...
static Layer *slide_layer;
static bool is_obstructed = false;
#else // Always untrue.
	#define is_obstructed false
#endif

// The moving bars
static Layer *secs_layer;
// The line above and below the seconds display
static Layer *secs_line;
#ifndef PBL_ROUND
static Layer *icon_line; // There's only one on round displays
#endif

// The backround for icons
static Layer *icon_bg;


static BitmapLayer *ap_logo;

static GBitmap *res_am;
static GBitmap *res_pm;

// Minute digits
static BitmapLayer *min_dig_ten;
static BitmapLayer *min_dig_one;
static GBitmap *res_digit[10];

#ifdef HAS_OBSTRUCTION
static GBitmap *res_digit_sml[10];
static BitmapLayer *min_dig_sml_ten;
static BitmapLayer *min_dig_sml_one;
#endif

// Packed data describing the battery timer dial.
// 12x12 pixels, 
const int QUADRANT = 12;
const unsigned char BATTERY_LAYOUT[72] = "\x88\x88\x88\x88\x88\x88\x12\x82\x88\x88\x88\x88\x30\x30\x80\x88\x88\x88\x12\x12\x12\x82\x88\x88\x30\x30\x30\x88\x84\x88\x12\x12\x82\x58\x86\x88\x30\x30\x88\x74\x84\x88\x12\x82\x58\x56\x56\x88\x30\x88\x74\x74\x74\x88\x82\x58\x56\x56\x56\x86\x88\x74\x74\x74\x74\x84\x88\x88\x88\x88\x88\x88";

static struct {
	unsigned char wedges; // 0-8 wedges
	bool charging; // whether to be blue or orange
	// 0-6, how many pixels to show for the current wedge
	// We have 4 colours, and a 2x2 dither pattern so 4 displayed
	// colors. Sliding one against the other, there's 7 distinct states:
	// [---||||---]
	// [---|||1234]
	// [---||1234-]
	// [---|1234--]
	// [---1234---]
	// [--1234----]
	// [-1234-----]
	// [1234------]
	unsigned char fade;
} battery_state;

const GColor GRADIENT_BLUE[] = {
	GColorBlack,
	GColorFromRGB(0, 0, 85),
	GColorFromRGB(0, 85, 170),
	GColorFromRGB(0, 170, 255),
};
const GColor GRADIENT_ORAN[] = {
	GColorBlack,
	GColorFromRGB(85, 0, 0),
	GColorFromRGB(170, 85, 0),
	GColorFromRGB(255, 170, 0),
};

static GBitmap *res_bluetooth_on;
static GBitmap *res_bluetooth_off; 
static GBitmap *res_quiet_time;

static GBitmap *res_ap_logo;

static bool charge_vibe_done = true;
// Whether the powerup sequence triggered the user light.
static bool powerup_light_enabled = false;

// If user is detected sleeping, suppress seconds animation.
#ifdef PBL_HEALTH
static bool sleep_mode = false;
#endif

// During powerup, play an animation of the seconds bar increasing to the value.
static int max_seconds_bar = 120;

const int HALF_WIDTH = PBL_DISPLAY_WIDTH / 2;
const int HALF_HEIGHT = PBL_DISPLAY_HEIGHT / 2;

const int MIN_PADDING = 6; // Distance beween minute digits
const int ICON_ROUND_OFFSET = 48; // Distance from center to put icons
const int SECONDS_RADIAL_WIDTH = 16; // Length of seconds lines on round display
const int SECONDS_OUTER_PADDING = 6; // Distance from edge
const int SECONDS_RADIAL_COUNT = 120 ; // Number of radial seconds lines
const int SECONDS_INNER_PADDING = 5; // Distance between inner line and seconds bar ring

// Distance the round logo and hour text are inset from the top and bottom
const int ROUND_VERT_INSET = 35;

#define ARRAY_SIZE(x) (int)(sizeof(x) / sizeof(x[0]))
	
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

const int RES_DIGIT_IDS[] = {
	RESOURCE_ID_IMG_NUM_0,
	RESOURCE_ID_IMG_NUM_1,
	RESOURCE_ID_IMG_NUM_2,
	RESOURCE_ID_IMG_NUM_3,
	RESOURCE_ID_IMG_NUM_4,
	RESOURCE_ID_IMG_NUM_5,
	RESOURCE_ID_IMG_NUM_6,
	RESOURCE_ID_IMG_NUM_7,
	RESOURCE_ID_IMG_NUM_8,
	RESOURCE_ID_IMG_NUM_9,
};

#ifdef HAS_OBSTRUCTION
const int RES_DIGIT_SML_IDS[] = {
	RESOURCE_ID_IMG_NUM_SML_0,
	RESOURCE_ID_IMG_NUM_SML_1,
	RESOURCE_ID_IMG_NUM_SML_2,
	RESOURCE_ID_IMG_NUM_SML_3,
	RESOURCE_ID_IMG_NUM_SML_4,
	RESOURCE_ID_IMG_NUM_SML_5,
	RESOURCE_ID_IMG_NUM_SML_6,
	RESOURCE_ID_IMG_NUM_SML_7,
	RESOURCE_ID_IMG_NUM_SML_8,
	RESOURCE_ID_IMG_NUM_SML_9,
};
#endif

const int RES_BATT_IDS[] = {
	RESOURCE_ID_IMG_BAT_1,
	RESOURCE_ID_IMG_BAT_2,
	RESOURCE_ID_IMG_BAT_3,
	RESOURCE_ID_IMG_BAT_4,
	RESOURCE_ID_IMG_BAT_5,
	RESOURCE_ID_IMG_BAT_6,
	RESOURCE_ID_IMG_BAT_7,
	RESOURCE_ID_IMG_BAT_8,
};

GBitmap *ico_bitmap[26];

BitmapLayer *ico_layers[5];


static BitmapLayer **all_box_layers[10] = {
	&box_blue,
	&box_batt,
	&box_apm,
	NULL, // Only one which isn't a BitmapLayer
	&box_quiet_time,
	&ico_layers[0],
	&ico_layers[1],
	&ico_layers[2],
	&ico_layers[3],
	&ico_layers[4],
};

static void quiet_time_update();
static void sleep_update();
static void time_handler(struct tm *tick_time, TimeUnits units_changed);
#ifdef HAS_OBSTRUCTION
static void unobstructed_set_vis(bool obstructed);
#endif

int max(int a, int b) {
	return a > b ? a : b;
}
int min(int a, int b) {
	return a < b ? a : b;
}

GRect box_pos(int off, bool second_row) {
	// Return the rect matching a specific box position.
	// If second_row is true, it's the right/bottom row.
	return GRect(
	#ifdef PBL_ROUND
		(second_row) ? HALF_WIDTH + ICON_ROUND_OFFSET : HALF_WIDTH - ICON_ROUND_OFFSET - 24, 
		(PBL_DISPLAY_HEIGHT - 5*26)/2 + (off) * 26,
	#else
		35 + (off)*26,
		142 + ((second_row) ? 26: 0),
	#endif
	24, 24);
}

static void initialise_ui(void) {
	main_win = window_create();
	
	root_layer = window_get_root_layer(main_win);
	
	GRect bounds = layer_get_bounds(root_layer);
	
	
	#if defined(PBL_RECT) // On rectangular displays, offset upwards
		min_dig_ten = bitmap_layer_create(GRect(HALF_WIDTH - 40 - MIN_PADDING/2, 3, 40, 110));
		min_dig_one = bitmap_layer_create(GRect(HALF_WIDTH + MIN_PADDING/2, 3, 40, 110));
	#elif defined(PBL_ROUND) // Round displays center the icons
		GRect min_pos = (GRect){.size = GSize(80 + MIN_PADDING, 110)};
		grect_align(&min_pos, &bounds, GAlignCenter, false);
		
		// minute tens digit
		min_dig_ten = bitmap_layer_create(grect_inset(
			min_pos, 
			GEdgeInsets(0, MIN_PADDING/2 + 40, 0, -MIN_PADDING/2)
		));

		// minte ones digit
		min_dig_one = bitmap_layer_create(grect_inset(
			min_pos, 
			GEdgeInsets(0, -MIN_PADDING/2, 0, MIN_PADDING/2 + 40)
		));
	#endif
	
	// The digits don't move, they get replaced by a smaller display
	layer_add_child(root_layer, (Layer *)min_dig_ten);
	layer_add_child(root_layer, (Layer *)min_dig_one);
	
	
	#ifdef HAS_OBSTRUCTION

		slide_layer = layer_create(bounds);
		layer_add_child(root_layer, (Layer *)slide_layer);

		min_dig_sml_ten = bitmap_layer_create(GRect(HALF_WIDTH - 24 - MIN_PADDING/2, 50, 24, 56));
		min_dig_sml_one = bitmap_layer_create(GRect(HALF_WIDTH + MIN_PADDING/2, 50, 24, 56));

		layer_add_child(slide_layer, (Layer *)min_dig_sml_ten);
		layer_add_child(slide_layer, (Layer *)min_dig_sml_one);
		
		#define ADD(child_layer) layer_add_child(slide_layer, (Layer *)child_layer)
	#else
		#define ADD(child_layer) layer_add_child(root_layer, (Layer *)child_layer)
	#endif

	// seconds bar layer. On rect displays it's constrained, but it covers everything on round.
	#ifdef PBL_RECT
		secs_layer = layer_create(GRect(15, 121, 172, 12));
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
	bitmap_layer_set_compositing_mode(box_batt, GCompOpSet);
	bitmap_layer_set_background_color(box_batt, COLOR_BLUE);
	bitmap_layer_set_bitmap(box_batt, res_batt[3]);
	ADD(box_batt);
	
	// aperture logo
	// On round displays it's just the aperture, and bottom-centered
	#ifdef PBL_RECT
		ap_logo = bitmap_layer_create(GRect(6, 200, 96, 24));
	#else
		GRect ap_logo_pos = (GRect){.size = GSize(24, 24)};
		grect_align(&ap_logo_pos, &bounds, GAlignBottom, false);
		ap_logo = bitmap_layer_create(grect_inset(
			ap_logo_pos,
			GEdgeInsets(-ROUND_VERT_INSET, 0, ROUND_VERT_INSET)
		));
	#endif
	bitmap_layer_set_bitmap(ap_logo, res_ap_logo);
	// The logo doesn't move, it just disappears.
	layer_add_child(root_layer, (Layer *)ap_logo);

	// weekday textbox
	GRect box_date_pos = box_pos(1, false);
	// It's not aligned perfectly, we need to adjust slightly.
	box_date_pos.origin.y -= 5;
	box_date = text_layer_create(box_date_pos);
	text_layer_set_background_color(box_date, GColorWhite);
	text_layer_set_text_color(box_date, GColorBlack);
	text_layer_set_text_alignment(box_date, GTextAlignmentCenter);
	text_layer_set_font(box_date, fonts_get_system_font(FONT_KEY_GOTHIC_24));
	ADD(box_date);
	
	// am/pm bitmap
	box_apm = bitmap_layer_create(box_pos(3, false));
	bitmap_layer_set_bitmap(box_apm, res_am);
	ADD(box_apm);
	
	// seconds line
	#if defined(PBL_RECT)
		secs_line = layer_create(GRect(14, 115, 170, 2));
	#elif defined(PBL_ROUND)
		// On round watches, it's positioned inside the seconds ring.
		// We need to inset the width of the seconds ring, plus our paddings.
		secs_line = layer_create(grect_inset(bounds, 
			GEdgeInsets(SECONDS_OUTER_PADDING + SECONDS_RADIAL_WIDTH)
		));
	#endif
	ADD(secs_line);
	
	// hour text
	#if defined(PBL_RECT)
		hour_text = text_layer_create(GRect(14, 92, 40, 20));
	#elif defined(PBL_ROUND)
		// This is positioned opposite to the aperture logo
		GRect hour_pos = (GRect){.size = GSize(60, 28)};
		grect_align(&hour_pos, &bounds, GAlignTop, false);
		hour_text = text_layer_create(grect_inset(
			hour_pos,
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
	text_layer_set_font(hour_text, fonts_get_system_font(PBL_IF_ROUND_ELSE(
		FONT_KEY_GOTHIC_24,
		FONT_KEY_GOTHIC_18
	)));
	ADD(hour_text);	
	
	
	// Icon line - we only use 1 on round watches
	#ifndef PBL_ROUND
	icon_line = layer_create(GRect(14, 115, 170, 2));
	ADD(icon_line);
	#endif
	
	//Icon background - on round it's split up!
	#ifdef PBL_ROUND
		GRect first_rect = box_pos(0, false);
		GRect last_rect = box_pos(4, true);
		// The round icon background needs to be the bounding box for each position, + 1 on each side.
		icon_bg = layer_create(GRect(
				first_rect.origin.x - 1, 
				first_rect.origin.y - 1, 
			    last_rect.origin.x+last_rect.size.w - first_rect.origin.x + 2,
			    last_rect.origin.y+last_rect.size.h - first_rect.origin.y + 2
			));
	#else
		icon_bg = layer_create(GRect(32, 141, 170, 56));
	#endif
		ADD(icon_bg);
	
	// Init all the bitmap icons
	for (int i=0; i<ARRAY_SIZE(ico_layers); i++) {
		ico_layers[i] = bitmap_layer_create(box_pos(i, true));
		bitmap_layer_set_bitmap(ico_layers[i], ico_bitmap[i]);
		ADD(ico_layers[i]);
	}
	box_quiet_time = bitmap_layer_create(box_pos(2, false));
	bitmap_layer_set_bitmap(box_quiet_time, ico_bitmap[5]);
	ADD(box_quiet_time);
	
	#ifdef HAS_OBSTRUCTION
	GRect unobstucted_bounds = layer_get_unobstructed_bounds(root_layer);
	is_obstructed = !grect_equal(&unobstucted_bounds, &bounds);
	unobstructed_set_vis(is_obstructed);
	if (is_obstructed) {
	    // Force the slide-frame to be in the correct position
	    GRect slide_frame = layer_get_frame((Layer *) slide_layer);
	    slide_frame.origin.y = min(0, unobstucted_bounds.size.h - bounds.size.h + 20);
	    layer_set_frame((Layer *) slide_layer, slide_frame);
	    layer_mark_dirty((Layer *) slide_layer);
	}
	#endif
}

static void handle_window_unload(Window* window) {
	window_destroy(main_win);
#ifdef HAS_OBSTRUCTION
	layer_destroy(slide_layer);
#endif
	
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

#ifdef HAS_OBSTRUCTION
	bitmap_layer_destroy(min_dig_sml_ten);
	bitmap_layer_destroy(min_dig_sml_one);
#endif

	bitmap_layer_destroy(ap_logo);

	gbitmap_destroy(res_bluetooth_on);
	gbitmap_destroy(res_bluetooth_off);
	gbitmap_destroy(res_quiet_time);
	
	gbitmap_destroy(res_ap_logo);
	
	gbitmap_destroy(res_am);
	gbitmap_destroy(res_pm);

	for (int i=0; i<ARRAY_SIZE(res_digit); i++) {
		gbitmap_destroy(res_digit[i]);
#ifdef HAS_OBSTRUCTION
		gbitmap_destroy(res_digit_sml[i]);
#endif
	}

	for (int i=0; i<ARRAY_SIZE(res_batt); i++) {
		gbitmap_destroy(res_batt[i]);
	}

	for (int i=0; i<ARRAY_SIZE(ico_layers); i++) {
		bitmap_layer_destroy(ico_layers[i]);
	}
	for (int i=0; i<ARRAY_SIZE(ico_bitmap); i++) {
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
	HIDE(box_quiet_time);
	
	HIDE(secs_layer);
	HIDE(secs_line);
	HIDE(hour_text);
	
	HIDE(min_dig_ten);
	HIDE(min_dig_one);
#ifdef HAS_OBSTRUCTION
	HIDE(min_dig_sml_ten);
	HIDE(min_dig_sml_one);
#endif

	HIDE(ap_logo);
	for (int i=0; i<ARRAY_SIZE(ico_layers); i++) {
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
	if ( !is_obstructed ) {
		SHOW(ap_logo);
	}
	
	// Hide everything from powerup_nums to allow switching between them
	HIDE(min_dig_ten);
	HIDE(min_dig_one);
	HIDE(hour_text);
	HIDE(icon_bg);
}

void powerup_nums(void *val) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Nums");
	#ifdef HAS_OBSTRUCTION
	if ( is_obstructed ) {
		SHOW(min_dig_sml_ten);
		SHOW(min_dig_sml_one);
	} else 
	#endif
	{
		SHOW(min_dig_ten);
		SHOW(min_dig_one);
	}
	SHOW(hour_text);
	SHOW(icon_bg);
	if (powerup_light_enabled) {
		light_enable_interaction(); // Keep light on.
	}
}

void powerup_box(void *layer) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Box");
	SHOW(layer);

	if (powerup_light_enabled) {
		light_enable_interaction(); // Keep light on.
	}
}
void powerup_date(void *) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Date Box");
	SHOW(box_date);

	if (powerup_light_enabled) {
		light_enable_interaction(); // Keep light on.
	}
}

static bool playing_powerup = false;
	
void powerup_progress_start(void *val) {
	// Show the seconds, but blank them
	max_seconds_bar = 0;
	layer_mark_dirty(secs_layer);
	SHOW(secs_layer);
}

void powerup_progress(void *val) {
	max_seconds_bar += 2;
	if (powerup_light_enabled) {
		light_enable_interaction(); // Keep light on.
	}
	layer_mark_dirty(secs_layer);
}

void powerup_done(void *val) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Powerup - Complete");

	max_seconds_bar = 60;

	if (powerup_light_enabled) {
		light_enable_interaction();
	}
	playing_powerup = powerup_light_enabled = false;
}

void powerup_shuffle() {
	int i, j;
	BitmapLayer **tmp;
	for (i=9; i>0; i--){
		j = rand() % (i+1);
		tmp = all_box_layers[i];
		all_box_layers[i] = all_box_layers[j];
		all_box_layers[j] = tmp;
	}
}

#undef SHOW
#undef HIDE

static void powerup(bool force_backlight) {
	#ifdef PBL_HEALTH
	// If forcing backlight, kick out of sleep mode.
	if (force_backlight) {
		sleep_mode = false;
	}
	#endif

	// Play the light flickering animation.
	if (playing_powerup){
		return; // Don't recurse
	}
	playing_powerup = true;
	powerup_light_enabled = force_backlight;

	// Shuffle turnon order:
	powerup_shuffle();

	powerdown();  // Hide everything first

	app_timer_register( 150, &powerup_lines, NULL);
	app_timer_register( 300, &powerup_nums, NULL);
	app_timer_register( 400, &powerup_logo, NULL);
	app_timer_register( 600, &powerup_nums, NULL);
	app_timer_register( 800, &powerup_logo, NULL);
	app_timer_register( 900, &powerup_progress_start, NULL);
	app_timer_register( 900, &powerup_nums, NULL);

	for (int i = 0; i < 10; ++i)
	{
		if (all_box_layers[i] == NULL)
		{
			app_timer_register(1100 + 10 * i, &powerup_date, NULL);
		}
		else
		{
			BitmapLayer *box = *all_box_layers[i];
			app_timer_register(1100 + 10 * i, &powerup_box, (void *)box);
		}
	}

	for(int i=1125; i <= 1900; i += PBL_IF_ROUND_ELSE(15, 25)) {
		app_timer_register(i, &powerup_progress, NULL);
	}
	app_timer_register(1900, &powerup_done, NULL);
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
	// Draw the line that separates seconds from the boxes or the testchamber number.
	graphics_draw_line(ctx, GPoint(0,0), GPoint(172, 0));
	#endif
}

static void draw_seconds(struct Layer *layer, GContext *ctx) {
	// Draw the seconds bar-graph.
	// Horizontal on rectangular displays, radial on round ones.
	time_t temp = time(NULL); 
	struct tm *cur_time = localtime(&temp);

	GRect bounds = layer_get_frame(layer);

#if defined(PBL_RECT) // Bar-graph display
	// In powerup mode, limit to max_seconds_bar size at most.
	int sec_count = cur_time->tm_sec;
	if ( max_seconds_bar < sec_count )
		sec_count = max_seconds_bar;
	int sec_pos = sec_count * 170 / 60;
	sec_pos -= sec_pos % 2;
	if ( sec_pos < 2) {
		sec_pos = 2;
	}

	// Just write to the framebuffer directly for efficiency.
  GBitmap *fb = graphics_capture_frame_buffer(ctx);
	for (int y = 0; y <= 12; y++) {
		GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, bounds.origin.y + y);
		for (int x = 0; x < 170; x += 2) {
			GColor color = ( x >= sec_pos ) ? GColorLightGray : GColorBlack;
			memset(&info.data[bounds.origin.x + x], color.argb, 1);
		}
	}
  graphics_release_frame_buffer(ctx, fb);

#elif defined(PBL_ROUND) // Radial
  // Can't use framebuffer here, since we need angled lines.
	GRect inner = grect_inset(bounds, GEdgeInsets(SECONDS_RADIAL_WIDTH));

	graphics_context_set_antialiased(ctx, true);

	for (int i = 0; i <= SECONDS_RADIAL_COUNT; i += 1) {
		if ((cur_time -> tm_sec * SECONDS_RADIAL_COUNT / 60 ) + 1 == i) {
			graphics_context_set_stroke_color(ctx, GColorLightGray);
		}
		if ((max_seconds_bar * SECONDS_RADIAL_COUNT / 120 ) + 1 == i) {
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
			graphics_draw_rect(ctx, GRect(x*26 + 2, y*26, 26, 26));
		}
	}
	#elif defined(PBL_ROUND)
	GRect bounds = layer_get_bounds(layer);
	int right_offset = bounds.size.w - 26;
	for (int y = 0; y <= 4; y += 1) {
		// Left
		graphics_draw_rect(ctx, GRect(0, 26*y, 26, 26));
		// Right
		graphics_draw_rect(ctx, GRect(right_offset, 26*y, 26, 26));
	}
	#endif
}

#ifdef HAS_OBSTRUCTION
static void unobstructed_start(GRect final_area, void *context);
static void unobstructed_anim(AnimationProgress progress, void *context);
static void unobstructed_end(void *context);
#endif

static void handle_window_return(Window * window) {
	quiet_time_update();
}

void show_main_window() {
	initialise_ui();

#ifdef HAS_OBSTRUCTION
  UnobstructedAreaHandlers handlers = {
    .will_change = &unobstructed_start,
    .change = &unobstructed_anim,
    .did_change = &unobstructed_end
  };
	unobstructed_area_service_subscribe(handlers, NULL);
#endif

	window_set_window_handlers(main_win, (WindowHandlers) {
		.unload = handle_window_unload,
		.appear = handle_window_return,
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
	} else {
		bitmap_layer_set_bitmap(box_blue, res_bluetooth_off);
	}
}

static void shuffle_icons(bool force_backlight) {
	// Rearrange the icon array and apply it to the display.
	int i, j;
	GBitmap *tmp;
	for (i=(NUM_ICONS-1); i>0; i--){
		j = rand() % (i+1);
		tmp = ico_bitmap[i];
		ico_bitmap[i] = ico_bitmap[j];
		ico_bitmap[j] = tmp;
	}
	
	for (i=0; i<ARRAY_SIZE(ico_layers); i++) {
		bitmap_layer_set_bitmap(ico_layers[i], ico_bitmap[i]);
	}
	quiet_time_update(); // This uses icon number 5 if disabled.
	powerup(force_backlight); // "Restart" the screen
}

static void shake_handler(AccelAxisType axis, int32_t dir) {
	// On shakes, shuffle the icons.
	shuffle_icons(true);
}

static void battery_update(BatteryChargeState state) {
	// We have 0-8 wedges, evenly spread that.
	// We want the crossover point to happen in-between the wedges, so
	// Calculate (charge / 100 + 1/16) // 8 -> *= 400/400
	// = ( 4 * charge + 25 ) / 50;
	int wedge = (state.charge_percent * 4 + 25) / 50;
	if (wedge > 8) { // Shouldn't happen, just in case
		wedge = 8;
	}

	// If charging, flash between icons.
	if (state.is_charging) {
		time_t temp = time(NULL);
		struct tm *cur_time = localtime(&temp);
		if (cur_time -> tm_sec % 2 == 0) {
			// Special case - if we're showing 8 wedges, oscillate down to 7.
			wedge = (wedge == 8) ? 7 : wedge + 1;
		}
	}

	bitmap_layer_set_background_color(box_batt, state.is_plugged ? COLOR_ORAN : COLOR_BLUE);
	bitmap_layer_set_bitmap(box_batt, res_batt[wedge]);

	if (state.charge_percent == 100) {
		// Only trigger vibration if we just switched states.
		if (!charge_vibe_done) {
			vibes_double_pulse();
			charge_vibe_done = true;
		}
	} else
		{
		charge_vibe_done = false;
	}
}

static void quiet_time_update() {
	if ( quiet_time_is_active() ) {
		bitmap_layer_set_bitmap( box_quiet_time, res_quiet_time );
	} else {
		bitmap_layer_set_bitmap( box_quiet_time, ico_bitmap[5] );
	}
}

static void sleep_update() {
#ifdef PBL_HEALTH
		// Check if the user is sleeping, if so disable the seconds display.
		HealthActivityMask activities = health_service_peek_current_activities();
		bool new_sleep = (activities & (HealthActivitySleep | HealthActivityRestfulSleep)) != 0;
		if ( sleep_mode != new_sleep ) {
			layer_set_hidden(secs_layer, sleep_mode);
			tick_timer_service_subscribe(HOUR_UNIT | MINUTE_UNIT | DAY_UNIT | (new_sleep ? SECOND_UNIT : 0), time_handler);
			sleep_mode = new_sleep;
		}
#endif
}

#ifdef HAS_OBSTRUCTION
// Handle animating when quickview appears / unobstructed-area

static void unobstructed_set_vis(bool obstructed) {
  layer_set_hidden((Layer *)ap_logo, obstructed);
  layer_set_hidden((Layer *)min_dig_one, obstructed);
  layer_set_hidden((Layer *)min_dig_ten, obstructed);
  layer_set_hidden((Layer *)min_dig_sml_one, !obstructed);
  layer_set_hidden((Layer *)min_dig_sml_ten, !obstructed);
}

static void unobstructed_start(GRect final_area, void *context) {
  GRect full_bounds = layer_get_bounds((Layer *)root_layer);
  is_obstructed = !grect_equal(&full_bounds, &final_area);
  if (is_obstructed) {
    // Appearing, hide things
    unobstructed_set_vis(true);
  }
	// See if the user woke up.
  sleep_update();
}

// Run while it's changing
static void unobstructed_end(void *context) {
  GRect full_bounds = layer_get_bounds((Layer *)root_layer);
  GRect bounds = layer_get_unobstructed_bounds((Layer *)root_layer);
  
  is_obstructed = !grect_equal(&full_bounds, &bounds);
  if (!is_obstructed) {
    // Screen is no longer obstructed, show stuff
    unobstructed_set_vis(false);
    
    // Force the slide-frame to be in the correct position
    GRect slide_frame = layer_get_frame((Layer *) slide_layer);
    slide_frame.origin.y = 0;
    layer_set_frame((Layer *) slide_layer, slide_frame);
    layer_mark_dirty((Layer *) slide_layer);
  }
	// See if the user woke up.
  sleep_update();
}

static void unobstructed_anim(AnimationProgress progress, void *context) {
  GRect full_bounds = layer_get_bounds((Layer *)root_layer);
  GRect bounds = layer_get_unobstructed_bounds((Layer *)root_layer);

  GRect slide_frame = layer_get_frame((Layer *) slide_layer);

  slide_frame.origin.y = min(0, bounds.size.h - full_bounds.size.h + 20);
  layer_set_frame((Layer *) slide_layer, slide_frame);
}
#endif

static void time_handler(struct tm *tick_time, TimeUnits units_changed) {
	if ((units_changed & SECOND_UNIT) != 0) {
#ifdef PBL_HEALTH
		if (!sleep_mode)
#endif
		{
			layer_mark_dirty(secs_layer);
		}

		BatteryChargeState st = battery_state_service_peek();
		if (st.is_charging) {
			battery_update(st);
		}
	}

	if ((units_changed & MINUTE_UNIT) != 0) {
		static char min_char[] = "00";
		strftime(min_char, sizeof("--"), "%M", tick_time);

		bitmap_layer_set_bitmap(min_dig_ten, res_digit[min_char[0] - '0']);
		bitmap_layer_set_bitmap(min_dig_one, res_digit[min_char[1] - '0']);
#ifdef HAS_OBSTRUCTION
		bitmap_layer_set_bitmap(min_dig_sml_ten, res_digit_sml[min_char[0] - '0']);
		bitmap_layer_set_bitmap(min_dig_sml_one, res_digit_sml[min_char[1] - '0']);
#endif
		sleep_update();
	}

	if ((units_changed & DAY_UNIT) != 0) {
		static char date_char[] = "22";
		strftime(date_char, sizeof("--"), "%d", tick_time);
		text_layer_set_text(box_date, date_char);
	}

	if ((units_changed & HOUR_UNIT) !=0) {
		shuffle_icons(false);

		static char hour_char[] = "00/19";
		if (clock_is_24h_style()) {
			// Reconstructing style test track chamber count
			strftime(hour_char, sizeof("-----"), "%H/34", tick_time);
		} else {
			// Portal 1 chamber count
			strftime(hour_char, sizeof("-----"), "%I/19", tick_time);
		}
		text_layer_set_text(hour_text, hour_char);

		if ((tick_time->tm_hour) < 12) {
			bitmap_layer_set_bitmap(box_apm, res_am);
		} else {
			bitmap_layer_set_bitmap(box_apm, res_pm);
		}

	}
}

static void init() {
	res_bluetooth_on = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUE_ON);
	res_bluetooth_off = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLUE_OFF);

	res_quiet_time = gbitmap_create_with_resource(RESOURCE_ID_IMG_QUIET_TIME);
	res_ap_logo = gbitmap_create_with_resource(RESOURCE_ID_IMG_AP_LOGO);

	res_am = gbitmap_create_with_resource(RESOURCE_ID_TS_ICO_AM);
	res_pm = gbitmap_create_with_resource(RESOURCE_ID_TS_ICO_PM);

	res_batt[0] = gbitmap_create_blank(GSize(24, 24), GBitmapFormat8Bit);
	for (int i=1; i<ARRAY_SIZE(res_batt); i++) {
		res_batt[i] = gbitmap_create_with_resource(RES_BATT_IDS[i - 1]);
	}

	for (int i=0; i<ARRAY_SIZE(res_digit); i++) {
		res_digit[i] = gbitmap_create_with_resource(RES_DIGIT_IDS[i]);
#ifdef HAS_OBSTRUCTION
		res_digit_sml[i] = gbitmap_create_with_resource(RES_DIGIT_SML_IDS[i]);
#endif
	}
	
	for (int i=0; i<ARRAY_SIZE(ico_bitmap); i++) {
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

	shuffle_icons(true); // also starts the powerup animation
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Shuffled icons");

	app_event_loop();

	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	bluetooth_connection_service_unsubscribe();
	accel_tap_service_unsubscribe();

	return 0;
}

#endif // defined(TSPEN_DISPLAY_HIGHRES)
