
#ifndef _EEPROM_SAM_H
#define _EEPROM_SAM_H

#include "Arduino.h"
#include "extEEPROM.h"

typedef struct {
    uint8_t  type;
    uint8_t  rlen;
    uint16_t capa;
} eesam_head;

class eeprom_sam {
public:
    eeprom_sam(extEEPROM* vol, unsigned long pos);
    eeprom_sam();
    bool read(void * record);
    bool write(void * record);
    bool write(void * record, bool sure);
    unsigned long size();
    bool available();

private:
    extEEPROM* _vol;
    unsigned long _origin;
    eesam_head _head;
    uint16_t _position;
} ;

class eeprom_catalog {
public:
    static eeprom_catalog catalog(extEEPROM* vol);
    eeprom_catalog(extEEPROM* vol) ;
    void init_rom();
    eeprom_sam create(uint8_t reclen, uint16_t capacity);
    int count();
    eeprom_sam sam(int entry);

private:
    bool read_header(unsigned long pos, eesam_head* head);

    extEEPROM* _vol;
};


#endif
