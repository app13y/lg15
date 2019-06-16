.code

extern    bitmask:xmmword
extern    precomputedLSTable:xmmword

encryptBlockWithGost15  proc

initialising:
    movdqu         xmm0, [rcx]                        ; [unaligned] loading block
    lea            r8, [precomputedLSTable + 01000h]  ; [aligned] aliasing table base with offset
    lea            r11, [precomputedLSTable]          ; [aligned] aliasing table base
    movdqa         xmm4, [bitmask]                    ; [aligned] loading bitmask
    mov            r10d, 9                            ; number of rounds, base 0

round:
    pxor           xmm0, [rdx]                        ; [aligned] mixing with round key

    movaps         xmm1, xmm4                         ; securing bitmask from @xmm4
    movaps         xmm2, xmm4                         ; securing bitmask from @xmm4

    pand           xmm2, xmm0                         ; calculating offsets
    pandn          xmm1, xmm0
    psrlw          xmm2, 4
    psllw          xmm1, 4

    pextrw         eax, xmm1, 0h                      ; accumulating caches
    movdqa         xmm0, [r11 + rax + 00000h]
    pextrw         eax, xmm2, 0h
    movdqa         xmm3, [r8 + rax + 00000h]
    pextrw         eax, xmm1, 1h
    pxor           xmm0, [r11 + rax + 02000h]
    pextrw         eax, xmm2, 1h
    pxor           xmm3, [r8 + rax + 02000h]
    pextrw         eax, xmm1, 2h
    pxor           xmm0, [r11 + rax + 04000h]
    pextrw         eax, xmm2, 2h
    pxor           xmm3, [r8 + rax + 04000h]
    pextrw         eax, xmm1, 3h
    pxor           xmm0, [r11 + rax + 06000h]
    pextrw         eax, xmm2, 3h
    pxor           xmm3, [r8 + rax + 06000h]
    pextrw         eax, xmm1, 4h
    pxor           xmm0, [r11 + rax + 08000h]
    pextrw         eax, xmm2, 4h
    pxor           xmm3, [r8 + rax + 08000h]
    pextrw         eax, xmm1, 5h
    pxor           xmm0, [r11 + rax + 0a000h]
    pextrw         eax, xmm2, 5h
    pxor           xmm3, [r8 + rax + 0a000h]
    pextrw         eax, xmm1, 6h
    pxor           xmm0, [r11 + rax + 0c000h]
    pextrw         eax, xmm2, 6h
    pxor           xmm3, [r8 + rax + 0c000h]
    pextrw         eax, xmm1, 7h
    pxor           xmm0, [r11 + rax + 0e000h]
    pextrw         eax, xmm2, 7h
    pxor           xmm3, [r8 + rax + 0e000h]

    pxor           xmm0, xmm3                         ; mixing caches

    add            rdx, 10h                           ; advancing to next round key
    dec            r10                                ; decrementing round index
    jnz            round

last_round:
    pxor           xmm0, [rdx]                        ; [aligned] mixing with round key
    movdqu         [rcx], xmm0                        ; [unaligned] storing block

    ret

encryptBlockWithGost15  endp

end