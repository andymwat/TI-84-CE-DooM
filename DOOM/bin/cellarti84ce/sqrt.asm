        .assume ADL=1
        
        XDEF    _square_root_fast
        XREF    _temp
        XREF    _square_root_table

; Square root table lookup algorithm : check if either of the top 2 bits is set.
; If so, look up using the top 10 bits in a 256-1023 table which is already shifted.
; If not, shift left 2 bits (i.e. multiply by 4) and try again, always shifting in 1s.
; Each additional cycle induces one right shift of the result. (i.e. divide by 2)
        
_square_root_fast:
        ld      hl,0
        add     hl,sp
        xor     a
        ld      (_temp+3),a     ; clear extra high byte
        inc     hl
        inc     hl
        inc     hl
        ld      hl,(hl)         ; HL = input value
        ld      (_temp),hl
        ld      b,1             ; shift count
        
square_root_search:
        ld      a,(_temp+2)     ; check high byte
        and     192
        jr      nz,square_root_found
        
        add     hl,hl           ; shift left with 1s coming in
        inc     hl
        add     hl,hl
        inc     hl
        ld      (_temp),hl
        inc     b               ; increment shift count
        jr      square_root_search
        
square_root_found:
        ld      hl,(_temp+1)    ; shifted left 8 bits
        add     hl,hl
        add     hl,hl           ; shifted left 6 bits
        ld      (_temp),hl
        ld      hl,(_temp+1)    ; shifted left 14 bits (only top 10 count)
        
        add     hl,hl
        ld      de,(_temp+1)
        add     hl,de           ; multiply by 3 (for table lookup)
        ld      de,_square_root_table-768
        add     hl,de           ; square root from table
        
        ld      hl,(hl)
        ex      de,hl
        ld      hl,_temp
        ld      (hl),de
        jr      square_root_shift_end

square_root_shift:
        inc     hl
        inc     hl
        sra     (hl)
        dec     hl
        rr      (hl)
        dec     hl
        rr      (hl)
square_root_shift_end:
        djnz    square_root_shift
        
        ld      hl,(hl)
        ret