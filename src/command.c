#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <la/boolean.h>
#include <la/datetime.h>
#include <la/directory.h>
#include <la/message.h>
#include <la/number.h>
#include <la/stringbuffer.h>
#include "command.h"
#include "config.h"
#include "system.h"

#define NETSTAT "netstat -ant"
#define BUFFER 1024
#ifdef __WIN32
#define CHECK "jar > NUL 2>&1"
#else
#define CHECK "jar &> /dev/null"
#endif
#define JAR "jar -cfMv"

static char *load() {
    stringbuffer_t *sb;
    char buf[BUFFER];
    FILE *ph;
    char *ret;

    sb = stringbuffer_init();
    ph = popen(NETSTAT, "r");
    if (!ph) return NULL;

    while (fgets(buf, BUFFER, ph)) {
        stringbuffer_append(sb, buf);
    }

    pclose(ph);

    ret = stringbuffer_getText(sb);

    stringbuffer_free(sb);

    return ret;
}

static boolean_t isPort(char *ns, int port) {
    char *num;
    char term[6]; /* :1234\0 */

    assert(ns);

    if (port <= 0 || port > 9999) return boolean_false;
    
    num = number_unsignedIntegerToString(port);
    strcpy(term, ":");
    strcat(term, num);
    free(num);

    if (strstr(ns, term)) return boolean_true;
    else return boolean_false;
}

boolean_t commandCheck() {
    char *ns;

    ns = load();
    printf("AdminServer HTTP...................[%s]\n", isPort(ns, getMiddlewareConsole()) ? "ONLINE" : "OFFLINE");
    if (isMiddlewareSecure()) printf("AdminServer HTTPS..................[%s]\n", isPort(ns, getMiddlewareConsole() + 1) ? "ONLINE" : "OFFLINE");
    printf("BI Server HTTP.....................[%s]\n", isPort(ns, getMiddlewareAnalytics()) ? "ONLINE" : "OFFLINE");
    if (isMiddlewareSecure()) printf("BI Server HTTPS....................[%s]\n", isPort(ns, getMiddlewareAnalytics() + 1) ? "ONLINE" : "OFFLINE");
    printf("AdminServer Internal...............[%s]\n", isPort(ns, getMiddlewareConsole() + 4) ? "ONLINE" : "OFFLINE");
    printf("BI Server Internal.................[%s]\n", isPort(ns, getMiddlewareConsole() + 5) ? "ONLINE" : "OFFLINE");
    printf("NodeManager........................[%s]\n", isPort(ns, getMiddlewareConsole() + 6) ? "ONLINE" : "OFFLINE");
    printf("BI Presentation Services...........[%s]\n", isPort(ns, getMiddlewareConsole() + 7) ? "ONLINE" : "OFFLINE");
    printf("Cluster Controller.................[%s]\n", isPort(ns, getMiddlewareConsole() + 8) ? "ONLINE" : "OFFLINE");
    printf("Cluster Controller Monitor.........[%s]\n", isPort(ns, getMiddlewareConsole() + 9) ? "ONLINE" : "OFFLINE");
    printf("Java Host..........................[%s]\n", isPort(ns, getMiddlewareConsole() + 10) ? "ONLINE" : "OFFLINE");
    printf("Scheduler..........................[%s]\n", isPort(ns, getMiddlewareConsole() + 11) ? "ONLINE" : "OFFLINE");
    printf("Scheduler Monitor..................[%s]\n", isPort(ns, getMiddlewareConsole() + 12) ? "ONLINE" : "OFFLINE");
    printf("Scheduler Script...................[%s]\n", isPort(ns, getMiddlewareConsole() + 13) ? "ONLINE" : "OFFLINE");
    printf("Server Nodes.......................[%s]\n", isPort(ns, getMiddlewareConsole() + 14) ? "ONLINE" : "OFFLINE");
    printf("Server Monitor Nodes...............[%s]\n", isPort(ns, getMiddlewareConsole() + 15) ? "ONLINE" : "OFFLINE");
    printf("Administration Tool................[%s]\n", isPort(ns, getMiddlewareConsole() + 16) ? "ONLINE" : "OFFLINE");
    free(ns);

    return boolean_true;
}

static char *getBitools(const char *name) {
    char *home = NULL;
    char *domain = NULL;
    char *cmd = NULL;
    stringbuffer_t *sb = NULL;

    home = getMiddlewareHome();
    printf("HOME: %s\n", home);
    if (!home) goto quit;
    domain = getMiddlewareDomain();
    printf("DOMAIN: %s\n", domain);
    if (!domain) goto quit;

    sb = stringbuffer_init();
    stringbuffer_append(sb, home);
    stringbuffer_append(sb, DIRECTORY_SEPARATOR_STRING);
    stringbuffer_append(sb, "user_projects");
    stringbuffer_append(sb, DIRECTORY_SEPARATOR_STRING);
    stringbuffer_append(sb, "domains");
    stringbuffer_append(sb, DIRECTORY_SEPARATOR_STRING);
    stringbuffer_append(sb, domain);
    stringbuffer_append(sb, DIRECTORY_SEPARATOR_STRING);
    stringbuffer_append(sb, "bitools");
    stringbuffer_append(sb, DIRECTORY_SEPARATOR_STRING);
    stringbuffer_append(sb, "bin");
    stringbuffer_append(sb, DIRECTORY_SEPARATOR_STRING);
    stringbuffer_append(sb, name);

    cmd = stringbuffer_getText(sb);

quit:
    if (sb)stringbuffer_free(sb);

    return cmd;
}

