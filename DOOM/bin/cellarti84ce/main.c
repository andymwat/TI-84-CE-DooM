#include <tice.h>
#include /*nolink*/ <keypadc.h>
#include <string.h>
#include <debug.h>
#include <stdio.h>

#define third_byte(cochon) (*(((uint8_t *)&cochon)+2))
#define second_byte(cochon) (*(((uint8_t *)&cochon)+1))
#define first_byte(cochon) (*((uint8_t *)&cochon))

extern void draw_column_tall(uint8_t *column, uint8_t *texture, uint24_t texture_delta, uint8_t texture_fraction);
extern void draw_column_short(uint8_t *column, uint8_t *texture, uint8_t height, uint8_t ceiling_height, uint24_t texture_delta);

extern void copy_buffer(uint8_t *dest);
extern void draw_sprite(uint8_t *sprite, uint24_t texture_delta, uint8_t height);
extern void draw_column_tall_buffer(uint8_t *texture, uint24_t texture_delta, uint8_t texture_fraction);
extern void draw_column_short_buffer(uint8_t *texture, uint8_t height, uint8_t ceiling_height, uint24_t texture_delta);

extern void draw_string(const char *string, uint24_t coords);

#define DISPLAY_CONTROL ((volatile uint8_t *)0xE30018)
#define DISPLAY_NORMAL 0x2D
#define DISPLAY_16COLOR 0x25
#define key_control ((volatile uint8_t *)0xF50000)
#define timer_control ((volatile uint8_t *)0xF20030)
/* actually a 32-bit counter but we only use the low 24 bits */
#define timer1_counter ((volatile uint24_t *)0xF20000)

#include "palette.h"

struct shared 
{
        int24_t ray_x, ray_y, dxdy, dydx, test_x, test_y;
};

/* Coordinates within map are 24-bit with grid coordinate in top 8 bits */
int24_t player_x = 0x38000;
int24_t player_y = 0x38000;
uint24_t angle = 0;
struct shared shared_struct;
uint8_t wall;
uint32_t temp, temp_x, temp_y;
uint24_t texture_start_row;
uint8_t wall_x;
uint24_t radiation = 0;
uint24_t radiation_inc = 0;
uint8_t old_radiation = 0;
uint8_t *init_map_pointer;
static uint8_t radiation_colors[] = {0x66, 0x77, 0x99, 0xAA, 0xCC};

uint24_t frame_counter = 0;
int24_t sprite_x = 0xa8000;
int24_t sprite_y = 0xa8000;
int24_t sprite_distance, sprite_height, sprite_min_angle, sprite_max_angle, sprite_angle, sprite_image_dx, full_angle, sprite_texture_delta;
uint8_t *sprite_start;
uint8_t sprite_direction = 0;

#define MAP_WIDTH 16
#define MAP_HEIGHT 17

unsigned char map[MAP_HEIGHT][MAP_WIDTH] = {
        "1311111311111131",
        "1000000000000001",
        "1000000000011001",
        "1000000000051001",
        "1000000000011011",
        "1000000000000011",
        "1115102200000011",
        "1111102200000001",
        "1111100000000001",
        "1111111111111001",
        "1000000000000001",
        "1000000000000001",
        "1015110000000001",
        "1011111111111111",
        "1011000000000001",
        "1000011000100001",
        "1311111141111131"
};

/* Angles range from 0-1023 for a full circle */
#include "trig.h"
int24_t full_sine_table[1024];
int24_t secant_table[1024];
#define fixed_sine(x) full_sine_table[x & 0x3FF]
#define fixed_cosine(x) full_sine_table[(x + 256) % 0x3FF]
#define fixed_secant(x) full_sine_table[x & 0x3FF]

/* Square root table lookup algorithm : check if either of the top 2 bits is set.
If so, look up using the top 10 bits in a 256-1023 table () which is already shifted.
If not, shift left 2 bits (i.e. multiply by 4) and try again, always shifting in 1s.
Each additional cycle induces one right shift of the result. (i.e. divide by 2) */
#include "sqrt.h"
extern uint24_t square_root_fast(uint24_t x);

/* Wall textures, which must be aligned on a 64-byte boundary */
extern uint8_t walls_source[64*64*5];
uint8_t *walls;
#define NUM_WALLS 6
#define WALLS_SIZE NUM_WALLS * 64 * 64
uint8_t walls_area[WALLS_SIZE + 0x3F];

