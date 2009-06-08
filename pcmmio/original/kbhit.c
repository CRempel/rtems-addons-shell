/* $Header$
*
*
*   Filename : $RCSfile$
*
*   $Id$
*
*   $Log$
*   Revision 1.1  2005/04/19 20:39:11  steve
*   Initial revision
*
*
*
*/


/* These functions are from the Wrox Press Book "Beginning Linux
   Programming - Second Edition" by Richard Stones and Neil Matthew.
   (C) 1996 & 1999 and are included here in compliance with the GNU
   Public license.

*/

#include <stdio.h>
#include <termios.h>
#include <term.h>
#include <curses.h>
#include <unistd.h>

static struct termios initial_settings, new_settings;
static int peek_character = -1;

void init_keyboard(void);
void close_keyboard(void);
int kbhit(void);
int readch(void);

void init_keyboard(void)
{
    tcgetattr(0,&initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_lflag &= ~ISIG;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard(void)
{
    tcsetattr(0, TCSANOW, &initial_settings);
}


int kbhit(void)
{
char ch;
int nread;

    if(peek_character != -1)
        return 1;

    new_settings.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0, &ch, 1);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);

    if(nread == 1)
    {
        peek_character = ch;
        return 1;
    }

    return 0;
}

int readch(void)
{
char ch;

    if(peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0,&ch,1);
    return ch;
}

