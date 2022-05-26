#pragma once

/*
This list matches GLFW 3.3.4.
It merely exists so applications using txtquad don't need to include the GLFW header.
It comes in 3 variants:
	UPPER_SNAKE_CASE (original)
	lower_snake_case
	PascalCase
*/

// UPPER_SNAKE_CASE
#define TQ_KEY_SPACE              32
#define TQ_KEY_APOSTROPHE         39  /* ' */
#define TQ_KEY_COMMA              44  /* , */
#define TQ_KEY_MINUS              45  /* - */
#define TQ_KEY_PERIOD             46  /* . */
#define TQ_KEY_SLASH              47  /* / */
#define TQ_KEY_0                  48
#define TQ_KEY_1                  49
#define TQ_KEY_2                  50
#define TQ_KEY_3                  51
#define TQ_KEY_4                  52
#define TQ_KEY_5                  53
#define TQ_KEY_6                  54
#define TQ_KEY_7                  55
#define TQ_KEY_8                  56
#define TQ_KEY_9                  57
#define TQ_KEY_SEMICOLON          59  /* ; */
#define TQ_KEY_EQUAL              61  /* = */
#define TQ_KEY_A                  65
#define TQ_KEY_B                  66
#define TQ_KEY_C                  67
#define TQ_KEY_D                  68
#define TQ_KEY_E                  69
#define TQ_KEY_F                  70
#define TQ_KEY_G                  71
#define TQ_KEY_H                  72
#define TQ_KEY_I                  73
#define TQ_KEY_J                  74
#define TQ_KEY_K                  75
#define TQ_KEY_L                  76
#define TQ_KEY_M                  77
#define TQ_KEY_N                  78
#define TQ_KEY_O                  79
#define TQ_KEY_P                  80
#define TQ_KEY_Q                  81
#define TQ_KEY_R                  82
#define TQ_KEY_S                  83
#define TQ_KEY_T                  84
#define TQ_KEY_U                  85
#define TQ_KEY_V                  86
#define TQ_KEY_W                  87
#define TQ_KEY_X                  88
#define TQ_KEY_Y                  89
#define TQ_KEY_Z                  90
#define TQ_KEY_LEFT_BRACKET       91  /* [ */
#define TQ_KEY_BACKSLASH          92  /* \ */
#define TQ_KEY_RIGHT_BRACKET      93  /* ] */
#define TQ_KEY_GRAVE_ACCENT       96  /* ` */
#define TQ_KEY_WORLD_1            161 /* non-US #1 */
#define TQ_KEY_WORLD_2            162 /* non-US #2 */
#define TQ_KEY_ESCAPE             256
#define TQ_KEY_ENTER              257
#define TQ_KEY_TAB                258
#define TQ_KEY_BACKSPACE          259
#define TQ_KEY_INSERT             260
#define TQ_KEY_DELETE             261
#define TQ_KEY_RIGHT              262
#define TQ_KEY_LEFT               263
#define TQ_KEY_DOWN               264
#define TQ_KEY_UP                 265
#define TQ_KEY_PAGE_UP            266
#define TQ_KEY_PAGE_DOWN          267
#define TQ_KEY_HOME               268
#define TQ_KEY_END                269
#define TQ_KEY_CAPS_LOCK          280
#define TQ_KEY_SCROLL_LOCK        281
#define TQ_KEY_NUM_LOCK           282
#define TQ_KEY_PRINT_SCREEN       283
#define TQ_KEY_PAUSE              284
#define TQ_KEY_F1                 290
#define TQ_KEY_F2                 291
#define TQ_KEY_F3                 292
#define TQ_KEY_F4                 293
#define TQ_KEY_F5                 294
#define TQ_KEY_F6                 295
#define TQ_KEY_F7                 296
#define TQ_KEY_F8                 297
#define TQ_KEY_F9                 298
#define TQ_KEY_F10                299
#define TQ_KEY_F11                300
#define TQ_KEY_F12                301
#define TQ_KEY_F13                302
#define TQ_KEY_F14                303
#define TQ_KEY_F15                304
#define TQ_KEY_F16                305
#define TQ_KEY_F17                306
#define TQ_KEY_F18                307
#define TQ_KEY_F19                308
#define TQ_KEY_F20                309
#define TQ_KEY_F21                310
#define TQ_KEY_F22                311
#define TQ_KEY_F23                312
#define TQ_KEY_F24                313
#define TQ_KEY_F25                314
#define TQ_KEY_KP_0               320
#define TQ_KEY_KP_1               321
#define TQ_KEY_KP_2               322
#define TQ_KEY_KP_3               323
#define TQ_KEY_KP_4               324
#define TQ_KEY_KP_5               325
#define TQ_KEY_KP_6               326
#define TQ_KEY_KP_7               327
#define TQ_KEY_KP_8               328
#define TQ_KEY_KP_9               329
#define TQ_KEY_KP_DECIMAL         330
#define TQ_KEY_KP_DIVIDE          331
#define TQ_KEY_KP_MULTIPLY        332
#define TQ_KEY_KP_SUBTRACT        333
#define TQ_KEY_KP_ADD             334
#define TQ_KEY_KP_ENTER           335
#define TQ_KEY_KP_EQUAL           336
#define TQ_KEY_LEFT_SHIFT         340
#define TQ_KEY_LEFT_CONTROL       341
#define TQ_KEY_LEFT_ALT           342
#define TQ_KEY_LEFT_SUPER         343
#define TQ_KEY_RIGHT_SHIFT        344
#define TQ_KEY_RIGHT_CONTROL      345
#define TQ_KEY_RIGHT_ALT          346
#define TQ_KEY_RIGHT_SUPER        347
#define TQ_KEY_MENU               348
#define TQ_KEY_LAST               TQ_KEY_MENU
#define TQ_BUTTON_1         0
#define TQ_BUTTON_2         1
#define TQ_BUTTON_3         2
#define TQ_BUTTON_4         3
#define TQ_BUTTON_5         4
#define TQ_BUTTON_6         5
#define TQ_BUTTON_7         6
#define TQ_BUTTON_8         7
#define TQ_BUTTON_LAST      TQ_BUTTON_8
#define TQ_BUTTON_LEFT      TQ_BUTTON_1
#define TQ_BUTTON_RIGHT     TQ_BUTTON_2
#define TQ_BUTTON_MIDDLE    TQ_BUTTON_3