/* ray casting routines */

extern void octant_1_side(void);
extern void octant_1_main(void);
void octant1(void) /* mostly right, some down */
{        
        uint8_t iy = third_byte(shared_struct.ray_y);
        shared_struct.dydx = short_tan_table[full_angle];
        shared_struct.dxdy = short_cot_table[full_angle];
        shared_struct.ray_y = player_y - ((((player_x & 0xFFFF) >> 5) * (shared_struct.dydx >> 5)) >> 6);
        shared_struct.ray_x = player_x & 0xFF0000;      
        
        if (iy == third_byte(shared_struct.ray_y))
        {
                octant_1_side();
        }
        else
        {
                octant_1_main();
        }
        
        /*if (iy == third_byte(shared_struct.ray_y))
        {
                shared_struct.test_y = shared_struct.ray_y + shared_struct.dydx;
                if (third_byte(shared_struct.test_y) != third_byte(shared_struct.ray_y))
                {
                        map_pointer += MAP_WIDTH;
                        if (*map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_y & 0xFFFF;
                                shared_struct.ray_x = shared_struct.ray_x + ((((0x10000 - frac) >> 5) * shared_struct.dxdy) >> 6);
                                shared_struct.ray_y = (shared_struct.ray_y + 0x10000) & 0xFF0000;
                                return;
                        }
                }    
        }                             
        
        while (1)
        {
                shared_struct.ray_x += 0x10000;
                shared_struct.ray_y += shared_struct.dydx;
                if (*++map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
                shared_struct.test_y = shared_struct.ray_y + shared_struct.dydx;
                if (third_byte(shared_struct.test_y) != third_byte(shared_struct.ray_y))
                {
                        map_pointer += MAP_WIDTH;
                        if (*map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_y & 0xFFFF;
                                shared_struct.ray_x = shared_struct.ray_x + ((((0x10000 - frac) >> 5) * shared_struct.dxdy) >> 6);
                                shared_struct.ray_y = (shared_struct.ray_y + 0x10000) & 0xFF0000;
                                return;
                        }
                }      
        }*/
}

extern void octant_4_side(void);
extern void octant_4_main(void);
void octant4(void) /* mostly left, some down */
{        
        uint8_t iy = third_byte(shared_struct.ray_y);        
        int24_t frac = shared_struct.ray_x & 0xFFFF;
        shared_struct.dydx = short_tan_table[511 - full_angle];
        shared_struct.dxdy = short_cot_table[511 - full_angle];
        shared_struct.ray_y = shared_struct.ray_y - ((((0x10000 - frac) >> 5) * (shared_struct.dydx >> 5)) >> 6);
        shared_struct.ray_x = (shared_struct.ray_x & 0xFF0000) + 0x10000;
        
        if (iy == third_byte(shared_struct.ray_y))
        {
                octant_4_side();
        }
        else
        {
                octant_4_main();
        }
        
        /*if (iy == third_byte(shared_struct.ray_y))
        {
                shared_struct.test_y = shared_struct.ray_y + shared_struct.dydx;
                if (third_byte(shared_struct.test_y) != third_byte(shared_struct.ray_y))
                {                        
                        map_pointer += MAP_WIDTH;
                        if (*map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_y & 0xFFFF;
                                shared_struct.ray_x = shared_struct.ray_x - ((((0x10000 - frac) >> 5) * shared_struct.dxdy) >> 6);
                                shared_struct.ray_y = (shared_struct.ray_y + 0x10000) & 0xFF0000;
                                return;
                        }
                }   
        }                             
        
        while (1)
        {
                shared_struct.ray_x -= 0x10000;
                shared_struct.ray_y += shared_struct.dydx;                
                if (*--map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
                shared_struct.test_y = shared_struct.ray_y + shared_struct.dydx;
                if (third_byte(shared_struct.test_y) != third_byte(shared_struct.ray_y))
                {                        
                        map_pointer += MAP_WIDTH;
                        if (*map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_y & 0xFFFF;
                                shared_struct.ray_x = shared_struct.ray_x - ((((0x10000 - frac) >> 5) * shared_struct.dxdy) >> 6);
                                shared_struct.ray_y = (shared_struct.ray_y + 0x10000) & 0xFF0000;
                                return;
                        }
                }    
        }*/
}

