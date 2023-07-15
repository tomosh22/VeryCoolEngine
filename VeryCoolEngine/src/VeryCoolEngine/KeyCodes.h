#pragma once

#ifdef VCE_WINDOWS
//stolen from glfw
#define VCE_KEY_SPACE              32
#define VCE_KEY_APOSTROPHE         39  /* ' */
#define VCE_KEY_COMMA              44  /* , */
#define VCE_KEY_MINUS              45  /* - */
#define VCE_KEY_PERIOD             46  /* . */
#define VCE_KEY_SLASH              47  /* / */
#define VCE_KEY_0                  48
#define VCE_KEY_1                  49
#define VCE_KEY_2                  50
#define VCE_KEY_3                  51
#define VCE_KEY_4                  52
#define VCE_KEY_5                  53
#define VCE_KEY_6                  54
#define VCE_KEY_7                  55
#define VCE_KEY_8                  56
#define VCE_KEY_9                  57
#define VCE_KEY_SEMICOLON          59  /* ; */
#define VCE_KEY_EQUAL              61  /* = */
#define VCE_KEY_A                  65
#define VCE_KEY_B                  66
#define VCE_KEY_C                  67
#define VCE_KEY_D                  68
#define VCE_KEY_E                  69
#define VCE_KEY_F                  70
#define VCE_KEY_G                  71
#define VCE_KEY_H                  72
#define VCE_KEY_I                  73
#define VCE_KEY_J                  74
#define VCE_KEY_K                  75
#define VCE_KEY_L                  76
#define VCE_KEY_M                  77
#define VCE_KEY_N                  78
#define VCE_KEY_O                  79
#define VCE_KEY_P                  80
#define VCE_KEY_Q                  81
#define VCE_KEY_R                  82
#define VCE_KEY_S                  83
#define VCE_KEY_T                  84
#define VCE_KEY_U                  85
#define VCE_KEY_V                  86
#define VCE_KEY_W                  87
#define VCE_KEY_X                  88
#define VCE_KEY_Y                  89
#define VCE_KEY_Z                  90
#define VCE_KEY_LEFT_BRACKET       91  /* [ */
#define VCE_KEY_BACKSLASH          92  /* \ */
#define VCE_KEY_RIGHT_BRACKET      93  /* ] */
#define VCE_KEY_GRAVE_ACCENT       96  /* ` */
#define VCE_KEY_WORLD_1            161 /* non-US #1 */
#define VCE_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define VCE_KEY_ESCAPE             256
#define VCE_KEY_ENTER              257
#define VCE_KEY_TAB                258
#define VCE_KEY_BACKSPACE          259
#define VCE_KEY_INSERT             260
#define VCE_KEY_DELETE             261
#define VCE_KEY_RIGHT              262
#define VCE_KEY_LEFT               263
#define VCE_KEY_DOWN               264
#define VCE_KEY_UP                 265
#define VCE_KEY_PAGE_UP            266
#define VCE_KEY_PAGE_DOWN          267
#define VCE_KEY_HOME               268
#define VCE_KEY_END                269
#define VCE_KEY_CAPS_LOCK          280
#define VCE_KEY_SCROLL_LOCK        281
#define VCE_KEY_NUM_LOCK           282
#define VCE_KEY_PRINT_SCREEN       283
#define VCE_KEY_PAUSE              284
#define VCE_KEY_F1                 290
#define VCE_KEY_F2                 291
#define VCE_KEY_F3                 292
#define VCE_KEY_F4                 293
#define VCE_KEY_F5                 294
#define VCE_KEY_F6                 295
#define VCE_KEY_F7                 296
#define VCE_KEY_F8                 297
#define VCE_KEY_F9                 298
#define VCE_KEY_F10                299
#define VCE_KEY_F11                300
#define VCE_KEY_F12                301
#define VCE_KEY_F13                302
#define VCE_KEY_F14                303
#define VCE_KEY_F15                304
#define VCE_KEY_F16                305
#define VCE_KEY_F17                306
#define VCE_KEY_F18                307
#define VCE_KEY_F19                308
#define VCE_KEY_F20                309
#define VCE_KEY_F21                310
#define VCE_KEY_F22                311
#define VCE_KEY_F23                312
#define VCE_KEY_F24                313
#define VCE_KEY_F25                314
#define VCE_KEY_KP_0               320
#define VCE_KEY_KP_1               321
#define VCE_KEY_KP_2               322
#define VCE_KEY_KP_3               323
#define VCE_KEY_KP_4               324
#define VCE_KEY_KP_5               325
#define VCE_KEY_KP_6               326
#define VCE_KEY_KP_7               327
#define VCE_KEY_KP_8               328
#define VCE_KEY_KP_9               329
#define VCE_KEY_KP_DECIMAL         330
#define VCE_KEY_KP_DIVIDE          331
#define VCE_KEY_KP_MULTIPLY        332
#define VCE_KEY_KP_SUBTRACT        333
#define VCE_KEY_KP_ADD             334
#define VCE_KEY_KP_ENTER           335
#define VCE_KEY_KP_EQUAL           336
#define VCE_KEY_LEFT_SHIFT         340
#define VCE_KEY_LEFT_CONTROL       341
#define VCE_KEY_LEFT_ALT           342
#define VCE_KEY_LEFT_SUPER         343
#define VCE_KEY_RIGHT_SHIFT        344
#define VCE_KEY_RIGHT_CONTROL      345
#define VCE_KEY_RIGHT_ALT          346
#define VCE_KEY_RIGHT_SUPER        347
#define VCE_KEY_MENU               348

#define VCE_MOUSE_BUTTON_1         0
#define VCE_MOUSE_BUTTON_2         1
#define VCE_MOUSE_BUTTON_3         2
#define VCE_MOUSE_BUTTON_4         3
#define VCE_MOUSE_BUTTON_5         4
#define VCE_MOUSE_BUTTON_6         5
#define VCE_MOUSE_BUTTON_7         6
#define VCE_MOUSE_BUTTON_8         7
#define VCE_MOUSE_BUTTON_LAST      VCE_MOUSE_BUTTON_8
#define VCE_MOUSE_BUTTON_LEFT      VCE_MOUSE_BUTTON_1
#define VCE_MOUSE_BUTTON_RIGHT     VCE_MOUSE_BUTTON_2
#define VCE_MOUSE_BUTTON_MIDDLE    VCE_MOUSE_BUTTON_3
#endif //ifdef VCE_WINDOWS