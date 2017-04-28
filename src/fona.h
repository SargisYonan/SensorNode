
#ifndef FONA_H_
#define FONA_H_

#include <string.h>
#include "module.h"

#ifndef FONA_MAX 
#define FONA_MAX 10
#endif

#define FONA_IDENTIFIER_STRING "FONA"

typedef Module Fona;

Fona new_fona(uint8_t, Fona);

void fona_init(Fona f);

void fona_write(Fona f, char *);

void fona_destroy(Fona f);

#endif