extern void octant_5_side(void);
extern void octant_5_main(void);
void octant5(void) /* mostly left, some up*/
{       
        uint8_t iy = third_byte(shared_struct.ray_y);        
        int24_t frac = shared_struct.ray_x & 0xFFFF;
        shared_struct.dydx = short_tan_table[full_angle - 512];
        shared_struct.dxdy = short_cot_table[full_angle - 512];
        shared_struct.ray_y = shared_struct.ray_y + ((((0x10000 - frac) >> 5) * (shared_struct.dydx >> 5)) >> 6);
        shared_struct.ray_x = (shared_struct.ray_x & 0xFF0000) + 0x10000;
        
        if (iy == third_byte(shared_struct.ray_y))
        {
                octant_5_side();
        }
        else
        {
                octant_5_main();
        }
        
        /*if (iy == third_byte(shared_struct.ray_y))
        {
                goto oct5_minor_direction;  
        }                             
        
        while (1)
        {
                shared_struct.ray_x -= 0x10000;
                shared_struct.ray_y -= shared_struct.dydx;                
                if (*--map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
oct5_minor_direction:
                shared_struct.test_y = shared_struct.ray_y - shared_struct.dydx;               
                if (third_byte(shared_struct.test_y) != third_byte(shared_struct.ray_y))
                {                      
                        map_pointer -= MAP_WIDTH;
                        if (*map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_y & 0xFFFF;
                                shared_struct.ray_x = shared_struct.ray_x - (((frac >> 5) * shared_struct.dxdy) >> 6);
                                shared_struct.ray_y = shared_struct.ray_y & 0xFF0000;
                                return;
                        }
                }    
        }*/
}

extern void octant_2_side(void);
extern void octant_2_main(void);
void octant2(void) /* mostly down, some right*/
{        
        uint8_t ix = third_byte(shared_struct.ray_x);
        shared_struct.dxdy = short_tan_table[255 - full_angle];
        shared_struct.dydx = short_cot_table[255 - full_angle];
        shared_struct.ray_x = shared_struct.ray_x - ((((shared_struct.ray_y & 0xFFFF) >> 5) * (shared_struct.dxdy >> 5)) >> 6);
        shared_struct.ray_y = shared_struct.ray_y & 0xFF0000;
        
        if (ix == third_byte(shared_struct.ray_x))
        {
                octant_2_side();
        }
        else
        {
                octant_2_main();
        }
        
        /*if (ix == third_byte(shared_struct.ray_x))
        {
                shared_struct.test_x = shared_struct.ray_x + shared_struct.dxdy;
                if (third_byte(shared_struct.test_x) != third_byte(shared_struct.ray_x))
                {
                        if (*++map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_x & 0xFFFF;
                                shared_struct.ray_y += ((((0x10000 - frac) >> 5) * shared_struct.dydx) >> 6);
                                shared_struct.ray_x = (shared_struct.ray_x & 0xFF0000) + 0x10000;
                                return;
                        }                        
                }
        }
        
        while (1)        
        {
                shared_struct.ray_x += shared_struct.dxdy;
                shared_struct.ray_y += 0x10000;     
                map_pointer += MAP_WIDTH;
                if (*map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
                shared_struct.test_x = shared_struct.ray_x + shared_struct.dxdy;
                if (third_byte(shared_struct.test_x) != third_byte(shared_struct.ray_x))
                {
                        if (*++map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_x & 0xFFFF;
                                shared_struct.ray_y += ((((0x10000 - frac) >> 5) * shared_struct.dydx) >> 6);
                                shared_struct.ray_x = (shared_struct.ray_x & 0xFF0000) + 0x10000;
                                return;
                        }                        
                }                                   
        }*/
}

