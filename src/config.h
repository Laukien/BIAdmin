#ifndef CONFIG_H
#define CONFIG_H

#include <la/boolean.h>

    boolean_t initConfig();
    boolean_t isConfig();
    boolean_t editConfig();
    char *getConfigError();
    char *getConfigFilename();

    char *getSystemUsername();
    char *getMiddlewareHome();
    char *getMiddlewareDomain();
    char *getMiddlewareUsername();
    int getMiddlewareConsole();
    int getMiddlewareAnalytics();
    boolean_t isMiddlewareSecure();

#endif
