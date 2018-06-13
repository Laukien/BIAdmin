#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <la/config.h>
#include <la/directory.h>
#include <la/directorylist.h>
#include <la/file.h>
#include <la/message.h>
#include <la/stringlist.h>
#include <la/stringmap.h>
#include "config.h"

#define BUFFER 2048 + 1
#define CONFIG ".biadmin"

#ifdef __WIN32
#define MW_HOME_1 "D:\\oracle\\product\\fmw"
#define MW_HOME_2 "D:\\app\\obiee"
#define MW_HOME_3 "D:\\oracle\\Middleware"
#define MW_HOME_4 "D:\\app\\oracle\\Middleware"
#define CL_HOME_1 "D:\\oracle\\product\\client"
#define CL_HOME_2 "D:\\app\\client"
#define CL_HOME_3 "D:\\oracle\\Client"
#define CL_HOME_4 "D:\\app\\oracle\\Client"
#else
#define MW_HOME_1 "/u01/oracle/products/fmw"
#define MW_HOME_2 "/u01/app/obiee"
#define MW_HOME_3 "/u01/oracle/Middleware"
#define MW_HOME_4 "/u01/app/oracle/Middleware"
#define CL_HOME_1 "/u01/oracle/products/client"
#define CL_HOME_2 "/u01/app/client"
#define CL_HOME_3 "/u01/oracle/Client"
#define CL_HOME_4 "/u01/app/oracle/Client"
#endif

#define MW_DOMAIN "bidomain"


/* internal buffer */
static boolean_t config_run = boolean_false;
static char config_filename[BUFFER] = "";
static char config_error[BUFFER] = "";

/* configuration object */
static char system_type[BUFFER];
static char system_username[BUFFER];
static char client_home[BUFFER];
static char database_username[BUFFER];
static char database_password[BUFFER];
static char database_service[BUFFER];
static char middleware_home[BUFFER];
static char middleware_domain[BUFFER];
static char middleware_console[BUFFER];
static char middleware_analytics[BUFFER];
static char middleware_username[BUFFER];
static char middleware_password[BUFFER];
static char middleware_secure[BUFFER];
static char repository_home[BUFFER];
static char repository_password[BUFFER];

static char *guessClientHome() {
    char *base = NULL;
    char *home = NULL;
    stringlist_t *dl = NULL;
    char *test = NULL;

    /* check if ther exists one of the given bi-base-directories */
    if (directory_exists(CL_HOME_1)) base = strdup(CL_HOME_1);
    else if (directory_exists(CL_HOME_2)) base = strdup(CL_HOME_2);
    else if (directory_exists(CL_HOME_3)) base = strdup(CL_HOME_3);
    else if (directory_exists(CL_HOME_4)) base = strdup(CL_HOME_4);
    else goto quit;

    /* look for ONE sub-directory */
    dl = directory_list(base, boolean_false);
    if (dl == NULL) goto quit;
    if (stringlist_size(dl) == 1) {
        home = stringlist_get(dl, 0);
    } else {
        home = strdup(base);
    }

    /* look for an identifying sub-directory */
    test = (char *)malloc(strlen(home) + 1 + 7 + 1);
    sprintf(test, "%s%c%s", home, DIRECTORY_SEPARATOR_CHAR, "network");
    if (!directory_exists(test)) {
        free(home);
        home = NULL;
    }
    
quit:
    if (test) free(test);
    if (dl) stringlist_free(dl);
    if (base) free(base);

    return home;
}

static char *guessMiddlewareHome() {
    char *base = NULL;
    char *home = NULL;
    stringlist_t *dl = NULL;
    char *test = NULL;

    /* check if ther exists one of the given bi-base-directories */
    if (directory_exists(MW_HOME_1)) base = strdup(MW_HOME_1);
    else if (directory_exists(MW_HOME_2)) base = strdup(MW_HOME_2);
    else if (directory_exists(MW_HOME_3)) base = strdup(MW_HOME_3);
    else if (directory_exists(MW_HOME_4)) base = strdup(MW_HOME_4);
    else goto quit;

    /* look for ONE sub-directory */
    dl = directory_list(base, boolean_false);
    if (dl == NULL) goto quit;
    if (stringlist_size(dl) == 1) {
        home = stringlist_get(dl, 0);
    } else {
        home = strdup(base);
    }

    /* look for an identifying sub-directory */
    test = (char *)malloc(strlen(home) + 1 + 2 + 1);
    sprintf(test, "%s%c%s", home, DIRECTORY_SEPARATOR_CHAR, "bi");
    if (!directory_exists(test)) {
        free(home);
        home = NULL;
    }
    
quit:
    if (test) free(test);
    if (dl) stringlist_free(dl);
    if (base) free(base);

    return home;
}

