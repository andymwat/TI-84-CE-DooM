        .assume ADL=1

        XDEF    _octant_1_main,_octant_1_side
        XDEF    _octant_2_main,_octant_2_side
        XDEF    _octant_3_main,_octant_3_side
        XDEF    _octant_4_main,_octant_4_side
        XDEF    _octant_5_main,_octant_5_side
        XDEF    _octant_6_main,_octant_6_side
        XDEF    _octant_7_main,_octant_7_side
        XDEF    _octant_8_main,_octant_8_side
        XREF    _shared_struct,_map,_wall,__imulu,_init_map_pointer
        
        XDEF	__indcallhl
__indcallhl:
	jp	(hl)
        
_ray_x  equ     _shared_struct
_ray_y  equ     _shared_struct+3
_dxdy   equ     _shared_struct+6
_dydx   equ     _shared_struct+9
_test_x equ     _shared_struct+12
_test_y equ     _shared_struct+15

ray_x  equ     0
ray_y  equ     3
dxdy   equ     6
dydx   equ     9
test_x equ     12
test_y equ     15

MAP_WIDTH       equ     16
        
; octant layout
;
;    \     |     /
;     \ 6  | 7  /             
;      \   |   /              
;       \  |  /               
;    5   \ | /   8            
;         \|/                 
;----------+-----------
;         /|\                 
;    4   / | \   1            
;       /  |  \               
;      /   |   \       
;     / 3  | 2  \
;    /     |     \
 
_octant_1_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_1_side

_octant_1_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 1: mainly going right (X increasing), also down (Y increasing)
octant_1_main:
        ld      hl,(iy+ray_y)           ; ray_y += dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        add     hl,bc
octant_1_main_hl:
        ld      (iy+ray_y),hl
        inc     (iy+ray_x+2)            ; ray_x += 0x10000, main direction, advance to next vertical line
        
        inc     de                      ; also adjust map pointer for one block X
        ld      a,(de)
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a horizontal line before the next vertical line
octant_1_side:
        ld      hl,(iy+ray_y)           ; test_y = ray_y + dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        add     hl,bc
        ld      (iy+test_y),hl
        ld      a,(iy+ray_y+2)
        cp      (iy+test_y+2)           ; continue if ipart(test_y) != ipart(ray_y)
        jr      z,octant_1_main_hl
        
        ld      hl,MAP_WIDTH
        add     hl,de
        ld      a,(hl)
        ex      de,hl                   ; advance wall pointer by one row
        or      a
        jr      z,octant_1_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type
        ld      hl,(iy+ray_y)
        ld      de,0
        ld      d,h
        ld      e,l                     ; DE = fpart(ray_y)
        ld      hl,10000h
        xor     a
        sbc     hl,de                   ; HL = 0x10000 - fpart(ray_y)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_y)) >> 5
        ld      bc,(iy+dxdy)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dx/dy
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      hl,(iy+test_x+1)
        
        ld      de,(iy+ray_x)
        add     hl,de
        ld      (iy+ray_x),hl           ; point of intercept with horizontal line
        
        lea     hl,iy+ray_y             ; Y coordinate is following horizontal line
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        inc     hl
        ld      a,(iy+test_y+2)
        ld      (hl),a
        ret
        
_octant_8_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_8_side

_octant_8_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 8: mainly going right (X increasing), also up (Y decreasing)
octant_8_main:        
        ld      hl,(iy+ray_y)           ; ray_y -= dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        xor     a
        sbc     hl,bc
