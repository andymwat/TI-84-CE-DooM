        .assume ADL=1
        
XDEF    _draw_sprite
XDEF    _draw_column_tall_buffer
XDEF    _draw_column_short_buffer
XDEF    _copy_buffer

_draw_sprite:
        ld      iy,0
        add     iy,sp
        ld      hl,0d60088h     ; pointer to screen column start (halfway down)

        exx                     ; switch to texture information registers
        ld      de,(iy+3)       ; DE = texture pointer
        ld      h,e
        ld      l,0             ; HL = texture position counter (8 bits fractional part)
        ld      bc,(iy+6)       ; BC = texture position delta
        exx                     ; switch to screen information registers

        ld      b,(iy+9)
sprite_loop:
        exx                     ; switch to texture information registers
        add     hl,bc           ; advance within texture
        ld      e,h             ; DE = update texture pointer
        ld      a,(de)          ; read byte from texture
        exx                     ; switch to screen information registers
        or      a
        jr      z,masked_out
        ld      (hl),a
masked_out:
        inc     hl
        djnz    sprite_loop
        
        ret
        
_copy_buffer:
        ld      iy,0
        add     iy,sp
        ld      hl,(iy+3)       ; pointer to screen column start      
        ld      de,0d60010h
        ld      bc,160          
        
loop_copy_buffer:
        ld      a,(de)
        inc     de
        ld      (hl),a
        add     hl,bc
        ld      a,e
        or      a               ; after copying 160 bytes the low addr byte is zero
        jr      nz,loop_copy_buffer
        ret
        
_draw_column_tall_buffer:        
        ld      iy,0
        add     iy,sp

        ld      hl,0d60010h     ; pointer to screen column start (first arg at lowest address)
        
        exx                     ; switch to texture information registers
        ld      de,(iy+3)       ; DE = texture pointer
        ld      h,e
        ld      l,(iy+9)        ; HL = texture position counter (8 bits fractional part)
        ld      bc,(iy+6)       ; BC = texture position delta
        exx                     ; switch to screen information registers

        ld      b,240
draw_column_tall_loop:
        exx                     ; switch to texture information registers
        add     hl,bc           ; advance within texture
        ld      e,h             ; DE = update texture pointer
        ld      a,(de)          ; read byte from texture
        exx                     ; switch to screen information registers
        ld      (hl),a
        inc     hl
        djnz    draw_column_tall_loop
     
        ret
        
_draw_column_short_buffer:
        ld      iy,0
        add     iy,sp

        ld      hl,0d60010h      ; pointer to screen column start (first arg at lowest address)

        ld      a,34
        ld      b,(iy+9)
erase_1_loop:
        ld      (hl),a
        inc     hl
        djnz    erase_1_loop
        
        exx                     ; switch to texture information registers
        ld      de,(iy+3)       ; DE = texture pointer
        ld      h,e
        ld      l,0             ; HL = texture position counter (8 bits fractional part)
        ld      bc,(iy+12)      ; BC = texture position delta
        exx                     ; switch to screen information registers

        ld      b,(iy+6)
draw_column_short_loop:
        exx                     ; switch to texture information registers
        add     hl,bc           ; advance within texture
        ld      e,h             ; DE = update texture pointer
        ld      a,(de)          ; read byte from texture
        exx                     ; switch to screen information registers
        ld      (hl),a
        inc     hl
        djnz    draw_column_short_loop
        
        ld      a,17
        ld      b,(iy+9)
erase_2_loop:
        ld      (hl),a
        inc     hl
        djnz    erase_2_loop    
        ret