// lower_snake_case
#define TQ_KEY_space              32
#define TQ_KEY_apostrophe         39  /* ' */
#define TQ_KEY_comma              44  /* , */
#define TQ_KEY_minus              45  /* - */
#define TQ_KEY_period             46  /* . */
#define TQ_KEY_slash              47  /* / */
#define TQ_KEY_semicolon          59  /* ; */
#define TQ_KEY_equal              61  /* = */
#define TQ_KEY_a                  65
#define TQ_KEY_b                  66
#define TQ_KEY_c                  67
#define TQ_KEY_d                  68
#define TQ_KEY_e                  69
#define TQ_KEY_f                  70
#define TQ_KEY_g                  71
#define TQ_KEY_h                  72
#define TQ_KEY_i                  73
#define TQ_KEY_j                  74
#define TQ_KEY_k                  75
#define TQ_KEY_l                  76
#define TQ_KEY_m                  77
#define TQ_KEY_n                  78
#define TQ_KEY_o                  79
#define TQ_KEY_p                  80
#define TQ_KEY_q                  81
#define TQ_KEY_r                  82
#define TQ_KEY_s                  83
#define TQ_KEY_t                  84
#define TQ_KEY_u                  85
#define TQ_KEY_v                  86
#define TQ_KEY_w                  87
#define TQ_KEY_x                  88
#define TQ_KEY_y                  89
#define TQ_KEY_z                  90
#define TQ_KEY_left_bracket       91  /* [ */
#define TQ_KEY_backslash          92  /* \ */
#define TQ_KEY_right_bracket      93  /* ] */
#define TQ_KEY_grave_accent       96  /* ` */
#define TQ_KEY_world_1            161 /* non-US #1 */
#define TQ_KEY_world_2            162 /* non-US #2 */
#define TQ_KEY_escape             256
#define TQ_KEY_enter              257
#define TQ_KEY_tab                258
#define TQ_KEY_backspace          259
#define TQ_KEY_insert             260
#define TQ_KEY_delete             261
#define TQ_KEY_right              262
#define TQ_KEY_left               263
#define TQ_KEY_down               264
#define TQ_KEY_up                 265
#define TQ_KEY_page_up            266
#define TQ_KEY_page_down          267
#define TQ_KEY_home               268
#define TQ_KEY_end                269
#define TQ_KEY_caps_lock          280
#define TQ_KEY_scroll_lock        281
#define TQ_KEY_num_lock           282
#define TQ_KEY_print_screen       283
#define TQ_KEY_pause              284
#define TQ_KEY_f1                 290
#define TQ_KEY_f2                 291
#define TQ_KEY_f3                 292
#define TQ_KEY_f4                 293
#define TQ_KEY_f5                 294
#define TQ_KEY_f6                 295
#define TQ_KEY_f7                 296
#define TQ_KEY_f8                 297
#define TQ_KEY_f9                 298
#define TQ_KEY_f10                299
#define TQ_KEY_f11                300
#define TQ_KEY_f12                301
#define TQ_KEY_f13                302
#define TQ_KEY_f14                303
#define TQ_KEY_f15                304
#define TQ_KEY_f16                305
#define TQ_KEY_f17                306
#define TQ_KEY_f18                307
#define TQ_KEY_f19                308
#define TQ_KEY_f20                309
#define TQ_KEY_f21                310
#define TQ_KEY_f22                311
#define TQ_KEY_f23                312
#define TQ_KEY_f24                313
#define TQ_KEY_f25                314
#define TQ_KEY_kp_0               320
#define TQ_KEY_kp_1               321
#define TQ_KEY_kp_2               322
#define TQ_KEY_kp_3               323
#define TQ_KEY_kp_4               324
#define TQ_KEY_kp_5               325
#define TQ_KEY_kp_6               326
#define TQ_KEY_kp_7               327
#define TQ_KEY_kp_8               328
#define TQ_KEY_kp_9               329
#define TQ_KEY_kp_decimal         330
#define TQ_KEY_kp_divide          331
#define TQ_KEY_kp_multiply        332
#define TQ_KEY_kp_subtract        333
#define TQ_KEY_kp_add             334
#define TQ_KEY_kp_enter           335
#define TQ_KEY_kp_equal           336
#define TQ_KEY_left_shift         340
#define TQ_KEY_left_control       341
#define TQ_KEY_left_alt           342
#define TQ_KEY_left_super         343
#define TQ_KEY_right_shift        344
#define TQ_KEY_right_control      345
#define TQ_KEY_right_alt          346
#define TQ_KEY_right_super        347
#define TQ_KEY_menu               348
#define TQ_BUTTON_left      TQ_BUTTON_1
#define TQ_BUTTON_right     TQ_BUTTON_2
#define TQ_BUTTON_middle    TQ_BUTTON_3

