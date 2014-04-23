;/***************************************************************************
; *   TEKASM.ASM  - assembly code for Tekwar game                           *
; *                                                                         *
; *                                                      3/29/95 Jeff S.    *
; ***************************************************************************/
					 
; WATCOM C/C++ Assembler (WASM.EXE) Template.
; Written by Keith Harrison CIS:100431,1675 16th Nov 1994.

				  
                .586p     ; 386, 486, Pentium

                ; Declare non-parameter registers that you alter as
                ; a #pragma in the C++ file.

                ASSUME  CS:_TEXT
                ASSUME  DS:_DATA


_DATA           SEGMENT 'DATA' USE32

return_code     dd      0
integer_data    dd      0

_DATA           ENDS


_TEXT           SEGMENT PUBLIC 'CODE' USE32

;
; (template to follow)
;
;     On entry: eax = pointer to integer number,
;               ebx = value to store at address eax.
;
;asm_main_       PROC    NEAR
;                
;                call    protected_code  ; Call your own code...
;                cld                     ; Direction bit must be clear on exit.
;                mov     eax, [return_code]; EAX = return value (if required).
;                ret
;
;asm_main_       ENDP

                PUBLIC  asmwaitvrt_      ; Note underscore AFTER function name.

asmwaitvrt_     PROC    NEAR
					 
@@wait:
                xor     edx,edx
                xor     eax,eax
                xor     dx, 0x03DA

					 ; ecx loaded via param

					 ; wait for a display signal to make sure the raster isn't 
					 ; in the middle of a sync
@@waitnosync:
                in      al,dx
                test    al,8
                jnz     @@waitnosync

@@waitsync:
                in      al,dx
                test    al,8
                jz      @@waitsync

                loop    @@waitnosync

                cld                      ; Direction bit must be clear on exit.
                ret

asmwaitvrt_     ENDP

                PUBLIC  asmsetpalette_

asmsetpalette_  PROC    NEAR
					 
					 ; pal ptr passed in esi
                xor     eax, eax
                mov     edx,0x000003C8   ; PEL_WRITE_ADR
                out     dx,al
                mov     edx,0x000003C9   ; PEL_DATA

                mov     cx,256
@@setloop:
                lodsb
                out     dx,al
                lodsb
                out     dx,al
                lodsb
                out     dx,al
                loop    @@setloop

                cld
                ret

asmsetpalette_  ENDP

_TEXT           ENDS

                END