extern void octant_7_side(void);
extern void octant_7_main(void);
void octant7(void) /* mostly up, some right*/
{       
        uint8_t ix = third_byte(shared_struct.ray_x);
        int24_t frac = shared_struct.ray_y & 0xffff;
        shared_struct.dxdy = short_tan_table[full_angle - 768];
        shared_struct.dydx = short_cot_table[full_angle - 768];
        shared_struct.ray_x = shared_struct.ray_x - ((((0x10000 - frac) >> 5) * (shared_struct.dxdy >> 5)) >> 6);
        shared_struct.ray_y = (shared_struct.ray_y & 0xFF0000) + 0x10000;

        if (ix == third_byte(shared_struct.ray_x))
        {
                octant_7_side();
        }
        else
        {
                octant_7_main();
        }
        
        /*       
        if (ix == third_byte(shared_struct.ray_x))
        {
                goto oct7_minor_direction;
        }
        while (1)        
        {
                shared_struct.ray_x += shared_struct.dxdy;
                shared_struct.ray_y -= 0x10000;
                map_pointer -= MAP_WIDTH;
                if (*map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
oct7_minor_direction:
                shared_struct.test_x = shared_struct.ray_x + shared_struct.dxdy;
                if (third_byte(shared_struct.test_x) != third_byte(shared_struct.ray_x))
                {
                        if (*++map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_x & 0xFFFF;
                                shared_struct.ray_y -= ((((0x10000 - frac) >> 5) * shared_struct.dydx) >> 6);
                                shared_struct.ray_x = (shared_struct.ray_x & 0xFF0000) + 0x10000;
                                return;
                        }
                }                                   
        }*/
}

extern void octant_3_side(void);
extern void octant_3_main(void);
void octant3(void) /* mostly down, some left */
{        
        uint8_t ix = third_byte(shared_struct.ray_x);
        shared_struct.dxdy = short_tan_table[full_angle - 256];
        shared_struct.dydx = short_cot_table[full_angle - 256];
        shared_struct.ray_x = shared_struct.ray_x + ((((shared_struct.ray_y & 0xFFFF) >> 5) * (shared_struct.dxdy >> 5)) >> 6);
        shared_struct.ray_y = shared_struct.ray_y & 0xFF0000;
        
        if (ix == third_byte(shared_struct.ray_x))
        {
                octant_3_side();
        }
        else
        {
                octant_3_main();
        }
        
        /*if (ix == third_byte(shared_struct.ray_x))
        {
                shared_struct.test_x = shared_struct.ray_x - shared_struct.dxdy;
                if (third_byte(shared_struct.test_x) != third_byte(shared_struct.ray_x))
                {                        
                        if (*--map_pointer)
                        {
                                wall = *map_pointer;
                                shared_struct.ray_y = shared_struct.ray_y + (((((uint16_t)shared_struct.ray_x) >> 5) * shared_struct.dydx) >> 6);
                                shared_struct.ray_x = shared_struct.ray_x & 0xFF0000;
                                return;
                        }
                } 
        }
        
        while (1)        
        {
                shared_struct.ray_x -= shared_struct.dxdy;
                shared_struct.ray_y += 0x10000;
                map_pointer += MAP_WIDTH;
                if (*map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
                shared_struct.test_x = shared_struct.ray_x - shared_struct.dxdy;
                if (third_byte(shared_struct.test_x) != third_byte(shared_struct.ray_x))
                {                        
                        if (*--map_pointer)
                        {
                                wall = *map_pointer;
                                shared_struct.ray_y = shared_struct.ray_y + (((((uint16_t)shared_struct.ray_x) >> 5) * shared_struct.dydx) >> 6);
                                shared_struct.ray_x = shared_struct.ray_x & 0xFF0000;
                                return;
                        }
                }                                    
        }*/
}

extern void octant_6_side(void);
extern void octant_6_main(void);
void octant6(void) /* mostly up, some left */
{       
        uint8_t ix = third_byte(shared_struct.ray_x);
        int24_t frac = shared_struct.ray_y & 0xFFFF;
        shared_struct.dxdy = short_tan_table[767 - full_angle];
        shared_struct.dydx = short_cot_table[767 - full_angle];
        shared_struct.ray_x = shared_struct.ray_x + ((((0x10000 - frac) >> 5) * (shared_struct.dxdy >> 5)) >> 6);
        shared_struct.ray_y = (shared_struct.ray_y & 0xFF0000) + 0x10000;
        
        if (ix == third_byte(shared_struct.ray_x))
        {
                octant_6_side();
        }
        else
        {
                octant_6_main();
        }
        
        /*if (ix == third_byte(shared_struct.ray_x))
        {
                goto oct6_minor_direction;
        }
        
        while (1)        
        {
                shared_struct.ray_x -= shared_struct.dxdy;
                shared_struct.ray_y -= 0x10000;
                map_pointer -= MAP_WIDTH;
                if (*map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
oct6_minor_direction:
                shared_struct.test_x = shared_struct.ray_x - shared_struct.dxdy;
                if (third_byte(shared_struct.test_x) != third_byte(shared_struct.ray_x))
                {                        
                        if (*--map_pointer)
                        { 
                                wall = *map_pointer;
                                frac = shared_struct.ray_x & 0xFFFF;
                                shared_struct.ray_y = shared_struct.ray_y - (((frac >> 5) * shared_struct.dydx) >> 6);;
                                shared_struct.ray_x = shared_struct.ray_x & 0xFF0000;
                                return;
                        }
                }                             
        }*/
}