// PascalCase
#define TQ_KEY_Space              32
#define TQ_KEY_Apostrophe         39  /* ' */
#define TQ_KEY_Comma              44  /* , */
#define TQ_KEY_Minus              45  /* - */
#define TQ_KEY_Period             46  /* . */
#define TQ_KEY_Slash              47  /* / */
#define TQ_KEY_Semicolon          59  /* ; */
#define TQ_KEY_Equal              61  /* = */
#define TQ_KEY_LeftBracket       91  /* [ */
#define TQ_KEY_Backslash          92  /* \ */
#define TQ_KEY_RightBracket      93  /* ] */
#define TQ_KEY_GraveAccent       96  /* ` */
#define TQ_KEY_World1            161 /* non-US #1 */
#define TQ_KEY_World2            162 /* non-US #2 */
#define TQ_KEY_Escape             256
#define TQ_KEY_Enter              257
#define TQ_KEY_Tab                258
#define TQ_KEY_Backspace          259
#define TQ_KEY_Insert             260
#define TQ_KEY_Delete             261
#define TQ_KEY_Right              262
#define TQ_KEY_Left               263
#define TQ_KEY_Down               264
#define TQ_KEY_Up                 265
#define TQ_KEY_PageUp            266
#define TQ_KEY_PageDown          267
#define TQ_KEY_Home               268
#define TQ_KEY_End                269
#define TQ_KEY_CapsLock          280
#define TQ_KEY_ScrollLock        281
#define TQ_KEY_NumLock           282
#define TQ_KEY_PrintScreen       283
#define TQ_KEY_Pause              284
#define TQ_KEY_Kp0               320
#define TQ_KEY_Kp1               321
#define TQ_KEY_Kp2               322
#define TQ_KEY_Kp3               323
#define TQ_KEY_Kp4               324
#define TQ_KEY_Kp5               325
#define TQ_KEY_Kp6               326
#define TQ_KEY_Kp7               327
#define TQ_KEY_Kp8               328
#define TQ_KEY_Kp9               329
#define TQ_KEY_KpDecimal         330
#define TQ_KEY_KpDivide          331
#define TQ_KEY_KpMultiply        332
#define TQ_KEY_KpSubtract        333
#define TQ_KEY_KpAdd             334
#define TQ_KEY_KpEnter           335
#define TQ_KEY_KpEqual           336
#define TQ_KEY_LeftShift         340
#define TQ_KEY_LeftControl       341
#define TQ_KEY_LeftAlt           342
#define TQ_KEY_LeftSuper         343
#define TQ_KEY_RightShift        344
#define TQ_KEY_RightControl      345
#define TQ_KEY_RightAlt          346
#define TQ_KEY_RightSuper        347
#define TQ_KEY_Menu               348
#define TQ_BUTTON_Left      TQ_BUTTON_1
#define TQ_BUTTON_Right     TQ_BUTTON_2
#define TQ_BUTTON_Middle    TQ_BUTTON_3
