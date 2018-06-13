#ifdef __WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <unistd.h>
#endif
#include <la/boolean.h>

boolean_t system_isAdmin() {
    boolean_t isAdmin = boolean_false;

#ifdef __WIN32
    char *env;
    char *filename;
    FILE *file;

    /* get filename */
    env = getenv("SYSTEMROOT");
    if (!env) return 0;

    filename = (char *)malloc(strlen(env) + 1 + 9 + 1);
    strcpy(filename, env);
    strcat(filename, "\\");
    strcat(filename, "la-C.tmp");

    file = fopen(filename, "w");
    if (file) {
        isAdmin = boolean_true;
        fclose(file);
        unlink(filename);
    } else {
        isAdmin = boolean_false;
    }

    free(filename);
#else
    if (geteuid() == 0) isAdmin = boolean_true;
    else isAdmin = boolean_false;
#endif

    return isAdmin;
}