static boolean_t commandRun(const char *name) {
    char *cmd;
    int rc;

    cmd = getBitools(name);
    puts(cmd);
#ifdef __WIN32
        rc = system(cmd);
#else
    if (system_isAdmin()) {
        char *ora;
        char *user;
        stringbuffer_t *sb;

        user = getSystemUsername();
        sb = stringbuffer_init();
        stringbuffer_append(sb, "su -l ");
        stringbuffer_append(sb, user);
        stringbuffer_append(sb, " -l \"");
        stringbuffer_append(sb, cmd);
        stringbuffer_append(sb, "\"");
        ora = stringbuffer_getText(sb);
        stringbuffer_free(sb);

        rc = system(ora);

        free(ora);
    } else {
        rc = system(cmd);
    }
#endif
    free(cmd);

    return rc ? boolean_false : boolean_true;
}

boolean_t commandStatus() {
#ifdef __WIN32
    return commandRun("status.cmd");
#else
    return commandRun("status.sh");
#endif
}

boolean_t commandStart() {
#ifdef __WIN32
    return commandRun("start.cmd");
#else
    return commandRun("start.sh");
#endif
}

boolean_t commandStop() {
#ifdef __WIN32
    return commandRun("stop.cmd");
#else
    return commandRun("stop.sh");
#endif
}

/*
 * checks AdminServer and ManagedServer
 */
static boolean_t isOnline() {
    char *netstat; /* buffer of netstat */
    boolean_t status;

    netstat = load();
    status = (isPort(netstat, 9500) && isPort(netstat, 9502)) ? boolean_true : boolean_false;
    free(netstat);

    return status;
}

/*
 * waits for shutting down servers
 */
static boolean_t waitForShutdown() {
    int count;

    printf("\nwaiting for shutdown ");
    fflush(stdout);
    for (count = 0; count < 50; ++count) {
        /* if online, waiting were okay */
        if (!isOnline()) {
            puts(" success");
            return boolean_true;
        }

        printf(".");
        fflush(stdout);

#ifdef __WIN32
        Sleep(5000);
#else
        sleep(5);
#endif
    }
    puts(" failure");

    return boolean_false;
}

/*
 * https://docs.oracle.com/cd/E28280_01/core.1111/e10105/br_intro.htm#ASADM11238
 *
 * stop server
 * jar -cfM /u01/backup/date.zip /u01/app/oracle/Middleware
 * start server
 */
boolean_t commandBackup() {
    boolean_t online; /* initial status */
    int rc = 0;
    char *timestamp = NULL;
    char *filename = NULL; /* name of the archive */
    char *home = NULL;
    char *command = NULL; /* jar -cfM test.zip -C /u01/app/oracle/Middleware . */

    /* check if connando exists */
#if 0
    process = popen("jar 2> /dev/null", "r");
    if (!process) return boolean_false;
    while (fgets(buf, 1024, process)) {
        puts(buf);
    }
    pclose(process);
exit(1);
#endif
    rc = system(CHECK);
#ifndef __WIN32
    rc = rc >> 8;
#endif
    if (rc != 1) {
        message_error("Java not found (%d)", rc);
        rc = 1;
        goto quit;
    }

    /* check status */
    online = isOnline();

    /* shutdown if the servers are online */
    if (online) {
        message_info("shutdown servers");
        rc = commandStop();
        if (waitForShutdown() == boolean_false) {
            message_error("unable to stop servers (%d)", rc);
            goto quit;
        }
    }

        

    /* create filename */
    timestamp = datetime_getTimestampAsString();

    /* create archive */
    filename = (char *)malloc(strlen("middleware") +  1 + strlen(timestamp) + strlen(".zip") + 1);
    sprintf(filename, "middleware.%s.zip", timestamp);

    /* get home of middleware */
    home = getMiddlewareHome();
    if (!home) goto quit;


    /* build comman      jar -cfM          middleware.ts.zip      -C      MW_HOME            .   \0 */
    command = (char *)malloc(strlen(JAR) + 1 + strlen(filename) + 1 + 2 + 1 + strlen(home) + 1 + 1 + 1);
    sprintf(command, "%s %s -C %s .", JAR, filename, home);

    message_info("backup filesystem (%s -> %s)", home, filename);

    rc = system(command);
    if ((rc >> 8) > 1) {
        message_error("error while create archive (%d)", rc);
        /* don't quit - try to start servers */
    }

    /* startup if the servers where online */
    if (online) {
        message_info("startup servers");
        rc = commandStart();
        if (rc) {
            message_error("unable to start servers (%d)", rc);
            goto quit;
        }
    }

quit:
    if (command) free(command);
    if (home) free(home);
    if (filename) free(filename);
    if (timestamp) free(timestamp);

    return rc ? boolean_true : boolean_false;
}
