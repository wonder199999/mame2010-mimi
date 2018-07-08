#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	#include "retroogl.c"
#endif

retro_log_printf_t log_cb = NULL;
retro_environment_t environ_cb = NULL;
retro_video_refresh_t video_cb = NULL;

static retro_input_state_t input_state_cb = NULL;
static retro_audio_sample_batch_t audio_batch_cb = NULL;
static retro_input_poll_t input_poll_cb = NULL;

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { }
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }

void retro_set_environment(retro_environment_t cb)
{
	static const struct retro_variable vars[] = {
      	{ "mame_mini_cpu_overclock",	"Adjust CPU speed; disabled|110%|120%|130%|140%|150%|160%|170%|180%|190%|200%|50%|60%|70%|80%|90%" },
      	{ "mame_mini_frame_skip", 	"Set frameskip; 0|1|2|3|4|automatic" },
	{ "mame_mini_aspect_ratio",	"Core provided aspect ratio; DAR|PAR" },
      	{ "mame_mini_turbo_button", 	"Enable autofire; disabled|button 1|button 2|R2 to button 1 mapping|R2 to button 2 mapping" },
      	{ "mame_mini_turbo_delay", 	"Set autofire pulse speed; medium|slow|fast" },
      	{ "mame_mini_kb_input", 	"Keyboard input; enabled|disabled" },
      	{ "mame_mini_macro_button", 	"Use macro button; disabled|assign A+B to L|assign A+B to R|assign C+D to L|assign C+D to R|assign A+B to L & C+D to R|assign A+B to R & C+D to L" },
      	{ "mame_mini_tate_mode", 	"T.A.T.E mode(Restart); disabled|enabled" },
      	{ "mame_mini_sample_rate", 	"Set sample rate (Restart); 48000Hz|44100Hz|32000Hz|22050Hz" },
	{ "mame_mini_rom_hash",		"Forced off ROM CRC verfiy(Restart); No|Yes" },
      	{ "mame_mini_adj_brightness",
	   "Set brightness; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },
      	{ "mame_mini_adj_contrast",
	   "Set contrast; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },
      	{ "mame_mini_adj_gamma",
	   "Set gamma; default|+1%|+2%|+3%|+4%|+5%|+6%|+7%|+8%|+9%|+10%|+11%|+12%|+13%|+14%|+15%|+16%|+17%|+18%|+19%|+20%|-20%|-19%|-18%|-17%|-16%|-15%|-14%|-13%|-12%|-11%|-10%|-9%|-8%|-7%|-6%|-5%|-4%|-3%|-2%|-1%" },

      	{ NULL, NULL },
   	};

   	environ_cb = cb;
   	cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
}

static void check_variables(void)
{
	struct retro_variable var = { 0 };
   	bool tmp_ar = set_par;

   	var.key = "mame_mini_aspect_ratio";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
      		if (!strcmp(var.value, "PAR"))
			set_par = true;
		else
			set_par = false;
   	}

   	var.key = "mame_mini_tate_mode";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
      		if (!strcmp(var.value, "enabled"))
			tate = 1;
		else
			tate = 0;
   	}
	else tate = 0;

   	var.key = "mame_mini_kb_input";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
      		if (!strcmp(var.value, "enabled"))
			keyboard_input = true;
		else
			keyboard_input = false;
   	}

	var.key = "mame_mini_frame_skip";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
		int temp_fs = set_frame_skip;
		if (!strcmp(var.value, "automatic"))
			set_frame_skip = -1;
		else
			set_frame_skip = atoi(var.value);

		if (temp_fs != set_frame_skip)
			video_set_frameskip(set_frame_skip);
   	}

   	var.key = "mame_mini_turbo_button";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
      		if (!strcmp(var.value, "button 1"))
			turbo_enable = 1;
      		else if (!strcmp(var.value, "button 2"))
			turbo_enable = 2;
      		else if (!strcmp(var.value, "R2 to button 1 mapping"))
			turbo_enable = 3;
      		else if (!strcmp(var.value, "R2 to button 2 mapping"))
			turbo_enable = 4;
      		else
			turbo_enable = 0;
   	}

   	var.key = "mame_mini_turbo_delay";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
      		if (!strcmp(var.value, "medium"))
			turbo_delay = 5;
      		else if (!strcmp(var.value, "slow"))
			turbo_delay = 7;
		else
			turbo_delay = 3;
   	}

   	var.key = "mame_mini_sample_rate";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
		sample_rate = atoi(var.value);

   	var.key = "mame_mini_adj_brightness";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
		float temp_value = arroffset[0];
		if (!strcmp(var.value, "default"))
			arroffset[0] = 0.0;
		else
			arroffset[0] = (float)atoi(var.value) / 100.0f;

		if (temp_value != arroffset[0])
			adjust_opt[0] = adjust_opt[3] = 1;
   	}

   	var.key = "mame_mini_adj_contrast";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
		float temp_value = arroffset[1];
		if (!strcmp(var.value, "default"))
			arroffset[1] = 0.0;
		else
			arroffset[1] = (float)atoi(var.value) / 100.0f;

		if (temp_value != arroffset[1])
			adjust_opt[0] = adjust_opt[4] = 1;
   	}

   	var.key = "mame_mini_adj_gamma";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
		float temp_value = arroffset[2];
		if (!strcmp(var.value, "default"))
			arroffset[2] = 0.0;
		else
			arroffset[2] = (float)atoi(var.value) / 100.0f;

		if (temp_value != arroffset[2])
			adjust_opt[0] = adjust_opt[5] = 1;
   	}

   	var.key = "mame_mini_cpu_overclock";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
		float temp_value = arroffset[3];
		if (!strcmp(var.value, "disabled"))
			arroffset[3] = 1.0f;
		else
			arroffset[3] = (float)atoi(var.value) / 100.0f;

		if (temp_value != arroffset[3])
			adjust_opt[0] = adjust_opt[6] = 1;
   	}

   	var.key = "mame_mini_macro_button";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   	{
		if (!strcmp(var.value, "disabled"))
			macro_state = 0;
		else if (!strcmp(var.value, "assign A+B to L"))
			macro_state = 1;
		else if (!strcmp(var.value, "assign A+B to R"))
			macro_state = 2;
		else if (!strcmp(var.value, "assign C+D to L"))
			macro_state = 3;
		else if (!strcmp(var.value, "assign C+D to R"))
			macro_state = 4;
		else if (!strcmp(var.value, "assign A+B to L & C+D to R"))
			macro_state = 5;
		else if (!strcmp(var.value, "assign A+B to R & C+D to L"))
			macro_state = 6;

		if (!macro_enable)
			macro_state = 0;
	}

   	var.key = "mame_mini_rom_hash";
   	var.value = NULL;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		if (!strcmp(var.value, "No"))
			verify_rom_hash = true;
		else
			verify_rom_hash = false;
	}

   	if (tmp_ar != set_par)
		update_geometry();
}