octant_8_main_hl:
        ld      (iy+ray_y),hl
        inc     (iy+ray_x+2)            ; ray_x += 0x10000, main direction, advance to next vertical line
        
        inc     de                      ; also adjust map pointer for one block X
        ld      a,(de)
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a horizontal line before the next vertical line
octant_8_side:
        ld      hl,(iy+ray_y)           ; test_y = ray_y - dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        xor     a
        sbc     hl,bc
        ld      (iy+test_y),hl
        ld      a,(iy+ray_y+2)
        cp      (iy+test_y+2)           ; continue if ipart(test_y) != ipart(ray_y)
        jr      z,octant_8_main_hl
        
        ld      hl,-MAP_WIDTH
        add     hl,de
        ld      a,(hl)
        ex      de,hl                   ; advance wall pointer by one row
        or      a
        jr      z,octant_8_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type
        ld      de,(iy+ray_y)
        ld      hl,0
        ld      h,d
        ld      l,e                     ; HL = fpart(ray_y)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_y)) >> 5
        ld      bc,(iy+dxdy)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dx/dy
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      hl,(iy+test_x+1)
        
        ld      de,(iy+ray_x)
        add     hl,de
        ld      (iy+ray_x),hl           ; point of intercept with horizontal line
        
        lea     hl,iy+ray_y             ; Y coordinate is following horizontal line
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        ret

_octant_2_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_2_side

_octant_2_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 2: mainly going down (Y increasing), also right (X increasing)
octant_2_main:        
        ld      hl,(iy+ray_x)           ; ray_x += dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        add     hl,bc
octant_2_main_hl:
        ld      (iy+ray_x),hl
        inc     (iy+ray_y+2)            ; ray_y += 0x10000, main direction, advance to next horizontal line crossing
        
        ld      hl,MAP_WIDTH
        add     hl,de
        ld      a,(hl)
        ex      de,hl                   ; advance wall pointer by one row
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a vertical line before the next horizontal line
octant_2_side:
        ld      hl,(iy+ray_x)           ; test_x = ray_x + dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        add     hl,bc
        ld      (iy+test_x),hl
        ld      a,(iy+ray_x+2)
        cp      (iy+test_x+2)           ; continue if ipart(test_x) != ipart(ray_x)
        jr      z,octant_2_main_hl
        
        inc     de                      ; advance wall pointer by one column
        ld      a,(de)
        or      a
        jr      z,octant_2_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type        
        ld      hl,(iy+ray_x)
        ld      de,0
        ld      d,h
        ld      e,l                     ; DE = fpart(ray_x)
        ld      hl,10000h
        xor     a
        sbc     hl,de                   ; HL = 0x10000 - fpart(ray_x)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_x)) >> 5
        ld      bc,(iy+dydx)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dy/dx
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      hl,(iy+test_x+1)
        
        ld      de,(iy+ray_y)
        add     hl,de
        ld      (iy+ray_y),hl           ; point of intercept with vertical line
        
        lea     hl,iy+ray_x             ; X coordinate of following vertical line
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        inc     hl
        inc     (hl)
        ret
        
_octant_7_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_7_side

_octant_7_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 7: mainly going up (Y increasing), also right (X increasing)
octant_7_main:        
        ld      hl,(iy+ray_x)           ; ray_x += dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        add     hl,bc
octant_7_main_hl:
        ld      (iy+ray_x),hl
        dec     (iy+ray_y+2)            ; ray_y -= 0x10000, main direction, advance to next horizontal line crossing
        
        ld      hl,-MAP_WIDTH
        add     hl,de
        ld      a,(hl)
        ex      de,hl                   ; advance wall pointer by one row
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a vertical line before the next horizontal line
octant_7_side:
        ld      hl,(iy+ray_x)           ; test_x = ray_x + dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        add     hl,bc
        ld      (iy+test_x),hl
        ld      a,(iy+ray_x+2)
        cp      (iy+test_x+2)           ; continue if ipart(test_x) != ipart(ray_x)
        jr      z,octant_7_main_hl
        
        inc     de                      ; advance wall pointer by one column
        ld      a,(de)
        or      a
        jr      z,octant_7_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type        
        ld      hl,(iy+ray_x)
        ld      de,0
        ld      d,h
        ld      e,l                     ; DE = fpart(ray_x)
        ld      hl,10000h
        xor     a
        sbc     hl,de                   ; HL = 0x10000 - fpart(ray_x)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_x)) >> 5
        ld      bc,(iy+dydx)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dy/dx
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      de,(iy+test_x+1)
        
        ld      hl,(iy+ray_y)
        sbc     hl,de
        ld      (iy+ray_y),hl           ; point of intercept with vertical line
        
        lea     hl,iy+ray_x             ; X coordinate of following vertical line
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        inc     hl
        inc     (hl)
        ret
        
