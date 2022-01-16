#include <gb/gb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "PrintCmd.h"
#include "tiles.h"
#include "background.h"
#include "font.h"
#include "cursors.h"

void print_tile(uint8_t* tile_data, uint8_t letter, uint8_t inverted)
{
    uint8_t tx, ty;
    uint8_t *tile_index = tile_data + letter * 16;
    uint8_t p_data[16];
    uint8_t i, val;
    uint8_t background = inverted? 0x00:0xFF;
    uint8_t foreground = inverted? 0xFF:0x00;
    // We embed a 8x8 graphics in a 20x16 tileset
    for (ty = 1; ty < 8; ty++)
    {
        for (i = 0; i < 2; i++)
        {
            // first tile is background
            memset(p_data, background, sizeof(p_data));
            PrintTileData(p_data, 0);
            PrintTileData(p_data, 0);
            for (tx = 0; tx < 8; tx++)
            {
                // get half nibble
                val = *(tile_index + (7-tx) * 2 + 1);
                val = val & (1 << (7-ty));
                val = val > 0? foreground:background;
                memset(p_data, val, sizeof(p_data));
                PrintTileData(p_data, 0);
                PrintTileData(p_data, 0);
            }
            // last tile is background
            memset(p_data, background, sizeof(p_data));
            PrintTileData(p_data, 0);
            PrintTileData(p_data, 0);
        }
        GetPrinterStatus();
        if (CheckForErrors())
        {
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

void draw_settings(uint8_t inverted) {
    if (!inverted){
        set_sprite_tile(4, 12);
        set_sprite_tile(5, 14);
    } else {
        set_sprite_tile(4, 16);
        set_sprite_tile(5, 18);
    }
}

uint8_t print_buffer[17];

void main(void)
{
    uint8_t i, x, y, c = 0, inverted = 0;
    HIDE_BKG;
    SPRITES_8x16;
    set_bkg_data(128, 128, font);
    set_bkg_data(0, 10, tiles);
    set_bkg_tiles(0, 0, 20,18, background);
    set_sprite_data(0, 20, cursors);
    set_sprite_tile(0, 0);
    set_sprite_tile(1, 2);
    draw_printer(0);
    draw_settings(inverted);
    move_sprite(2, 12*8, 17*8);
    move_sprite(3, 13*8, 17*8);
    move_sprite(4, 8*8, 17*8);
    move_sprite(5, 9*8, 17*8);
    x = 2; y = 3;
    for(i = 0; i < 96; i++){
        set_vram_byte(get_bkg_xy_addr(x, y), 128 + 32 + i);
        if (++x > 0x11) {
            x = 2;
            ++y;
        }   
    }
    x = 2; y = 3;
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
                    draw_cursor(2+i, 0x0C);
                    print_tile(font, print_buffer[i], inverted);
                    GetPrinterStatus();
                    while(CheckBusy()) {
                        // do progress bar here
                        wait_vbl_done();
                    }
                }
                draw_cursor(x,y);
            }
        } else if (i == J_SELECT) {
            inverted = inverted? 0:1;
            draw_settings(inverted);
        } else if (i == J_A){
            if (c < 16 ) {
                set_vram_byte(get_bkg_xy_addr(2+c, 0x0C), 128 + 32 + (y-3)*16+(x-2));
                print_buffer[c] = 32 + (y-3)*16+(x-2);
                c++;
            }
        } else if (i == J_B){
            if (c > 0) {
                --c;
                set_vram_byte(get_bkg_xy_addr(2+c, 0x0C),0x9);
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
            if (++y > 0x8)
                y = 0x3;
            draw_cursor(x, y);
        } else if (i == J_UP){
            if (--y < 0x3)
                y = 0x8;
            draw_cursor(x, y);
        }
    }
}