unsigned retro_api_version(void)
{
	return RETRO_API_VERSION;
}

static void update_geometry(void)
{
	struct retro_system_av_info av_info;
   	retro_get_system_av_info(&av_info);
   	environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &av_info);
}

static void init_input_descriptors(void)
{
#define describe_buttons(INDEX) \
		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    	"Joystick Up" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  	"Joystick Down" }, \
		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,	"Joystick Left" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, 	"Joystick Right" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     	"Button 1" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     	"Button 2" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     	"Button 3" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     	"Button 4" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     	"Button 5" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     	"Button 6" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2,     "Turbo Button" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,	"Insert Coin" }, \
   		{ INDEX, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,	"Start" },

	struct retro_input_descriptor desc[] = {
   		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2,     	"UI Menu" },
   		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L3,     	"Service" },
   		{ 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R3,     	"Framerate" },
      		describe_buttons(0)
      		describe_buttons(1)
      		describe_buttons(2)
      		describe_buttons(3)
      		{ 0, 0, 0, 0, NULL }
   	};
   	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

void retro_get_system_info(struct retro_system_info *info)
{
	memset(info, 0, sizeof(*info));
   	info->library_name = "M.B.A mini";
#ifndef GIT_VERSION
	#define GIT_VERSION ""
#endif
   	info->library_version = GIT_VERSION " (M.B.A = MAME's skeleton + FBA's romsets)";
   	info->valid_extensions = "zip|chd|7z";
   	info->need_fullpath = true;
   	info->block_extract = true;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	int width =  (tate && vertical) ? rthe : rtwi;
	int height = (tate && vertical) ? rtwi : rthe;

	info->geometry.base_width   = width;
   	info->geometry.base_height  = height;

   	info->geometry.max_width    = width * 2;
   	info->geometry.max_height   = height * 2;

   	float display_ratio = set_par ? (vertical && !tate) ? (float)height / (float)width : (float)width / (float)height : (vertical && !tate) ? 3.0f / 4.0f : 4.0f / 3.0f;
   	info->geometry.aspect_ratio = display_ratio;

   	info->timing.fps            = refresh_rate;
   	info->timing.sample_rate    = (double)sample_rate;

#if 1	/* Test */
	int common_factor = 1;
	if (set_par)
	{
		int temp_width = rtwi;
		int temp_height = rthe;
		while (temp_width != temp_height)
		{
			if (temp_width > temp_height)
				temp_width -= temp_height;
			else
				temp_height -= temp_width;
		}
		common_factor = temp_height;
	}
	write_log("Current aspect ratio = %d : %d , screen refresh rate = %f , sound sample rate = %.1f \n", set_par ? vertical ? height / common_factor : width / common_factor :
			vertical ? 3 : 4, set_par ? vertical ? width / common_factor : height / common_factor : vertical ? 4 : 3, info->timing.fps, info->timing.sample_rate);
#endif
}