extern void octant_8_side(void);
extern void octant_8_main(void);
void octant8(void) /* mostly right, some up */
{        
        uint8_t iy = third_byte(shared_struct.ray_y);
        shared_struct.dydx = short_tan_table[1023 - full_angle];
        shared_struct.dxdy = short_cot_table[1023 - full_angle];
        shared_struct.ray_y = shared_struct.ray_y + ((((shared_struct.ray_x & 0xFFFF) >> 5) * (shared_struct.dydx >> 5)) >> 6);
        shared_struct.ray_x = shared_struct.ray_x & 0xFF0000;
        
        if (iy == third_byte(shared_struct.ray_y))
        {
                octant_8_side();
        }
        else
        {
                octant_8_main();
        }
        
        /*if (iy == third_byte(shared_struct.ray_y))
        {
                goto oct8_minor_direction;  
        }                             
        
        while (1)
        {
                shared_struct.ray_x += 0x10000;
                shared_struct.ray_y -= shared_struct.dydx;
                if (*++map_pointer)
                {
                        wall = *map_pointer;
                        return;
                }
oct8_minor_direction:
                shared_struct.test_y = shared_struct.ray_y - shared_struct.dydx;
                if (third_byte(shared_struct.test_y) != third_byte(shared_struct.ray_y))
                {
                        map_pointer -= MAP_WIDTH;
                        if (*map_pointer)
                        {
                                wall = *map_pointer;
                                frac = shared_struct.ray_y & 0xFFFF;
                                shared_struct.ray_x = shared_struct.ray_x + (((frac >> 5) * shared_struct.dxdy) >> 6);
                                shared_struct.ray_y = shared_struct.ray_y - frac;
                                return;
                        }
                }      
        }*/
}

void final_message(const char *msg)
{
        int count = 100000;
        draw_string(msg, (128 << 8) + 116); 
        draw_string("Press 5", (128 << 8) + 124);
        draw_string("to exit.", (128 << 8) + 132);
        do
        {
                *key_control = 2;
                while (*key_control != 0);
        }
        while (count-- && !((kb_Data[kb_Key5 >> 8] & (kb_Key5 & 255))));
}

/* Movement check routines (prevent walking through walls */

#define MIN_DISTANCE 0x2800

int check_position(int new_x, int new_y)
{
        int test_x, test_y;
        for (test_x = new_x - MIN_DISTANCE; test_x <= new_x + MIN_DISTANCE; test_x += 2 * MIN_DISTANCE)
                for (test_y = new_y - MIN_DISTANCE; test_y <= new_y + MIN_DISTANCE; test_y += 2 * MIN_DISTANCE)
                        if (map[third_byte(test_y)][third_byte(test_x)])
                                return 0;
        return 1;
}

int move_player(int direction)
{
        int new_x = player_x + (fixed_cosine(angle) >> 2) * direction;
        int new_y = player_y + (fixed_sine(angle) >> 2) * direction;        
        
        if (check_position(new_x, new_y))
        {
                player_x = new_x;
                player_y = new_y;
        }
        else 
        {
                int xonly = check_position(new_x, player_y);
                int yonly = check_position(player_x, new_y);
                if (xonly && !yonly)
                {
                        player_x = new_x;
                }
                else if (yonly && !xonly)
                {
                        player_y = new_y;
                }
                else
                {
                        return 0;
                }
        }
        
        shared_struct.test_x = player_x + (fixed_cosine(angle) >> 1) * direction;
        shared_struct.test_y = player_y + (fixed_sine(angle) >> 1) * direction;

        wall = map[third_byte(shared_struct.test_y)][third_byte(shared_struct.test_x)];

        if (wall == 4)
        {                
                final_message("You Win!");
                return 1;
        }
        if (wall == 5)
        {
                int x;
                map[third_byte(shared_struct.test_y)][third_byte(shared_struct.test_x)] = 3;
                for (x = 0; x < 20; x++)
                {
                        if (radiation >= 0x10000)
                        {
                                memset((char *)(0xd40000 + 160 * (240 - third_byte(radiation)) + 128), 0, 32);
                                radiation -= 0x10000;
                        }
                }
        }
        return 0;
}

