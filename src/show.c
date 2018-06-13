#include <stdlib.h>
#include <string.h>
#include <la/console.h>
#include <la/string.h>
#include "main.h"
#include "show.h"

void showCopyright() {
    printf("%s v.%s\n", PRG_NAME, PRG_VERSION);
    printf("(c) %s by %s\n", PRG_DATE, PRG_AUTHOR);
    puts("");
}

void showHelp(const char *name) {
    printf("CALL:\t\t%s [OPTION]\n", name);
    puts("");
    puts("Central administrativ tool to manage a BI-System");
    puts("");
    puts("Arguments:");
    puts("\tconfig\t\topen configuration of BIAdmin");
    puts("\tversion\t\tshow version and copyright");
    puts("\thelp\t\tshow this help");
    puts("\tcheck\t\tchecks the system very quickly");
    puts("\tstatus\t\tgets the status via AdminServer");
    puts("\tstart\t\tstart the system");
    puts("\tstop\t\tstop the system");
    puts("\trestart\t\trestart the system");
    puts("\tbackup\t\tdrives the system into backup-mode and creates a ZIP-file which includes all the installation");
}

void showTitle(const char *name) {
    char *title;
    int width;

    console_clear();

    if (name) {
        title = string_toUpper(name);
        console_setCursor(0, 0);
        console_printf(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_LIGHTBLUE, " %s ", title);
        free(title);
    }

    width = console_getWidth();
    width /= 2;
    width -= strlen(PRG_NAME) / 2;
    if (width < 0) width = 0;
    console_setCursor(width, 0);
    console_printf(CONSOLE_COLOR_LIGHTBLUE, CONSOLE_COLOR_BLACK, "%s v.%s\n", PRG_NAME, PRG_VERSION);
    console_puts(CONSOLE_COLOR_DEFAULT, CONSOLE_COLOR_DEFAULT, "");
}
