
#ifndef FONA_H_
#define FONA_H_

#include <string.h>
#include "module.h"

#ifndef FONA_MAX
#define FONA_MAX 1
#endif

#define FONA_IDENTIFIER_STRING "FONA"

typedef Module Fona;

Fona new_fona(uint8_t, Fona);

void fona_init(Fona f);

void fona_read(Fona f, char *, uint16_t);

void fona_write(Fona f, char *, uint16_t);

void fona_destroy(Fona f);

#endif

