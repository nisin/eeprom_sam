
#include "eeprom_sam.h"

// #define DEBUG 1
#undef DEBUG



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
bool eeprom_catalog::read_header(unsigned long pos, eesam_head* head) {
    bool rc = (_vol->read(pos,(void*)head,sizeof(*head))==0) ;
#ifdef DEBUG
    Serial.print("read header. rlen:");
    Serial.print(head->rlen);
    Serial.print(" capacity:");
    Serial.print(head->capa);
    Serial.print(" type:");
    Serial.print(head->type);
    Serial.print(" pos:");
    Serial.println(pos);
#endif
    return rc;
}

eeprom_sam eeprom_catalog::create(uint8_t reclen, uint16_t capacity) {
    unsigned long volcap,pos =0;
    volcap = _vol->capacity();
    eesam_head head ;
    while (read_header(pos,&head)) {
        if (head.rlen==0 && head.capa==0) {
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
            head.type = 0;
            head.rlen = reclen;
            head.capa = capacity;
            _vol->write(pos,(void*)&head,sizeof(head));
            delay(1);
            return eeprom_sam(_vol,pos);
        }
        pos += (head.rlen * head.capa) + sizeof(head);
        if (pos>volcap) break;
    }
    return eeprom_sam(); // its empty.
}
int eeprom_catalog::count() {
    unsigned long volcap,pos =0;
    volcap = _vol->capacity();
    int count = 0;
    eesam_head head;
    while (read_header(pos,&head)) {
        if (head.rlen==0 && head.capa==0) 
            break;

        count++;
        pos += head.rlen * head.capa + sizeof(head);
        if (pos>=volcap) break;
    }
    return count;
}
eeprom_sam eeprom_catalog::sam(int entry) {
    unsigned long volcap,pos =0;
    volcap = _vol->capacity();
    eesam_head head ;
    int count = 0;
    while (read_header(pos,&head)) {
        if (head.rlen==0 && head.capa==0) 
            break;
        
        if (count == entry) 
            return eeprom_sam(_vol,pos);
        count++;
        pos += head.rlen * head.capa + sizeof(head);
        if (pos>=volcap) break;
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
    if (byte rc = vol->read(pos,(void*)&_head,sizeof(_head))) {
#ifdef DEBUG
        Serial.print("sam> header read error. -- ");
        Serial.println(rc);
#endif
        return;
    }  
    _origin=pos+sizeof(_head);
    _position=0;
    _vol=vol;
#ifdef DEBUG
    Serial.println("eeprom_sam.");
    Serial.print("vol:");
    Serial.println((int)_vol,HEX);
    Serial.print("origin:");
    Serial.println(_origin);
    Serial.print("reclen:");
    Serial.println(_head.rlen);
    Serial.print("capacity:");
    Serial.println(_head.capa);
    Serial.print("position:");
    Serial.println(_position);
#endif
}
bool eeprom_sam::read(void * record) {
    if (_vol==NULL) return false;
    if (_position >= _head.capa) return false;
    unsigned long addr = _origin + (_position * _head.rlen);
    if (_vol->read(addr,record,_head.rlen)==0) {
        _position++;
        return true;
    }
    return false;
}
bool eeprom_sam::write(void * record) {
    return write(record,false);
}
bool eeprom_sam::write(void * record, bool sure) {
    if (_vol==NULL) return false;
    if (_position >= _head.capa) return false;
    unsigned long addr = _origin + (_position * _head.rlen);
    byte buf[_head.rlen] = {0};
    do {
        if (_vol->write(addr,record,_head.rlen)!=0) return false; 
        if (sure) {
            if (_vol->read(addr,buf,_head.rlen)!=0) return false;
        }
        else break;
    } while(memcmp(record,buf,_head.rlen));
    _position++;
    return true;
}
unsigned long eeprom_sam::size() {
    return _head.rlen * _head.capa;
}
bool eeprom_sam::available() {
    if (_vol==NULL) return false;
    if (_position >= _head.capa) return false;
    return true;    
}
