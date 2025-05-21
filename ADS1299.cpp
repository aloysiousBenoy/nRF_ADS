//  ADS1299.cpp
//  
//  Created by Conor Russomanno on 6/17/13.
//  Modified to use SPI library

#include "pins_arduino.h"
#include "ADS1299.h"
#include <SPI.h>  // Include the SPI library

void ADS1299::setup(int _DRDY, int _CS){
    
    // **** ----- SPI Setup ----- **** //
    
    // Initialize the SPI library
    SPI.begin();
    
    // Set SPI parameters
    // SPI_CLOCK_DIV16 gives ~1MHz with 16MHz clock
    // MODE1 = clock polarity 0, clock phase 1
    // MSBFIRST = most significant bit first
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    
    // Initialize chip select and data ready pins
    DRDY = _DRDY;
    CS = _CS;
    pinMode(DRDY, INPUT);
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH); // Deselect chip initially
    
    tCLK = 0.000666; // 666 ns (Datasheet, pg. 8)
    outputCount = 0;
    
    SPI.endTransaction();
}

//System Commands
void ADS1299::WAKEUP() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW); // Select chip
    SPI.transfer(_WAKEUP);
    digitalWrite(CS, HIGH); // Deselect chip
    SPI.endTransaction();
    delay(4.0*tCLK);  // Must wait at least 4 tCLK cycles before sending another command (Datasheet, pg. 35)
}

void ADS1299::STANDBY() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW);
    SPI.transfer(_STANDBY);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS1299::RESET() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW);
    SPI.transfer(_RESET);
    delay(10); // Using a longer delay for reset
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS1299::START() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW);
    SPI.transfer(_START);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS1299::STOP() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW);
    SPI.transfer(_STOP);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

//Data Read Commands
void ADS1299::RDATAC() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW);
    SPI.transfer(_RDATAC);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS1299::SDATAC() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW);
    SPI.transfer(_SDATAC);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

void ADS1299::RDATA() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW);
    SPI.transfer(_RDATA);
    digitalWrite(CS, HIGH);
    SPI.endTransaction();
}

//Register Read/Write Commands
void ADS1299::getDeviceID() {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW); // Select chip
    SPI.transfer(_SDATAC); // SDATAC
    SPI.transfer(_RREG); // RREG
    SPI.transfer(0x00); // Asking for 1 byte
    byte data = SPI.transfer(0x00); // Byte to read (hopefully 0b???11110)
    SPI.transfer(_RDATAC); // Turn read data continuous back on
    digitalWrite(CS, HIGH); // Deselect chip
    SPI.endTransaction();
    Serial.println(data, BIN);
}

void ADS1299::RREG(byte _address) {
    byte opcode1 = _RREG + _address; // 001rrrrr; _RREG = 00100000 and _address = rrrrr
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW); // Select chip
    SPI.transfer(_SDATAC); // SDATAC
    SPI.transfer(opcode1); // RREG
    SPI.transfer(0x00); // opcode2
    byte data = SPI.transfer(0x00); // Returned byte should match default of register map unless edited manually
    printRegisterName(_address);
    Serial.print("0x");
    if(_address<16) Serial.print("0");
    Serial.print(_address, HEX);
    Serial.print(", ");
    Serial.print("0x");
    if(data<16) Serial.print("0");
    Serial.print(data, HEX);
    Serial.print(", ");
    for(byte j = 0; j<8; j++){
        Serial.print(bitRead(data, 7-j), BIN);
        if(j!=7) Serial.print(", ");
    }
    SPI.transfer(_RDATAC); // Turn read data continuous back on
    digitalWrite(CS, HIGH); // Deselect chip
    SPI.endTransaction();
    Serial.println();
}

void ADS1299::RREG(byte _address, byte _numRegistersMinusOne) {
    byte opcode1 = _RREG + _address; // 001rrrrr; _RREG = 00100000 and _address = rrrrr
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW); // Select chip
    SPI.transfer(_SDATAC); // SDATAC
    SPI.transfer(opcode1); // RREG
    SPI.transfer(_numRegistersMinusOne); // opcode2
    for(byte i = 0; i <= _numRegistersMinusOne; i++){
        byte data = SPI.transfer(0x00); // Returned byte should match default of register map unless edited
        printRegisterName(i);
        Serial.print("0x");
        if(i<16) Serial.print("0"); // Lead with 0 if value is between 0x00-0x0F to ensure 2 digit format
        Serial.print(i, HEX);
        Serial.print(", ");
        Serial.print("0x");
        if(data<16) Serial.print("0"); // Lead with 0 if value is between 0x00-0x0F to ensure 2 digit format
        Serial.print(data, HEX);
        Serial.print(", ");
        for(byte j = 0; j<8; j++){
            Serial.print(bitRead(data, 7-j), BIN);
            if(j!=7) Serial.print(", ");
        }
        Serial.println();
    }
    SPI.transfer(_RDATAC); // Turn read data continuous back on
    digitalWrite(CS, HIGH); // Deselect chip
    SPI.endTransaction();
}

