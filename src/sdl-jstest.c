// sdl-jstest - Joystick Test Program for SDL
// Copyright (C) 2007 Ingo Ruhnke <grumbel@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <assert.h>
#include <curses.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#  include <windows.h>
#endif

void print_bar(int pos, int len)
{
  addch('[');
  for(int i = 0; i < len; ++i)
  {
    if (i == pos)
      addch('#');
    else
      addch(' ');
  }
  addch(']');
}

int str2int(const char* str, int* val)
{
  char* endptr;

  errno = 0;
  long tmp = strtol(str, &endptr, 10);

  // error
  if (errno != 0) {
    return 0;
  }

  // garbage at the end
  if (*endptr != '\0') {
    return 0;
  }

  // out of range of int
  if (tmp < INT_MIN || tmp > INT_MAX) {
    return 0;
  }

  *val = (int)tmp;
  return 1;
}

void print_joystick_info(int joy_idx, SDL_Joystick* joy)
{
  printf("Joystick Name:     '%s'\n", SDL_JoystickName(joy_idx));
  printf("Joystick Number:   %2d\n", joy_idx);
  printf("Number of Axes:    %2d\n", SDL_JoystickNumAxes(joy));
  printf("Number of Buttons: %2d\n", SDL_JoystickNumButtons(joy));
  printf("Number of Hats:    %2d\n", SDL_JoystickNumHats(joy));
  printf("Number of Balls:   %2d\n", SDL_JoystickNumBalls(joy));
  printf("\n");
}

void print_help(const char* prg)
{
  printf("Usage: %s [OPTION]\n", prg);
  printf("List available joysticks or test a joystick.\n");
  printf("This programm uses SDL for doing its test instead of using the raw\n"
         "/dev/input/jsX interface\n");
  printf("\n");
  printf("Options:\n");
  printf("  --help             Print this help\n");
  printf("  --version          Print version number and exit\n");
  printf("  --list             Search for available joysticks and list their properties\n");
  printf("  --test  JOYNUM     Display a graphical representation of the current joystick state\n");
  printf("  --event JOYNUM     Display the events that are received from the joystick\n");
  printf("\n");
  printf("Examples:\n");
  printf("  %s --list\n", prg);
  printf("  %s --test 1\n", prg);
}

