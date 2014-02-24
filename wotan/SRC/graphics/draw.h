#ifndef DRAW_H
#define DRAW_H

#include "wotan_init.h"
#include "graphics.h"

/**** Defines ****/
#define WINDOW_NAME "Wotan"
#define WINDOW_WIDTH 1000.0
#define UPPER_LEFT_X 0.0
#define UPPER_LEFT_Y WINDOW_WIDTH
#define LOWER_RIGHT_X WINDOW_WIDTH
#define LOWER_RIGHT_Y 0.0


/**** Function Definitions ****/
/* initializes and starts easygl graphics */
void start_easygl_graphics(INP User_Options user_opts);
/* the main drawscreen function which we will pass to event_loop */
void drawscreen(void);
/* the function which determines what happens if a user clicks a button somewhere in the drawing area */
void act_on_button_press(INP float x, INP float y, INP t_event_buttonPressed button_info);

/* sets the filesoce flood results pointer */
void set_flood_results_ptr( INP t_flood_results *flood_results);



#endif
