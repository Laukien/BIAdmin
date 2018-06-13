#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <la/boolean.h>
#include <la/console.h>
#include <la/message.h>
#include "command.h"
#include "config.h"
#include "main.h"
#include "show.h"

static void pause(void) {
    puts("\nPress any key to continue...");
    console_getKey();
}

int main(int argc, char *argv[]) {
    int ret = EXIT_SUCCESS;
    char *arg;
    char key;

    /* load data from configuration-file */
    if (isConfig() == boolean_false || initConfig() == boolean_false) {
        if (argc == 1 || strcmp(argv[1], "config") != 0) {
            message_error("unable to read configuration (%s); run \"%s config\"", getConfigError(), argv[0]);
            ret = EXIT_FAILURE;
            goto quit;
        }
    }

    /* check arguments */
    if (argc == 1) {
        /* run main menu */
        while (1) {
            showTitle(NULL);
            console_puts(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_GREEN, "   *** Command ***   ");
            console_puts(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, "\t1. Check");
            console_puts(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, "\t2. Status");
            console_puts(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, "\t3. Start");
            console_puts(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, "\t4. Stop");
            console_puts(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, "\t5. Restart");
            console_puts(CONSOLE_COLOR_GREEN, CONSOLE_COLOR_BLACK, "\t6. Backup");
            console_puts(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT, "");
            console_puts(CONSOLE_COLOR_RED, CONSOLE_COLOR_BLACK, "\t0. Quit");
            console_puts(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT, "");
#ifdef __WIN32
            console_printf(CONSOLE_COLOR_LIGHGREY, CONSOLE_COLOR_DEFAULT, "Select a command: ");
#else
            console_printf(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT, "Select a command: ");
#endif
            fflush(stdout);
            key = console_getKey();

            switch (key) {
                case '0':
                case 'q':
                    showTitle("quit");
                    goto quit;
                case '1':
                case 'c':
                    showTitle("check");
                    commandCheck() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
                    pause();
                    break;
                case '2':
                case 's':
                    showTitle("status");
                    commandStatus() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
                    pause();
                    break;
                case '3':
                    showTitle("start");
                    commandStart() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
                    pause();
                    break;
                case '4':
                    showTitle("stop");
                    commandStop() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
                    pause();
                    break;
                case '5':
                case 'r':
                    showTitle("restart");
                    commandStop() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
                    commandStart() ? ret = ret | EXIT_SUCCESS : ret | EXIT_FAILURE;
                    pause();
                    break;
                case '6':
                case 'b':
                    showTitle("backup");
                    commandBackup() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
                    pause();
                    break;
                default:
                    break;
            }
        }
    } else if (argc == 2) {
        arg = argv[1];
        if (strcmp(arg, "config") == 0) {
            initConfig();
            editConfig();
            ret = EXIT_FAILURE;
        } else if (strcmp(arg, "version") == 0) {
            showCopyright();
            ret = EXIT_SUCCESS;
        } else if (strcmp(arg, "help") == 0) {
            showHelp(argv[0]);
            ret = EXIT_SUCCESS;
        } else if (strcmp(arg, "check") == 0) {
            commandCheck() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
        } else if (strcmp(arg, "status") == 0) {
            commandStatus() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
        } else if (strcmp(arg, "start") == 0) {
            commandStart() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
        } else if (strcmp(arg, "stop") == 0) {
            commandStop() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
        } else if (strcmp(arg, "restart") == 0) {
            commandStop() ? ret = EXIT_SUCCESS : EXIT_FAILURE;
            commandStart() ? ret = ret | EXIT_SUCCESS : ret | EXIT_FAILURE;
        } else if (strcmp(arg, "backup") == 0) {
            ret = commandBackup();
        } else {
            message_error("invalid argument");
            showHelp(argv[0]);
            ret = EXIT_FAILURE;
        }
    } else {
        message_error("invalid arguments");
        showHelp(argv[0]);
        ret = EXIT_FAILURE;
    }

quit:

    return ret;
}