_octant_3_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_3_side

_octant_3_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 3: mainly going down (Y increasing), also left (X decreasing)
octant_3_main:       
        xor     a
        ld      hl,(iy+ray_x)           ; ray_x -= dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        sbc     hl,bc
octant_3_main_hl:
        ld      (iy+ray_x),hl
        inc     (iy+ray_y+2)           ; ray_y += 0x10000, main direction, advance to next horizontal line crossing
        
        ld      hl,MAP_WIDTH
        add     hl,de
        ld      a,(hl)
        ex      de,hl                   ; advance wall pointer by one row
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a vertical line before the next horizontal line
octant_3_side:
        xor     a
        ld      hl,(iy+ray_x)           ; test_x = ray_x - dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        sbc     hl,bc
        ld      (iy+test_x),hl
        ld      a,(iy+ray_x+2)
        cp      (iy+test_x+2)           ; continue if ipart(test_x) != ipart(ray_x)
        jr      z,octant_3_main_hl
        
        dec     de                      ; advance wall pointer by one column
        ld      a,(de)
        or      a
        jr      z,octant_3_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type        
        ld      de,(iy+ray_x)
        ld      hl,0
        ld      h,d
        ld      l,e                     ; HL = fpart(ray_x)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_x)) >> 5
        ld      bc,(_dydx)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dy/dx
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      hl,(iy+test_x+1)
        
        ld      de,(iy+ray_y)
        add     hl,de
        ld      (iy+ray_y),hl           ; point of intercept with vertical line
        
        lea     hl,iy+ray_x             ; ray_x &= 0xFF0000
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        ret
        
_octant_6_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_6_side

_octant_6_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 6: mainly going up (Y decreasing), also left (X decreasing)
octant_6_main:       
        xor     a
        ld      hl,(iy+ray_x)           ; ray_x -= dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        sbc     hl,bc
octant_6_main_hl:
        ld      (iy+ray_x),hl
        dec     (iy+ray_y+2)            ; ray_y -= 0x10000, main direction, advance to next horizontal line crossing
        
        ld      hl,-MAP_WIDTH
        add     hl,de
        ld      a,(hl)
        ex      de,hl                   ; advance wall pointer by one row
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a vertical line before the next horizontal line
octant_6_side:
        xor     a
        ld      hl,(iy+ray_x)           ; test_x = ray_x - dx/dy
        ld      bc,(iy+dxdy)            ; secondary direction, advance fractional amount
        sbc     hl,bc
        ld      (iy+test_x),hl
        ld      a,(iy+ray_x+2)
        cp      (iy+test_x+2)           ; continue if ipart(test_x) != ipart(ray_x)
        jr      z,octant_6_main_hl
        
        dec     de                      ; advance wall pointer by one column
        ld      a,(de)
        or      a
        jr      z,octant_6_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type        
        ld      de,(iy+ray_x)
        ld      hl,0
        ld      h,d
        ld      l,e                     ; HL = fpart(ray_x)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_x)) >> 5
        ld      bc,(iy+dydx)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dy/dx
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      de,(iy+test_x+1)
        
        ld      hl,(iy+ray_y)
        sbc     hl,de
        ld      (iy+ray_y),hl           ; point of intercept with vertical line
        
        lea      hl,iy+ray_x            ; ray_x &= 0xFF0000
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        ret
        
_octant_4_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_4_side

_octant_4_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 4: mainly going left (X decreasing), also down (Y increasing)
octant_4_main:        
        ld      hl,(iy+ray_y)           ; ray_y += dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        add     hl,bc
