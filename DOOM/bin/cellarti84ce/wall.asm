        .assume ADL=1
        
        XDEF    _draw_column_tall
        XDEF    _draw_column_short
        XDEF    _walls_source
        
_draw_column_tall:        
        ld      iy,0
        add     iy,sp

        ld      de,160
        ld      hl,(iy+3)       ; pointer to screen column start (first arg at lowest address)
        
        exx                     ; switch to texture information registers
        ld      de,(iy+6)       ; DE = texture pointer
        ld      h,e
        ld      l,(iy+12)       ; HL = texture position counter (8 bits fractional part)
        ld      bc,(iy+9)       ; BC = texture position delta
        exx                     ; switch to screen information registers

        ld      b,240
draw_column_tall_loop:
        exx                     ; switch to texture information registers
        add     hl,bc           ; advance within texture
        ld      e,h             ; DE = update texture pointer
        ld      a,(de)          ; read byte from texture
        exx                     ; switch to screen information registers
        ld      (hl),a
        add     hl,de
        djnz    draw_column_tall_loop
     
        ret
        
_draw_column_short:
        ld      iy,0
        add     iy,sp

        ld      de,160
        ld      hl,(iy+3)       ; pointer to screen column start (first arg at lowest address)

        ld      a,34
        ld      b,(iy+12)
erase_1_loop:
        ld      (hl),a
        add     hl,de
        djnz    erase_1_loop
        
        exx                     ; switch to texture information registers
        ld      de,(iy+6)       ; DE = texture pointer
        ld      h,e
        ld      l,0             ; HL = texture position counter (8 bits fractional part)
        ld      bc,(iy+15)      ; BC = texture position delta
        exx                     ; switch to screen information registers

        ld      b,(iy+9)
draw_column_short_loop:
        exx                     ; switch to texture information registers
        add     hl,bc           ; advance within texture
        ld      e,h             ; DE = update texture pointer
        ld      a,(de)          ; read byte from texture
        exx                     ; switch to screen information registers
        ld      (hl),a
        add     hl,de
        djnz    draw_column_short_loop
        
        ld      a,17
        ld      b,(iy+12)
erase_2_loop:
        ld      (hl),a
        add     hl,de
        djnz    erase_2_loop    
        ret
        
_walls_source:
        .include walls.inc