static char *guessMiddlewareDomain() {
    char *home = NULL;
    char *base = NULL;
    char *domain = NULL;
    stringlist_t *dl = NULL;
    char *tmp;

    home = guessMiddlewareHome();
    if (!home) goto quit;

    /* MW_HOME/user_projects/domains */
    base = (char *)malloc(strlen(home) + 1 + 13 + 1 + 7 + 1);
    sprintf(base, "%s%c%s%c%s", home, DIRECTORY_SEPARATOR_CHAR, "user_projects", DIRECTORY_SEPARATOR_CHAR, "domains");
    if (!directory_exists(base)) goto quit;

    /* look for ONE sub-directory */
    dl = directory_list(base, boolean_false);
    if (dl == NULL) goto quit;
    if (stringlist_size(dl) == 1) {
        tmp = stringlist_get(dl, 0);
        domain = strdup(tmp + strlen(base) + 1);
        free(tmp);
    } else {
        /* check if bi-domain exists */
        char *sub;

        sub = (char *)malloc(strlen(base) + 1 + strlen(MW_DOMAIN) + 1);
        sprintf(sub, "%s%c%s", base, DIRECTORY_SEPARATOR_CHAR, MW_DOMAIN);
        if (directory_exists(sub)) {
            domain = strdup(MW_DOMAIN);
            free(sub);
            goto quit;
        }

        domain = NULL;
    }

quit:
    if (dl) stringlist_free(dl);
    if (base) free(base);
    if (home) free(home);

    return domain;
}

char *getConfigFilename() {
    char *home;

#ifdef __WIN32
    home = getenv("USERPROFILE");
#else
    home = getenv("HOME");
#endif

    /* break if home not exists */
    if (!home || file_exists(home) || strlen(home) > BUFFER - 16) {
        return NULL;
    }

    strcpy(config_filename, home);
    strcat(config_filename, DIRECTORY_SEPARATOR_STRING);
    strcat(config_filename, CONFIG);

    return config_filename;
}

static char *guessSystemUser() {
#ifdef __WIN32
    return getenv("USERNAME");
#else
    if (directory_exists("/home/oracle")) {
        return strdup("oracle");
    } else {
        return NULL;
    }
#endif
}

static boolean_t createFile(const char *filename) {
    FILE *file;
    char *su; /* system.user */
    char *ch; /* client.home */
    char *mh; /* middleware.home */
    char *md; /*middleware.domain */

    /* write ~/.biadmin */
    file = fopen(filename, "w");
    if (!file) return boolean_false;

    /* guess directories */
    su = guessSystemUser();
    ch = guessClientHome();
    mh = guessMiddlewareHome();
    md = guessMiddlewareDomain();

    /* convert NULL to empty string */
    if (!su) su = strdup("");
    if (!ch) ch = strdup("");
    if (!mh) mh = strdup("");
    if (!md) md = strdup("");

    /* system */
    fprintf(file, "[system]\n");
#ifdef __WIN32
    fprintf(file, "system=windows\n");
#else
    fprintf(file, "system=linux\n");
#endif
    fprintf(file, "username=%s\n", su);
    fprintf(file, "\n");

    /* client */
    fprintf(file, "[client]\n");
    fprintf(file, "home=%s\n", ch);
    fprintf(file, "\n");

    /* database */
    fprintf(file, "[database]\n");
    fprintf(file, "username=\n");
    fprintf(file, "password=\n");
    fprintf(file, "service=\n");
    fprintf(file, "\n");

    /* middleware */
    fprintf(file, "[middleware]\n");
    fprintf(file, "home=%s\n", mh);
    fprintf(file, "domain=%s\n", md);
    fprintf(file, "console=9500\n");
    fprintf(file, "analytics=9502\n");
    fprintf(file, "username=weblogic\n");
    fprintf(file, "password=\n");
    fprintf(file, "secure=false\n");
    fprintf(file, "\n");

    /* repository */
    fprintf(file, "[repository]\n");
    fprintf(file, "home=\n");
    fprintf(file, "password=\n");
    fprintf(file, "\n");

    free(md);
    free(mh);
    free(ch);
    free(su);
    fclose(file);

    return boolean_true;
}

