/* Fast DHT Lirary
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 * Modified By Zachary Graham (zwgraham@soe.ucsc.edu)
 * * Uses AVR-LIBC ATOMIC_BLOCK
 * * Chagned run-time pin definition to compile-time
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 *
 * Original library written by Adafruit Industries. MIT license.
 */

#include "dht.h"
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <avr/interrupt.h>


#define DHT_COUNT 6
#define DHT_MAXTIMINGS 85


//void dht_init(struct dht22 *dht, uint8_t pin)
void dht_init(struct dht22 *dht)
{
//    dht->pin = DHT_BIT
    /* Setup the pins! */
    DHT_DDR &= ~_BV(DHT_BIT);
    DHT_PORT |= _BV(DHT_BIT);
    dht->data[0] = 0;
    dht->data[1] = 0;
    dht->data[2] = 0;
    dht->data[3] = 0;
    dht->data[4] = 0;
    dht->data[5] = 0;
}

static uint8_t dht_read(struct dht22 *dht)
{
    uint8_t tmp;
    uint8_t sum = 0;
    uint8_t j = 0, i;
    uint8_t last_state = 1;
    uint16_t counter = 0;
    /*
     * Pull the pin 1 and wait 250 milliseconds
     */
//    DHT_PORT |= _BV(DHT_BIT);
//    _delay_ms(250);
// this is unneccessary?
    /* clear previously recieved data */
    dht->data[0] = dht->data[1] = dht->data[2] = dht->data[3] = dht->data[4] = 0;

    /* 
     * Send start condition to DHT
     *     BUS=0 for at least 1ms
     */
    //perhaps add to atomic block
    DHT_DDR |= _BV(DHT_BIT);
    DHT_PORT &= ~_BV(DHT_BIT);
    //_delay_ms(20); //20x minimum delay seems excessive
    _delay_ms(2);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        /* 
         * release bus
         */
        DHT_DDR &= ~_BV(DHT_BIT);
        /*
         * DHT waits 20-40us before response
         */
        _delay_us(20);

        /* Read the timings */
        for (i = 0; i < DHT_MAXTIMINGS; i++) {
            counter = 0;
            /*
             * watch the bus, until it transitions to a new state
             */
            while (1) {
                tmp = (DHT_PIN & _BV(DHT_BIT)) ? 1 : 0;
                _delay_us(3);

                /* bus has transitioned from high->low
                 * or low->high
                 */
                if (tmp != last_state)
                    break;

                counter++;
                _delay_us(1);

                /* timeout has occured */
                if (counter == 255)
                    break;
            }

            last_state = (DHT_PIN & _BV(DHT_BIT)) ? 1 : 0;

            /* finish timeout exit */
            if (counter == 255)
                break;

            /* Ignore first 3 transitions */
            if ((i >= 4) && (i % 2 == 0)) {
                /* Shove each bit into the storage bytes */
                dht->data[j/8] <<= 1;
                if (counter > DHT_COUNT)
                    dht->data[j/8] |= 1;
                j++;
            }
        }
    }
    sum = dht->data[0] + dht->data[1] + dht->data[2] + dht->data[3];

    if ((j >= 40) && (dht->data[4] == (sum & 0xFF)))
        return 1;
    return 0;
}

uint8_t dht_read_temp(struct dht22 *dht, float *temp)
{
    if (dht_read(dht)) {
        *temp = dht->data[2] & 0x7F;
        *temp *= 256;
        *temp += dht->data[3];
        *temp /= 10;

        if (dht->data[2] & 0x80)
            *temp *= -1;
        return 1;
    }
    return 0;
}

uint8_t dht_read_hum(struct dht22 *dht, float *hum)
{
    if (dht_read(dht)) {
        *hum = dht->data[0];
        *hum *= 256;
        *hum += dht->data[1];
        *hum /= 10;
        if (*hum == 0.0f)
            return 0;
        return 1;
    }
    return 0;
}

uint8_t dht_read_data(struct dht22 *dht, float *temp, float *hum)
{
    if (dht_read(dht)) {
        /* Reading temperature */
        *temp = dht->data[2] & 0x7F;
        *temp *= 256;
        *temp += dht->data[3];
        *temp /= 10;

        if (dht->data[2] & 0x80)
            *temp *= -1;

        /* Reading humidity */
        *hum = dht->data[0];
        *hum *= 256;
        *hum += dht->data[1];
        *hum /= 10;
        if (*hum == 0.0f)
            return 0;
        return 1;
    }
    return 0;
}