typedef void (*octant)();
octant octants[8] = { &octant1, &octant2, &octant3, &octant4, &octant5, &octant6, &octant7, &octant8 };
        
void render_section(uint8_t start_col, uint8_t end_col)
{
        uint8_t col;
        uint8_t octant_num;
        uint24_t dist;
        int24_t texture_delta, dx, dy;
        octant octant_fn;
        unsigned char *col_start = (unsigned char *)(0xd40000 + start_col);
        int24_t local_min_angle = sprite_min_angle;
        int24_t local_max_angle = sprite_max_angle;
        int24_t sprite_image_x = 0;
        
        full_angle = (start_col + angle - 63) & 0x3FF;
        octant_num = (full_angle >> 7) & 7;        
        octant_fn = octants[octant_num];
        
        if (octant_num < 4 && local_max_angle >= 1024)
        {
                /* Handle the case of a sprite which starts a later octant and wraps around. */
                local_min_angle -= 1024;
                local_max_angle -= 1024;
        }
        
        if (local_min_angle < full_angle)
        {
                /* Calculate starting point in sprite when already partially into it. */
                sprite_image_x = sprite_image_dx * (full_angle - local_min_angle);
        }
        
        for (col = start_col; col <= end_col; col++)
        {
                /* Graphics loop step 1 - calculate ray end coordinate */           
                        
                int height;                                                                
                
                shared_struct.ray_x = player_x;
                shared_struct.ray_y = player_y;
                
                /* octant layout
                
                         \ 6  | 7  /             
                          \   |   /              
                           \  |  /               
                        5   \ | /   8            
                             \|/                 
                --------------+--------------
                             /|\                 
                        4   / | \   1            
                           /  |  \               
                          /   |   \              
                         / 3  | 2  \  */
                
                octant_fn();

                dx = (shared_struct.ray_x - player_x) >> 10;
                dy = (shared_struct.ray_y - player_y) >> 10;
                dist = square_root_fast(dx * dx + dy * dy);
                if (dist == 0) dist = 1;
                height = secants[col] / dist;                    
                
                /* Graphics loop step 2 - calculate texture (middle bits of coordinate for column within texture) */
                
                wall_x = (second_byte(shared_struct.ray_x) + second_byte(shared_struct.ray_y)) >> 2;                
                texture_delta = 8192 / height;
                
                /* Graphics loop step 3 - render texture, along with sprite if present */                
                
                if (full_angle >= local_min_angle && full_angle <= local_max_angle)
                {                        
                        if (dist > sprite_distance)
                        {
                                uint8_t *draw_sprite_image = sprite_start + 64 * third_byte(sprite_image_x);
                                if (height >= 120)
                                {
                                        texture_start_row = (32 << 8) - (120 * texture_delta);
                                        draw_column_tall_buffer(walls + 64 * wall_x + (wall - 1) * 4096 + (texture_start_row >> 8), 8192 / height, texture_start_row & 0xFF);
                                }
                                else
                                {
                                        draw_column_short_buffer(walls + 64 * wall_x + (wall - 1) * 4096, height * 2, 120 - height, texture_delta);
                                }
                                draw_sprite(
                                        draw_sprite_image,
                                        sprite_texture_delta,
                                        sprite_height);
                                        
                                copy_buffer(col_start);
                        }
                        else
                        {
                                if (height >= 120)
                                {
                                        texture_start_row = (32 << 8) - (120 * texture_delta);
                                        draw_column_tall(col_start, walls + 64 * wall_x + (wall - 1) * 4096 + (texture_start_row >> 8), 8192 / height, texture_start_row & 0xFF);
                                }
                                else
                                {
                                        draw_column_short(col_start, walls + 64 * wall_x + (wall - 1) * 4096, height * 2, 120 - height, texture_delta);
                                }
                        }
                        sprite_image_x += sprite_image_dx;
                }
                else
                {
                        if (height >= 120)
                        {
                                texture_start_row = (32 << 8) - (120 * texture_delta);
                                draw_column_tall(col_start, walls + 64 * wall_x + (wall - 1) * 4096 + (texture_start_row >> 8), 8192 / height, texture_start_row & 0xFF);
                        }
                        else
                        {
                                draw_column_short(col_start, walls + 64 * wall_x + (wall - 1) * 4096, height * 2, 120 - height, texture_delta);
                        }
                }
                
                col_start++;
                full_angle++;
        }                
}