void retro_init (void) { }

void retro_deinit(void)
{
	if (retro_load_ok)
		retro_finish();

	LOGI("Retro DeInit\n");
}

void retro_reset (void)
{
	mame_reset = 1;
}

void retro_run (void)
{
	bool updated = false;
   	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      		check_variables();

	turbo_state > turbo_delay ? turbo_state = 0 : turbo_state++;

	retro_poll_mame_input();
	retro_main_loop();

	RLOOP = 1;

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
	do_gl2d();
#else
	if (draw_this_frame)
      		video_cb(videoBuffer, rtwi, rthe, topw << PITCH);
   	else
      		video_cb(	NULL, rtwi, rthe, topw << PITCH);
#endif
}

void prep_retro_rotation(int rot)
{
	LOGI("Rotation:%d\n", rot);
   	environ_cb(RETRO_ENVIRONMENT_SET_ROTATION, &rot);
}

bool retro_load_game(const struct retro_game_info *info)
{
	struct retro_log_callback log;
	char basename[128];
#ifdef M16B
	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
#else
   	enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
#endif
   	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
   	{
      		fprintf(stderr, "RGB pixel format is not supported.\n");
      		exit(0);
   	}
#ifdef M16B
	memset(videoBuffer, 0, 512 * 512 * 2);
#else
   	memset(videoBuffer, 0, 512 * 512 * 2 * 2);
#endif
   	check_variables();

#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
   #ifdef HAVE_OPENGLES
	hw_render.context_type = RETRO_HW_CONTEXT_OPENGLES2;
   #else
	hw_render.context_type = RETRO_HW_CONTEXT_OPENGL;
   #endif
	hw_render.context_reset = context_reset;
   	hw_render.context_destroy = context_destroy;

   	if (!environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
		return false;
#endif
	init_input_descriptors();

	basename[0] = '\0';
   	extract_basename(basename, info->path, sizeof(basename));
   	extract_directory(retro_content_dir, info->path, sizeof(retro_content_dir));
   	strcpy(RPATH, info->path);

	int result = mmain(1, RPATH);
   	if (result != 1)
	{
        	printf("Error: mame return an error\n");
		return 0;
   	}

	retro_load_ok = true;

   	video_set_frameskip(set_frame_skip);

   	for (int i = 0; i < 7; i++)
		adjust_opt[i] = 1;

   	if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
		log_cb = log.log;
   	else
		log_cb = NULL;

   	if (log_cb)
      		log_cb(RETRO_LOG_INFO, "CONTENT_DIRECTORY: %s", retro_content_dir);

	return 1;
}

void retro_unload_game(void)
{
	if (!pauseg)
		pauseg = 1;

	LOGI("Retro unload_game \n");
}
