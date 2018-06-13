#ifndef COMMAND_H
#define COMMAND_H

#include <la/boolean.h>

boolean_t commandCheck();
boolean_t commandStatus();
boolean_t commandStart();
boolean_t commandStop();
boolean_t commandBackup();

#endif