void ADS1299::WREG(byte _address, byte _value) {
    byte opcode1 = _WREG + _address; // 001rrrrr; _RREG = 00100000 and _address = rrrrr
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    digitalWrite(CS, LOW); // Select chip
    SPI.transfer(_SDATAC); // SDATAC
    SPI.transfer(opcode1);
    SPI.transfer(0x00);
    SPI.transfer(_value);
    SPI.transfer(_RDATAC);
    digitalWrite(CS, HIGH); // Deselect chip
    SPI.endTransaction();
    Serial.print("Register 0x");
    Serial.print(_address, HEX);
    Serial.println(" modified.");
}

void ADS1299::updateData(){
    if(digitalRead(DRDY) == LOW){
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
        digitalWrite(CS, LOW);
        long output[9];
        long dataPacket;
        for(int i = 0; i<9; i++){
            for(int j = 0; j<3; j++){
                byte dataByte = SPI.transfer(0x00);
                dataPacket = (dataPacket<<8) | dataByte;
            }
            output[i] = dataPacket;
            dataPacket = 0;
        }
        digitalWrite(CS, HIGH);
        SPI.endTransaction();
        Serial.print(outputCount);
        Serial.print(", ");
        for (int i=0; i<9; i++) {
            Serial.print(output[i], HEX);
            if(i!=8) Serial.print(", ");
        }
        Serial.println();
        outputCount++;
    }
}

// String-Byte converters for RREG and WREG
void ADS1299::printRegisterName(byte _address) {
    if(_address == ID){
        Serial.print("ID, ");
    }
    else if(_address == CONFIG1){
        Serial.print("CONFIG1, ");
    }
    else if(_address == CONFIG2){
        Serial.print("CONFIG2, ");
    }
    else if(_address == CONFIG3){
        Serial.print("CONFIG3, ");
    }
    else if(_address == LOFF){
        Serial.print("LOFF, ");
    }
    else if(_address == CH1SET){
        Serial.print("CH1SET, ");
    }
    else if(_address == CH2SET){
        Serial.print("CH2SET, ");
    }
    else if(_address == CH3SET){
        Serial.print("CH3SET, ");
    }
    else if(_address == CH4SET){
        Serial.print("CH4SET, ");
    }
    else if(_address == CH5SET){
        Serial.print("CH5SET, ");
    }
    else if(_address == CH6SET){
        Serial.print("CH6SET, ");
    }
    else if(_address == CH7SET){
        Serial.print("CH7SET, ");
    }
    else if(_address == CH8SET){
        Serial.print("CH8SET, ");
    }
    else if(_address == BIAS_SENSP){
        Serial.print("BIAS_SENSP, ");
    }
    else if(_address == BIAS_SENSN){
        Serial.print("BIAS_SENSN, ");
    }
    else if(_address == LOFF_SENSP){
        Serial.print("LOFF_SENSP, ");
    }
    else if(_address == LOFF_SENSN){
        Serial.print("LOFF_SENSN, ");
    }
    else if(_address == LOFF_FLIP){
        Serial.print("LOFF_FLIP, ");
    }
    else if(_address == LOFF_STATP){
        Serial.print("LOFF_STATP, ");
    }
    else if(_address == LOFF_STATN){
        Serial.print("LOFF_STATN, ");
    }
    else if(_address == GPIO){
        Serial.print("GPIO, ");
    }
    else if(_address == MISC1){
        Serial.print("MISC1, ");
    }
    else if(_address == MISC2){
        Serial.print("MISC2, ");
    }
    else if(_address == CONFIG4){
        Serial.print("CONFIG4, ");
    }
}

//SPI communication methods
byte ADS1299::transfer(byte _data) {
    // Replace custom transfer with SPI library transfer
    return SPI.transfer(_data);
}