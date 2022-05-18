#include <gb/gb.h>
#include <gb/cgb.h>
#include <string.h>
#include "PrintCmd.h"
#include "tiles.h"
#include "background.h"
#include "font.h"
#include "cursors.h"

void update_printer_progress(void) {
    static uint8_t status = 0;
    switch (status)
    {
    case 0:
        set_sprite_tile(2, 28);
        set_sprite_tile(3, 30);
        break;
    case 1:
        set_sprite_tile(2, 32);
        set_sprite_tile(3, 34);
        break;
    case 2:
        set_sprite_tile(2, 36);
        set_sprite_tile(3, 38);
        break;
    case 3:
        set_sprite_tile(2, 8);
        set_sprite_tile(3, 10);
        break;
    default:
        break;
    }
    status = (status + 1) % 4;
}


void print_tile(uint8_t* tile_data, uint8_t letter, uint8_t status, uint8_t linefeed)
{
    uint8_t tx, ty;
    uint8_t *tile_index = tile_data + letter * 16;
    uint8_t p_data[16];
    uint8_t i, val;
    uint8_t background = (status & 0x1)? 0x00:0xFF;
    uint8_t foreground = (status & 0x1)? 0xFF:0x00;
    uint8_t rotated = status & 0x2;
    uint8_t fontwidth = ((letter == '@' || letter >= '{') && !rotated)? 9:7;
    uint8_t printlength = (rotated)? 9:fontwidth;
    // We embed a 8x8 graphics in a 20x16 tileset
    ty = (fontwidth == 9 || rotated)? 0:1;
    // additional blank line at the beginning if rotated
    if (rotated) {
        memset(p_data, background, sizeof(p_data));
        for (i = 0; i < 40; i++) {
            PrintTileData(p_data, linefeed, printlength);
        }
    }
    for (; ty < 8; ty++)
    {
        for (i = 0; i < 2; i++)
        {
            // first tile is background
            memset(p_data, background, sizeof(p_data));
            PrintTileData(p_data, linefeed, printlength);
            PrintTileData(p_data, linefeed, printlength);
            for (tx = 0; tx < 8; tx++)
            {
                // get half nibble
                if (rotated) {
                    val = *(tile_index + ty * 2 + 1);
                    val = val & (1 << (7-tx));
                } else {
                    val = *(tile_index + (7-tx) * 2 + 1);
                    val = val & (1 << (7-ty));
                }
                val = val > 0? foreground:background;
                memset(p_data, val, sizeof(p_data));
                PrintTileData(p_data, linefeed, printlength);
                PrintTileData(p_data, linefeed, printlength);
            }
            // last tile is background
            memset(p_data, background, sizeof(p_data));
            PrintTileData(p_data, linefeed, printlength);
            PrintTileData(p_data, linefeed, printlength);
        }
        update_printer_progress();
        GetPrinterStatus();
        if (CheckForErrors()){
            return;
        }
    }
    if (fontwidth == 9) {
        // additional blank line at the end for wide characters
        memset(p_data, background, sizeof(p_data));
        for (i = 0; i < 40; i++) {
            PrintTileData(p_data, 0, printlength);
        }
        update_printer_progress();
        GetPrinterStatus();
        if (CheckForErrors()){
            return;
        }
    }
}


void draw_cursor(uint8_t x, uint8_t y) {
    move_sprite(0, 8*x+4, 8*(y+1)+4);
    move_sprite(1, 8*(x+1)+4, 8*(y+1)+4);
}


void draw_printer(uint8_t bad) {
    if (!bad){
        set_sprite_tile(3, 10);
        set_sprite_tile(2, 8);
    } else {
        set_sprite_tile(3, 6);
        set_sprite_tile(2, 4);
    }
}


void draw_settings(uint8_t status) {
    switch (status)
    {
    case 2:
        set_sprite_tile(4, 12);
        set_sprite_tile(5, 14);
        break;
    case 3:
        set_sprite_tile(4, 16);
        set_sprite_tile(5, 18);
        break;
    case 0:
        set_sprite_tile(4, 20);
        set_sprite_tile(5, 22);
        break;
    case 1:
        set_sprite_tile(4, 24);
        set_sprite_tile(5, 26);
        break;
    default:
        break;
    }
}


uint8_t print_buffer[17];
const uint16_t cgb_palette[4] = {21369, 2737, 6534, 2274};

void main(void)
{
    uint8_t i, x, y, c = 0, status = 0, wait = 0;
    HIDE_BKG;
    SPRITES_8x16;
    set_bkg_data(128, 128, font);
    set_bkg_data(0, 10, tiles);
    set_bkg_tiles(0, 0, 20,18, background);
    set_bkg_palette(0, 1, cgb_palette);
    set_sprite_palette(0, 1, cgb_palette);
    set_sprite_data(0, 40, cursors);
    set_sprite_tile(0, 0);
    set_sprite_tile(1, 2);
    draw_printer(0);
    draw_settings(status);
    move_sprite(2, 12*8, 16*8);
    move_sprite(3, 13*8, 16*8);
    move_sprite(4, 8*8, 16*8);
    move_sprite(5, 9*8, 16*8);
    x = 2; y = 2;
    for(i = 0; i < 96; i++){
        set_vram_byte(get_bkg_xy_addr(x, y), 128 + 32 + i);
        if (++x > 0x11) {
            x = 2;
            ++y;
        }   
    }
    x = 2; y = 2;
    draw_cursor(x, y);
    SHOW_BKG;
    SHOW_SPRITES;

    while(1) {
        do {
            i = joypad();
            draw_printer(GetPrinterStatus() || CheckLinkCable());
            wait_vbl_done();
        } while (!i);
        waitpadup();

        if (i == J_START) {
            if (!(GetPrinterStatus() && CheckLinkCable())){
                PrinterInit();
                for(i = 0; i < c; i++){
                    if (print_buffer[i] != 127) { //CR character
                        draw_cursor(2+i, 0x0B);
                        // if the next character is a CR, add linefeed
                        print_tile(font,
                                   print_buffer[i],
                                   status, print_buffer[i+1] == 127);
                        GetPrinterStatus();
                        while(!CheckBusy()) {
                            wait_vbl_done();
                        }
                        while(CheckBusy()) {
                            for (wait = 0; wait < 30; wait++)
                                wait_vbl_done();
                            update_printer_progress();
                        }
                    }
                }
                draw_cursor(x,y);
            }
        } else if (i == J_SELECT) {
            status = (status + 1) % 4;
            draw_settings(status);
        } else if (i == J_A){
            if (c < 16 ) {
                set_vram_byte(get_bkg_xy_addr(2+c, 0x0B), 128 + 32 + (y-2)*16+(x-2));
                print_buffer[c] = 32 + (y-2)*16+(x-2);
                c++;
            }
        } else if (i == J_B){
            if (c > 0) {
                --c;
                set_vram_byte(get_bkg_xy_addr(2+c, 0x0B),0x9);
            }
        } else if (i == J_LEFT){
            if (--x < 0x2)
                x = 0x11;
            draw_cursor(x, y);
        } else if (i == J_RIGHT){
            if (++x > 0x11)
                x = 0x2;
            draw_cursor(x, y);
        } else if (i == J_DOWN){
            if (++y > 0x7)
                y = 0x2;
            draw_cursor(x, y);
        } else if (i == J_UP){
            if (--y < 0x2)
                y = 0x7;
            draw_cursor(x, y);
        }
    }
}
