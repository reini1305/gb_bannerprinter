// Original library from http://shen.mansell.tripod.com/games/gameboy/gameboy.html
// Ported to GBDK 2020 by T0biasCZe https://drive.google.com/file/d/1pEHIUL0EyPDbkVUQ2C5DMvJX_z5_1e4X/view?usp=sharing

#include <gb/gb.h>
uint8_t PrinterStatus[3];

const uint8_t PRINTER_INIT[]={
    10,0x88,0x33,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00
};
const uint8_t PRINTER_STATUS[]={
    10,0x88,0x33,0x0F,0x00,0x00,0x00,0x0F,0x00,0x00,0x00
};
const uint8_t PRINTER_EOF[]={
    10,0x88,0x33,0x04,0x00,0x00,0x00,0x04,0x00,0x00,0x00
};
const uint8_t PRINTER_START[]={
    14,0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x13,0xE4,0x7F,0x7D,0x01,0x00,0x00
};
const uint8_t PRINT_TILE[]={
    6,0x88,0x33,0x04,0x00,0x80,0x02
};
const uint8_t PRINTER_LINE[]={
    14,0x88,0x33,0x02,0x00,0x04,0x00,0x01,0x00,0xE4,0x7F,0x6A,0x01,0x00,0x00
};

uint8_t tile_num, packet_num;

uint16_t CRC;

uint8_t SendPrinterByte(uint8_t byte){
    uint8_t result;
    disable_interrupts();
    SB_REG = byte; //data to send
    SC_REG = 0x81; //1000 0001 - start, internal clock
    while (SC_REG & 0x80){} //wait until b1 reset
    result = SB_REG; //return response stored in SB_REG
    enable_interrupts();
    return result;
}

void SendByte(uint8_t byte){
    uint8_t result;
    result = SendPrinterByte(byte);
    PrinterStatus[0]=PrinterStatus[1];
    PrinterStatus[1]=PrinterStatus[2];
    PrinterStatus[2]=result;
}

void SendPrinterCommand(uint8_t *Command){
    uint8_t length,index;
    index=0;
    length=*Command;
    while(index < length){
        index++;
        SendByte(*(Command+index));
    }
}

void PrinterInit (void)
{
    tile_num = 0;
    CRC = 0;
    packet_num = 0;

    SendPrinterCommand(PRINTER_INIT);
}

int CheckLinkCable(){
    if(PrinterStatus[0] != 0){
        return 2;
    }
    if((PrinterStatus[1] & 0xF0) != 0x80){
        return 2;
    }
    return 0;
}

int GetPrinterStatus(){
    SendPrinterCommand(PRINTER_STATUS);
    return CheckLinkCable();
}

int CheckForErrors(){
    if(PrinterStatus[2] & 128){
        return 1;
    }
    if(PrinterStatus[2] & 64){
        return 4;
    }
    if(PrinterStatus[2] & 32){
        return 3;
    }
    return 0;
}

uint8_t CheckBusy() {
    SendPrinterCommand(PRINTER_STATUS);
    return (PrinterStatus[2] & 0x2);
}

uint8_t GetHigh(uint16_t w) {
    return (w & 0xFF00u) >> 8;
}

uint8_t GetLow(uint16_t w) {
    return (w & 0xFFu);
}

void PrintTileData(uint8_t *TileData, uint8_t lf){
    uint8_t TileIndex;
    
    if (tile_num == 0)
    {
        SendPrinterCommand(PRINT_TILE);
        CRC = 0x04 + 0x80 + 0x02;
    }      
    
    tile_num ++;

    for(TileIndex = 0; TileIndex < 16; TileIndex++)
    {
        CRC += TileData[TileIndex];
        SendByte(TileData[TileIndex]);
    }

    if (tile_num == 40)
    {
        SendByte(GetLow(CRC));
        SendByte(GetHigh(CRC));
        SendByte(0x00);
        SendByte(0x00);
        tile_num = 0;  
        CRC = 0;
        packet_num ++;

        if (packet_num == 8) // all done the page
        {
            SendPrinterCommand(PRINTER_EOF); // data end packet
            if (lf)
                SendPrinterCommand(PRINTER_START);
            else
                SendPrinterCommand(PRINTER_LINE);
            packet_num = 0;
            SendPrinterCommand(PRINTER_STATUS);
        }
    }
}

