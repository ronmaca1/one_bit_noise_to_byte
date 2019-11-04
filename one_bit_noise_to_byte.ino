#include <stdio.h>
#include <SPI.h>

#define DACSEL_L    10
#define LDAC_L      9
#define DACACFG     0x10   // gain of 1, shdn disabled
#define DACBCFG     0x90   // gain of 1, shdn disabled
#define HEARTBEAT   8

uint8_t dacoutH,dacoutL;
uint8_t chrcount=0;
char buffer[9];
volatile uint8_t bitcounter = 0;
volatile uint8_t noisebytevalue = 0;
volatile bool sampleready = false;
volatile bool bitval = false;


void samplebit(void) {
    

    bitval = digitalRead(2);

    
    if (bitval == true) noisebytevalue |= (1 << bitcounter);
    if (bitval == false) noisebytevalue &= ~(1 << bitcounter);
    //else noisebytevalue &= (0 << bitcounter);
    
    bitcounter++;
    if(bitcounter >7) {
            digitalWrite(HEARTBEAT,HIGH);
    digitalWrite(HEARTBEAT,LOW);
        bitcounter = 0;
        sampleready = true;
    }
    
}

void writedacs(unsigned char randval) {
    // DACA
    dacoutH = DACACFG | (((randval)>>4) & 0x0F);
    dacoutL = ((randval)<<4) & 0xF0;
    digitalWrite(DACSEL_L, LOW);
    delayMicroseconds(1);            // let the DAC get ready
    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);
    delayMicroseconds(1);
    digitalWrite(DACSEL_L, HIGH);
    // wait a little before the B dac
    delayMicroseconds(1);
    // DACB
    dacoutH = DACBCFG | (((randval)>>4) & 0x0F);
    dacoutL = ((randval)<<4) & 0xF0;
    digitalWrite(DACSEL_L, LOW);
    delayMicroseconds(1);
    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);
    delayMicroseconds(1);            // let the DAC settle
    digitalWrite(DACSEL_L, HIGH);
    // done with dac A and B
    digitalWrite(LDAC_L, LOW);
    delayMicroseconds(1);
    digitalWrite(LDAC_L, HIGH);

}

void setup() {

    attachInterrupt(digitalPinToInterrupt(2), samplebit, CHANGE);
    // put your setup code here, to run once:
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();

    Serial.begin(115200);

    pinMode(2,INPUT_PULLUP);

    digitalWrite(DACSEL_L,HIGH);
    pinMode(DACSEL_L,OUTPUT);
    digitalWrite(LDAC_L,HIGH);
    pinMode(LDAC_L,OUTPUT);
    digitalWrite(HEARTBEAT,HIGH);
    pinMode(HEARTBEAT,OUTPUT);

}

void loop() {
  
        if(sampleready) {
            cli();
            sampleready = false;
            writedacs(noisebytevalue);
            sprintf(buffer, "%03d", noisebytevalue);
            Serial.print(buffer);
            chrcount++;
            if(chrcount < 15) {
                Serial.print(" - ");
            }
            else {
                chrcount = 0;
                Serial.println();
            }
            noisebytevalue = 0;
        }
        
        sei();
}
