#ifndef D_LOG_H
#  define D_LOG_H

#  include <errno.h>

void vprint(char *pszmsg, ...);
void vperror(char *pszmsg, ...);
void verror(char *pszmsg);

#endif                          /* D_LOG_H */

 /* EOF */