int main(int argc, char** argv)
{
#ifdef _WIN32
  FILE* fp;

  AllocConsole();
  freopen_s(&fp, "CONIN$", "r", stdin);
  freopen_s(&fp, "CONOUT$", "w", stdout);
  freopen_s(&fp, "CONOUT$", "w", stderr);
#endif

  if (argc == 1)
  {
    print_help(argv[0]);
    exit(1);
  }

  // FIXME: We don't need video, but without it SDL will fail to work in SDL_WaitEvent()
  if(SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
  {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  else
  {
    atexit(SDL_Quit);

    if (argc == 2 && (strcmp(argv[1], "--help") == 0 ||
                      strcmp(argv[1], "-h") == 0))
    {
      print_help(argv[0]);
    }
    if (argc == 2 && (strcmp(argv[1], "--version") == 0))
    {
      printf("sdl-jstest " SDL_JSTEST_VERSION "\n");
      exit(EXIT_SUCCESS);
    }
    else if (argc == 2 && (strcmp(argv[1], "--list") == 0 ||
                           (strcmp(argv[1], "-l") == 0)))
    {
      int num_joysticks = SDL_NumJoysticks();
      if (num_joysticks == 0)
      {
        printf("No joysticks were found\n");
      }
      else
      {
        printf("Found %d joystick(s)\n\n", num_joysticks);
        for(int joy_idx = 0; joy_idx < num_joysticks; ++joy_idx)
        {
          SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
          if (!joy)
          {
            fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
          }
          else
          {
            print_joystick_info(joy_idx, joy);
            SDL_JoystickClose(joy);
          }
        }
      }
    }
    else if (argc == 3 && (strcmp(argv[1], "--test") == 0 ||
                           strcmp(argv[1], "-t") == 0))
    {
      int joy_idx;
      if (!str2int(argv[2], &joy_idx))
      {
        fprintf(stderr, "Error: JOYSTICKNUM argument must be a number, but was '%s'\n", argv[2]);
        exit(1);
      }

      SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
      if (!joy)
      {
        fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
      }
      else
      {
        initscr();

        //cbreak();
        noecho();
        nodelay(stdscr, TRUE);
        //nonl();
        curs_set(0);

        int num_axes    = SDL_JoystickNumAxes(joy);
        if (num_axes < 0) {
          fprintf(stderr, "Unable to get SDL axes count: %s\n", SDL_GetError());
          exit(EXIT_FAILURE);
        }

        int num_buttons = SDL_JoystickNumButtons(joy);
        if (num_buttons < 0) {
          fprintf(stderr, "Unable to get SDL buttons count: %s\n", SDL_GetError());
          exit(EXIT_FAILURE);
        }

        int num_hats    = SDL_JoystickNumHats(joy);
        if (num_hats < 0) {
          fprintf(stderr, "Unable to get SDL hats count: %s\n", SDL_GetError());
          exit(EXIT_FAILURE);
        }

        int num_balls   = SDL_JoystickNumBalls(joy);
        if (num_balls < 0) {
          fprintf(stderr, "Unable to get SDL balls count: %s\n", SDL_GetError());
          exit(EXIT_FAILURE);
        }

        Sint16* axes    = calloc((size_t)num_axes,    sizeof(Sint16));
        Uint8*  buttons = calloc((size_t)num_buttons, sizeof(Uint8));
        Uint8*  hats    = calloc((size_t)num_hats,    sizeof(Uint8));
        Sint16* balls   = calloc((size_t)num_balls,   2*sizeof(Sint16));

        int quit = 0;
        SDL_Event event;
        bool something_new = TRUE;
        while(!quit)
        {
          SDL_Delay(10);

          while (SDL_PollEvent(&event)) {
            something_new = TRUE;
            switch(event.type)
            {
              case SDL_JOYAXISMOTION:
                assert(event.jaxis.axis < num_axes);
                axes[event.jaxis.axis] = event.jaxis.value;
                break;

              case SDL_JOYBUTTONDOWN:
              case SDL_JOYBUTTONUP:
                assert(event.jbutton.button < num_buttons);
                buttons[event.jbutton.button] = event.jbutton.state;
                break;

              case SDL_JOYHATMOTION:
                assert(event.jhat.hat < num_hats);
                hats[event.jhat.hat] = event.jhat.value;
                break;

              case SDL_JOYBALLMOTION:
                assert(event.jball.ball < num_balls);
                balls[2*event.jball.ball + 0] = event.jball.xrel;
                balls[2*event.jball.ball + 1] = event.jball.yrel;
                break;

              case SDL_QUIT:
                quit = 1;
                printf("Recieved interrupt, exiting\n");
                break;

              default:
                fprintf(stderr, "Error: Unhandled event type: %d\n", event.type);
                break;
            }
          }

          if (something_new)
          {
            //clear();
            move(0,0);

            printw("Joystick Name:   '%s'\n", SDL_JoystickName(joy_idx));
            printw("Joystick Number: %d\n", joy_idx);
            printw("\n");

            printw("Axes %2d:\n", num_axes);
            for(int i = 0; i < num_axes; ++i)
            {
              int len = COLS - 20;
              printw("  %2d: %6d  ", i, axes[i]);
              print_bar((axes[i] + 32767) * (len-1) / 65534, len);
              addch('\n');
            }
            printw("\n");

            printw("Buttons %2d:\n", num_buttons);
            for(int i = 0; i < num_buttons; ++i)
            {
              printw("  %2d: %d  %s\n", i, buttons[i], buttons[i] ? "[#]":"[ ]");
            }
            printw("\n");

            printw("Hats %2d:\n", num_hats);
            for(int i = 0; i < num_hats; ++i)
            {
              printw("  %2d: value: %d\n", i, hats[i]);
              printw("  +-----+  up:    %c\n"
                     "  |%c %c %c|  down:  %c\n"
                     "  |%c %c %c|  left:  %c\n"
                     "  |%c %c %c|  right: %c\n"
                     "  +-----+\n",

                     (hats[i] & SDL_HAT_UP)?'1':'0',

                     ((hats[i] & SDL_HAT_UP) && (hats[i] & SDL_HAT_LEFT)) ? 'O' : ' ',
                     ((hats[i] & SDL_HAT_UP) && !(hats[i] & (SDL_HAT_LEFT | SDL_HAT_RIGHT))) ? 'O' : ' ',
                     ((hats[i] & SDL_HAT_UP) && (hats[i] & SDL_HAT_RIGHT)) ? 'O' : ' ',

                     (hats[i] & SDL_HAT_DOWN)?'1':'0',

                     (!(hats[i] & (SDL_HAT_UP | SDL_HAT_DOWN)) && (hats[i] & SDL_HAT_LEFT)) ? 'O' : ' ',
                     (!(hats[i] & (SDL_HAT_UP | SDL_HAT_DOWN)) && !(hats[i] & (SDL_HAT_LEFT | SDL_HAT_RIGHT))) ? 'O' : ' ',
                     (!(hats[i] & (SDL_HAT_UP | SDL_HAT_DOWN)) && (hats[i] & SDL_HAT_RIGHT)) ? 'O' : ' ',

                     (hats[i] & SDL_HAT_LEFT)?'1':'0',

                     ((hats[i] & SDL_HAT_DOWN) && (hats[i] & SDL_HAT_LEFT)) ? 'O' : ' ',
                     ((hats[i] & SDL_HAT_DOWN) && !(hats[i] & (SDL_HAT_LEFT | SDL_HAT_RIGHT))) ? 'O' : ' ',
                     ((hats[i] & SDL_HAT_DOWN) && (hats[i] & SDL_HAT_RIGHT)) ? 'O' : ' ',

                     (hats[i] & SDL_HAT_RIGHT)?'1':'0');
            }
            printw("\n");

            printw("Balls %2d: ", num_balls);
            for(int i = 0; i < num_balls; ++i)
            {
              printw("  %2d: %6d %6d\n", i, balls[2*i+0], balls[2*i+1]);
            }
            printw("\n");
            printw("\n");
            printw("Press Ctrl-c to exit\n");

            refresh();

            something_new = FALSE;
          }

          if ( getch() == 3 ) // Ctrl-c
          {
            quit = 1;
          }
        } // while

        free(balls);
        free(hats);
        free(buttons);
        free(axes);

        endwin();
      }
    }
    else if (argc == 3 && (strcmp(argv[1], "--event") == 0 ||
                           strcmp(argv[1], "-e") == 0))
    {
      int joy_idx;
      if (!str2int(argv[2], &joy_idx))
      {
        fprintf(stderr, "Error: JOYSTICKNUM argument must be a number, but was '%s'\n", argv[2]);
        exit(1);
      }

      SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
      if (!joy)
      {
        fprintf(stderr, "Unable to open joystick %d\n", joy_idx);
      }
      else
      {
        print_joystick_info(joy_idx, joy);

        printf("Entering joystick test loop, press Ctrl-c to exit\n");
        int quit = 0;
        SDL_Event event;

        while(!quit && SDL_WaitEvent(&event))
        {
          switch(event.type)
          {
            case SDL_JOYAXISMOTION:
              printf("SDL_JOYAXISMOTION: joystick: %d axis: %d value: %d\n",
                     event.jaxis.which, event.jaxis.axis, event.jaxis.value);
              break;

            case SDL_JOYBUTTONDOWN:
              printf("SDL_JOYBUTTONDOWN: joystick: %d button: %d state: %d\n",
                     event.jbutton.which, event.jbutton.button, event.jbutton.state);
              break;
            case SDL_JOYBUTTONUP:
              printf("SDL_JOYBUTTONUP: joystick: %d button: %d state: %d\n",
                     event.jbutton.which, event.jbutton.button, event.jbutton.state);
              break;

            case SDL_JOYHATMOTION:
              printf("SDL_JOYHATMOTION: joystick: %d hat: %d value: %d\n",
                     event.jhat.which, event.jhat.hat, event.jhat.value);
              break;

            case SDL_JOYBALLMOTION:
              printf("SDL_JOYBALLMOTION: joystick: %d ball: %d x: %d y: %d\n",
                     event.jball.which, event.jball.ball, event.jball.xrel, event.jball.yrel);
              break;

            case SDL_QUIT:
              quit = 1;
              printf("Recieved interrupt, exiting\n");
              break;

            default:
              fprintf(stderr, "Error: Unhandled event type: %d\n", event.type);
              break;
          }
        }
        SDL_JoystickClose(joy);

      }
      fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    }
    else
    {
      fprintf(stderr, "%s: unknown arguments\n", argv[0]);
      fprintf(stderr, "Try '%s --help' for more informations\n", argv[0]);
    }
  }

  return EXIT_SUCCESS;
}

/* EOF */