void render(void)
{
        /* calculate last column on the same octant as the first */
        uint8_t final_col = 0x7F - ((angle - 0x3F) & 0x7F);
        
        /* calculate sprite position */
        
        int24_t dx = sprite_x - player_x; 
        int24_t dy = sprite_y - player_y;
        sprite_angle = 0x900;
        if (dx >= 0)
        {
                if (dy > 0)
                {
                        if (dx > dy)
                        {
                                int24_t slope = dy / (dx >> 8);
                                sprite_angle = arctan_table[slope];
                        }
                        else
                        {
                                int24_t slope = dx / (dy >> 8);
                                sprite_angle = 0x100 - arctan_table[slope];
                        }
                }
                else
                {
                        dy = -dy;
                        if (dx > dy)
                        {
                                int24_t slope = dy / (dx >> 8);
                                sprite_angle = 0x400 - arctan_table[slope];
                        }
                        else
                        {
                                int24_t slope = dx / (dy >> 8);
                                sprite_angle = 0x300 + arctan_table[slope];
                        }
                }
        }     
        else
        {
                dx = -dx;
                if (dy > 0)
                {
                        if (dx > dy)
                        {
                                int24_t slope = dy / (dx >> 8);
                                sprite_angle = 0x200 - arctan_table[slope];
                        }
                        else
                        {
                                int24_t slope = dx / (dy >> 8);
                                sprite_angle = 0x100 + arctan_table[slope];
                        }
                }
                else
                {
                        dy = -dy;
                        if (dx > dy)
                        {
                                int24_t slope = dy / (dx >> 8);
                                sprite_angle = 0x200 + arctan_table[slope];
                        }
                        else
                        {
                                int24_t slope = dx / (dy >> 8);
                                sprite_angle = 0x300 - arctan_table[slope];
                        }
                }
                
        }
        
        dx >>= 10;
        dy >>= 10;
        sprite_distance = square_root_fast(dx * dx + dy * dy);
        sprite_height = secants[64] / sprite_distance;
        sprite_texture_delta = 8191 / sprite_height;
        if (sprite_height >= 120) sprite_height = 119;
        if (sprite_height == 0) sprite_height = 1;
        sprite_min_angle = sprite_angle - (sprite_height >> 1);
        sprite_max_angle = sprite_angle + (sprite_height >> 1);
        sprite_image_dx = (64 << 16) / sprite_height;
        
        if (sprite_min_angle < 0)
        {
                sprite_min_angle += 1024;
                sprite_max_angle += 1024;
        }

        sprite_start = (frame_counter & 14) ? walls + 5 * 4096 : walls + 5 * 4096 + 32;
        init_map_pointer = &map[third_byte(player_y)][third_byte(player_x)];
        
        if (final_col >= 126)
        {
                render_section(0, 126);
        }
        else
        {
                render_section(0, final_col);
                render_section(final_col + 1, 126);
        }
}