static boolean_t setParameter(config_t *cfg, char *parameter, const char *section, const char *key, boolean_t required) {
    char *tmp;

    parameter[0] = '\0';
    tmp = config_get(cfg, section, key);
    if (tmp && strlen(tmp) < BUFFER) {
        strcpy(parameter, tmp);
        free(tmp);
    } else if (required) {
        message_error("parameter invalid [%s] -> %s", section, key);
        return boolean_false;
    }

    return boolean_true;
}

boolean_t initConfig() {
    boolean_t ret;
    config_t *cfg = NULL;
    char *filename;

    /* check if config has been ran */
    if (config_run == boolean_true) return boolean_true;

    filename = getConfigFilename();
    if (!file_exists(filename)) {
        ret = createFile(filename);
        if (ret == boolean_false) goto quit;
    }

    /* load configuration */
    cfg = config_init();
    config_load(cfg, filename);

    ret = boolean_false;
    if (!setParameter(cfg, system_type, "system", "type", boolean_false)) goto quit;
    if (!setParameter(cfg, system_username, "system", "username", boolean_true)) goto quit;
    if (!setParameter(cfg, client_home, "client", "home", boolean_false)) goto quit;
    if (!setParameter(cfg, database_username, "database", "username", boolean_false)) goto quit;
    if (!setParameter(cfg, database_password, "database", "password", boolean_false)) goto quit;
    if (!setParameter(cfg, database_service, "database", "service", boolean_false)) goto quit;
    if (!setParameter(cfg, middleware_home, "middleware", "home", boolean_true)) goto quit;
    if (!setParameter(cfg, middleware_domain, "middleware", "domain", boolean_true)) goto quit;
    if (!setParameter(cfg, middleware_console, "middleware", "console", boolean_true)) goto quit;
    if (!setParameter(cfg, middleware_analytics, "middleware", "analytics", boolean_false)) goto quit;
    if (!setParameter(cfg, middleware_username, "middleware", "username", boolean_false)) goto quit;
    if (!setParameter(cfg, middleware_password, "middleware", "password", boolean_false)) goto quit;
    if (!setParameter(cfg, middleware_secure, "middleware", "secure", boolean_false)) goto quit;
    if (!setParameter(cfg, repository_home, "repository", "home", boolean_false)) goto quit;
    if (!setParameter(cfg, repository_password, "repository", "password", boolean_false)) goto quit;
    ret = boolean_true;

quit:
    if (cfg) config_free(cfg);

    if (errno) ret = boolean_false;

    /* mark config as ran */
    config_run = boolean_true;

    return ret;
}

boolean_t isConfig() {
    char *filename;

    filename = getConfigFilename();
    if (!filename || strlen(filename) < 3) {
        strcpy(config_error, "invalid filename");
        return boolean_false;
    }

    if (file_exists(getConfigFilename())) return boolean_true;

    strcpy(config_error, "file not found");

    return boolean_false;
}

char *getConfigError() {
    return config_error;
}

boolean_t editConfig() {
    boolean_t ret;
    char *filename = NULL;
    char cmd[BUFFER];

    filename = getConfigFilename();
    if (!filename || strlen(filename) > (BUFFER - 10) || !file_exists(filename)) {
        ret = boolean_false;
        goto quit;
    }

#ifdef __WIN32
    strcpy(cmd, "notepad");
#else
    strcpy(cmd, "vi");
#endif
    strcat(cmd, " ");
    strcat(cmd, filename);

    ret = (system(cmd) ? boolean_false : boolean_true);

quit:
    return ret;
}

char *getSystemUsername() {
    return system_username;
}

char *getMiddlewareHome() {
    return middleware_home;
}

char *getMiddlewareDomain() {
    return middleware_domain;
}

char *getMiddlewareUsername() {
    return middleware_username;
}

char *getMiddlewarePassword() {
    return middleware_password;
}

int getMiddlewareConsole() {
    return atoi(middleware_console);
}

int getMiddlewareAnalytics() {
    int port;

    port = atoi(middleware_analytics);
    if (port == -1) {
        port = atoi(middleware_console);
        port += 2;
    }

    return port;
}

boolean_t isMiddlewareSecure() {
    boolean_t secure;

    if (strlen(middleware_secure) < 1) return boolean_false;
    secure = boolean_toBoolean(middleware_secure);

    return secure;
}
