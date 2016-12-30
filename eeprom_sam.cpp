
#include "eeprom_sam.h"

#define DEBUG 1

eeprom_catalog eeprom_catalog::catalog(extEEPROM* vol) {
    return eeprom_catalog(vol);
}
eeprom_catalog::eeprom_catalog(extEEPROM* vol) {
    _vol=vol;
} 

void eeprom_catalog::init_rom() {
    byte buf[8] = {0};
    unsigned long capacity,pos =0;
    capacity = _vol->capacity();
    while (_vol->write(pos,buf,sizeof(buf))==0) {
        pos += sizeof(buf);
        if (pos>capacity) break;
    }
}
bool eeprom_catalog::read_header(unsigned long pos, uint8_t* len, uint16_t* cap) {
    int lensz = sizeof(*len);
    bool rc = (_vol->read(pos,len,lensz)==0 && 
        _vol->read(pos+lensz,(byte*)cap,sizeof(*cap))==0);
#ifdef DEBUG
    Serial.print("read header. rlen:");
    Serial.print(*len);
    Serial.print(" capacity:");
    Serial.print(*cap);
    Serial.print(" pos:");
    Serial.println(pos);
#endif
    return rc;
}

eeprom_sam eeprom_catalog::create(uint8_t reclen, uint16_t capacity) {
    unsigned long volcap,pos =0;
    volcap = _vol->capacity();
    uint8_t len ;
    uint16_t cap ;
    while (read_header(pos,&len,&cap)) {
        if (len==0 && cap==0) {
            _vol->write(pos,&reclen,sizeof(reclen));
            _vol->write(pos+sizeof(reclen),(byte*)&capacity,sizeof(capacity));
            unsigned long overrun = pos + sizeof(reclen) + sizeof(capacity) 
                + reclen * capacity;
#ifdef DEBUG
    Serial.print("overrun test. volcap:");
    Serial.print(volcap);
    Serial.print(" overrun:");
    Serial.println(overrun);
#endif
            if ( overrun > volcap )
                return eeprom_sam(); // its empty.
            delay(5);
            return eeprom_sam(_vol,pos);
        }
        pos += (len * cap) + sizeof(len) + sizeof(cap);
        if (pos>volcap) break;
    }
    return eeprom_sam(); // its empty.
}
int eeprom_catalog::count() {
    unsigned long volcap,pos =0;
    volcap = _vol->capacity();
    int count = 0;
    uint8_t len ;
    uint16_t cap ;
    while (read_header(pos,&len,&cap)) {
        if (len==0 && cap==0) {
            break;
        }
        count++;
        pos += len * cap + sizeof(len) + sizeof(cap);
        if (pos>volcap) break;
    }
    return count;
}
eeprom_sam eeprom_catalog::sam(int entry) {
    unsigned long volcap,pos =0;
    volcap = _vol->capacity();
    uint8_t len ;
    uint16_t cap ;
    int count = 0;
    while (read_header(pos,&len,&cap)) {
        if (len==0 && cap==0) {
            break;
        }
        if (count == entry) 
            return eeprom_sam(_vol,pos);
        count++;
        pos += len * cap + sizeof(len) + sizeof(cap);
        if (pos>volcap) break;
    }
    return eeprom_sam(); 
}


// sam

eeprom_sam::eeprom_sam() {
    _vol=NULL;
#ifdef DEBUG
    Serial.println("eeprom_sam. make null.");
#endif
}
eeprom_sam::eeprom_sam(extEEPROM* vol, unsigned long pos){
    _vol=NULL;
    if (byte rc = vol->read(pos,&_reclen,sizeof(_reclen))) {
#ifdef DEBUG
    Serial.print("sam> header reclen read error. -- ");
    Serial.println(rc);
#endif
        return;
    }  
    if (byte rc = vol->read(pos+sizeof(_reclen),(byte*)&_capacity, sizeof(_capacity))) {
#ifdef DEBUG
    Serial.print("sam> header capacity read error. -- ");
    Serial.println(rc);
#endif
        return;
    }
    _origin=pos+sizeof(_reclen)+sizeof(_capacity);
    _position=0;
    _vol=vol;
#ifdef DEBUG
    Serial.println("eeprom_sam.");
    Serial.print("vol:");
    Serial.println((int)_vol,HEX);
    Serial.print("origin:");
    Serial.println(_origin);
    Serial.print("reclen:");
    Serial.println(_reclen);
    Serial.print("capacity:");
    Serial.println(_capacity);
    Serial.print("position:");
    Serial.println(_position);
#endif
}
bool eeprom_sam::read(void * record) {
    if (_vol==NULL) return false;
    if (_position >= _capacity) return false;
    unsigned long addr = _origin + (_position * _reclen);
    if (_vol->read(addr,record,_reclen)==0) {
        _position++;
        return true;
    }
    return false;
}
bool eeprom_sam::write(void * record) {
    if (_vol==NULL) return false;
    if (_position >= _capacity) return false;
    unsigned long addr = _origin + (_position * _reclen);
    if (_vol->write(addr,record,_reclen)==0) {
        _position++;
        return true;
    }
    return false;
}
unsigned long eeprom_sam::size() {
    return _reclen * _capacity;
}
bool eeprom_sam::available() {
    if (_vol==NULL) return false;
    if (_position >= _capacity) return false;
    return true;    
}
