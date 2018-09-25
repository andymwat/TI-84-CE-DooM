        .assume ADL=1

        XDEF    _draw_string
        XREF    _char_tiles

_draw_string:
        push    ix
        ld      ix,0
        add     ix,sp            
        ld      hl,(ix+6)
        ld      bc,(ix+9)
        pop     ix
        
draw_str_loop:
        ld      a,(hl)
        or      a
        ret     z
        inc     hl
        push    bc
        push    hl
to_draw_char:
        call    Draw_Char
        pop     hl
        pop     bc
        inc     b
        inc     b
        inc     b
        inc     b
        jr      draw_str_loop

;####### Display a single character

Draw_Char:
        sub     32
        add     a,a

        ld      de,0
        ld      e,a
        ld      hl,_char_tiles
        add     hl,de
        add     hl,de
        add     hl,de
        add     hl,de                   ; HL -> start of char
        push    hl
        
        ld      hl,0
        ld      e,c
        ld      l,c
        add     hl,hl
        add     hl,hl
        add     hl,de                   ; HL = 5 * Y
        add     hl,hl
        add     hl,hl
        add     hl,hl
        add     hl,hl
        add     hl,hl                   ; HL = 160 * Y
        ld      e,b
        add     hl,de                   ; HL = 160 * Y + X
        
        ld      de,0d40000h
        add     hl,de
        
        pop     de
        
        ld      c,8
outer_char_loop_16:
        ld      a,(de)
        inc     de
        ld      b,4
inner_char_loop_16:
        ld      (hl),0
        rlca
        jr      c,inner_16_start_clear
        ld      (hl),8
inner_16_start_clear:
        rlca
        jr      c,inner_16_second_clear
        set     7,(hl)
inner_16_second_clear:
        inc     hl
        djnz    inner_char_loop_16
        push    de
        ld      de,160-4
        add     hl,de
        pop     de
        dec     c
        jr      nz,outer_char_loop_16
        ret
        
_char_tiles:
        .include chars.inc