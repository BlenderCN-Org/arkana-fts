#ifndef D_SETUP_H
#  define D_SETUP_H

#  include "main.h"

void init(bool bAdminMode);
bool connect_to_server(void);

void inject_time_pulse(double &last_time_pulse);
void inject_input(bool & must_quit);

void handle_mouse_down(Uint8 button);
void handle_mouse_up(Uint8 button);

void render_gui(void);

#endif                          /* D_SETUP_H */