void main(void) 
{         
        uint24_t dist;
        int24_t dx, dy, ix, iy;
        
        asm("di");
        for (ix = 0; ix < MAP_WIDTH; ix++)
                for (iy = 0; iy < MAP_HEIGHT; iy++)
                        map[iy][ix] &= 0xF;
        
        walls = (uint8_t *) (((uint24_t)&walls_area[0x3F]) & 0xFFFFC0);
        memcpy(walls, walls_source, WALLS_SIZE);
        
        for (angle = 0; angle < 256; angle++)
        {
                full_sine_table[angle] = short_sine_table[angle];
                full_sine_table[511 - angle] = short_sine_table[angle];
                full_sine_table[512 + angle] = -short_sine_table[angle];
                full_sine_table[1023 - angle] = -short_sine_table[angle];
        }
        for (angle = 0; angle < 1023; angle++)
        {
                secant_table[angle] = 8388600 / (full_sine_table[angle] >> 6);
        }
        
        *DISPLAY_CONTROL = DISPLAY_16COLOR;
        memcpy((char *)0xE30200, palette, 32);
        
        memset((char *)0xD40000, 0, 160 * 240);
        draw_string("Cellar3D", 128 << 8);
        draw_string("Preview", (130 << 8) + 10);

        draw_string("by", (140 << 8) + 30);
        draw_string("Patrick", (130 << 8) + 40);
        draw_string("Davidson", (128 << 8) + 50);
        
        angle = 0x3f;

        while (1)
        {
                old_radiation = third_byte(radiation);
                frame_counter++;
                
                if ((first_byte(frame_counter) & 15) == 0) sprite_direction++;
                switch (sprite_direction & 3)
                {
                        case 0:
                                sprite_x += 8192;
                                break;
                        case 1:
                                sprite_y += 8192;
                                break;
                        case 2:
                                sprite_x -= 8192;
                                break;
                        case 3:
                                sprite_y -= 8192;
                                break;
                }
                        
                render();
                if (sprite_distance < 100 && radiation < (239 << 16)) radiation = 239 << 16;
                
                *key_control = 2;
                while (*key_control != 0);
                if (kb_Data[kb_KeyClear >> 8] & (kb_KeyClear & 255)) break;
                if (kb_Data[kb_KeyLeft >> 8] & (kb_KeyLeft & 255)) angle -= 8;
                if (kb_Data[kb_KeyRight >> 8] & (kb_KeyRight & 255)) angle += 8;
                if (kb_Data[kb_KeyUp >> 8] & (kb_KeyUp & 255)) 
                        if (move_player(1))
                                break;
                if (kb_Data[kb_KeyDown >> 8] & (kb_KeyDown & 255))
                        if (move_player(-1))
                                break;
                if (kb_Data[kb_KeyPrgm >> 8] & (kb_KeyPrgm & 255))
                {
                        char buf[7];
                        sprintf(buf, "%06x", player_x);
                        draw_string(buf, (136 << 8) + 216);
                        sprintf(buf, "%06x", player_y);
                        draw_string(buf, (136 << 8) + 224);
                        sprintf(buf, "%03x", angle & 0x3FF);
                        draw_string(buf, (148 << 8) + 232);
                        sprintf(buf, "%06x", sprite_min_angle);
                        draw_string(buf, (136 << 8) + 200);
                        sprintf(buf, "%06x", sprite_max_angle);
                        draw_string(buf, (136 << 8) + 208);
                }
                if (kb_Data[kb_KeyTrace >> 8] & (kb_KeyTrace & 255))
                {
                        char buf[9];
                        int oldx = player_x;
                        int oldy = player_y;
                        int olda = angle;
                        
                        angle = 0x3f;
                        player_x = 0x38000;
                        player_y = 0x38000;
                        timer_control[0] &= 0xFA; /* disable, no interrupt */
                        timer_control[0] |= 2; /* count as 32768 Hz */
                        timer_control[1] |= 2; /* count up */
                        *timer1_counter = 0;
                        *timer_control |= 1; /* enable */
                        for (angle = 0; angle <= 0x3FF; angle++)
                                render();                                            
                        
                        sprintf(buf, "%08u", *timer1_counter);
                        draw_string(buf, (128 << 8) + 232);
                        
                        angle = olda;
                        player_x = oldx;
                        player_y = oldy;
                }
                
                dx = (0x70000 - player_x) >> 10;
                dy = (0x70000 - player_y) >> 10;
                dist = square_root_fast(dx * dx + dy * dy);
                radiation += 7000000 / dist;
                if (third_byte(radiation) > 240)
                {
                        final_message("You Lose");
                        break;
                }
                while (old_radiation < third_byte(radiation))
                {
                        old_radiation++;
                        memset((char *)(0xd40000 + 160 * (240 - old_radiation) + 128), radiation_colors[(240 - old_radiation) / 48], 32);
                }
        }        
        
        *DISPLAY_CONTROL = DISPLAY_NORMAL;
        prgm_CleanUp();
}