octant_4_main_hl:
        ld      (iy+ray_y),hl
        dec     (iy+ray_x+2)            ; ray_x -= 0x10000, main direction, advance to next vertical line
        
        dec     de                      ; also adjust map pointer for one block X
        ld      a,(de)
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a horizontal line before the next vertical line
octant_4_side:
        ld      hl,(iy+ray_y)           ; test_y = ray_y + dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        add     hl,bc
        ld      (iy+test_y),hl
        ld      a,(iy+ray_y+2)
        cp      (iy+test_y+2)           ; continue if ipart(test_y) != ipart(ray_y)
        jr      z,octant_4_main_hl
        
        ld      hl,MAP_WIDTH            ; advance wall pointer by one row
        add     hl,de
        ld      a,(hl)
        ex      de,hl
        or      a
        jr      z,octant_4_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type
        ld      hl,(iy+ray_y)
        ld      de,0
        ld      d,h
        ld      e,l                     ; DE = fpart(ray_y)
        ld      hl,10000h
        xor     a
        sbc     hl,de                   ; HL = 0x10000 - fpart(ray_y)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_y)) >> 5
        ld      bc,(iy+dxdy)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dx/dy
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      de,(iy+test_x+1)
        
        ld      hl,(iy+ray_x)
        sbc     hl,de
        ld      (iy+ray_x),hl           ; point of intercept with horizontal line
        
        lea     hl,iy+ray_y             ; Y coordinate is following horizontal line
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        inc     hl
        inc     (hl)
        ret

_octant_5_side:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
        jr      octant_5_side

_octant_5_main:
        ld      iy,_shared_struct
        ld      de,(_init_map_pointer)
 
; Octant 5: mainly going left (X decreasing), also up (Y decreasing)
octant_5_main:        
        ld      hl,(iy+ray_y)           ; ray_y -= dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        xor     a
        sbc     hl,bc
octant_5_main_hl:
        ld      (iy+ray_y),hl
        dec     (iy+ray_x+2)            ; ray_x -= 0x10000, main direction, advance to next vertical line
        
        dec     de                      ; also adjust map pointer for one block X
        ld      a,(de)
        or      a
        ld      (_wall),a
        ret     nz                      ; exit if wall present

; test if we will cross a horizontal line before the next vertical line
octant_5_side:
        ld      hl,(iy+ray_y)           ; test_y = ray_y + dy/dx
        ld      bc,(iy+dydx)            ; secondary direction, advance fractional amount
        xor     a
        sbc     hl,bc
        ld      (iy+test_y),hl
        ld      a,(iy+ray_y+2)
        cp      (iy+test_y+2)           ; continue if ipart(test_y) != ipart(ray_y)
        jr      z,octant_5_main_hl
        
        ld      hl,-MAP_WIDTH           ; advance wall pointer by one row
        add     hl,de
        ld      a,(hl)
        ex      de,hl
        or      a
        jr      z,octant_5_main         ; continue if no wall present

        ld      (_wall),a               ; save wall type
        ld      de,(iy+ray_y)
        ld      hl,0
        ld      h,d
        ld      l,e                     ; HL = fpart(ray_y)
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l
        srl     h
        rr      l                       ; HL = (0x10000 - fpart(ray_y)) >> 5
        ld      bc,(iy+dxdy)
        call    __imulu                 ; HL = ((0x10000 - fpart(ray_y)) >> 5) * dx/dy
        
        xor     a
        add     hl,hl                   ; shift AHL left 2
        rla
        add     hl,hl
        rla
        ld      (iy+test_x),hl          ; shift right 8 bits (net shift right 6)
        ld      (iy+test_x+3),a
        ld      de,(iy+test_x+1)
        
        ld      hl,(iy+ray_x)
        sbc     hl,de
        ld      (iy+ray_x),hl            ; point of intercept with horizontal line
        
        lea     hl,iy+ray_y              ; Y coordinate is following horizontal line
        xor     a
        ld      (hl),a
        inc     hl
        ld      (hl),a
        ret