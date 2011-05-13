

;; Entry point from the bootloader ! we need to init everything but the usb variables

;; In large copy,paste from the microchip crt0/crt1 startup code. This file is then NOT under LGPLv3.


.text
.section .init,code
.global	__reset
.global __resetALT
__reset:
__resetALT:

	.weak    __user_init, __has_user_init

; Reset the stack
	goto 0x14800 			;; go to bootloader
	
	

	mov #__SP_init, w15
	mov #__SPLIM_init, w0
	mov w0, SPLIM
	nop
	
	rcall    __psv_init
	mov      #__dinit_tbloffset,w0 ; w0,w1 = template
    mov      #__dinit_tblpage,w1   ;
    rcall    __data_init_standard  ; initialize data
    
    mov      #__has_user_init,w0
    cp0      w0                ; user init functions?
    bra      eq,1f             ; br if not
    call     __user_init       ; else call them
1:
    call  _main                ; call user's main()
    
    reset                      ; reset the processor
	
__psv_init:
	.equiv   PSV, 0x0002
	bclr     _CORCON,#PSV        ; disable PSV (default)
    mov      #__const_length,w0  ;
	cp0      w0                  ; test length of constants
    bra      z,1f                ; br if zero

    mov      #__const_psvpage,w0 ;
    mov      w0,_PSVPAG          ; PSVPAG = psvpage(constants)
    bset     _CORCON,#PSV        ; enable PSV

1:  return                       ;  and exit

__data_init_standard:
	.equiv   FMT_CLEAR,0    ;  format codes
    .equiv   FMT_COPY2,1    ;
    .equiv   FMT_COPY3,2    ;
#define ZERO      w0
#define TLOFFSET  w1
#define DSTOFFSET w2
#define LEN       w3
#define UPPERBYTE w4
#define FORMAT    w5
#define PAGE      w6
#define REALLY_INIT w7

	mov #0x1000-1, w8
	mov #0x11c6-1, w9


    mov       w1,_TBLPAG
    mov       w0,w1
    clr       w0
    bra      4f

1:  inc2      w1,w1
    addc     _TBLPAG                         ; ZERO must be tied to 0

    tblrdl.w [w1],w3
    add      w1,#2,w1
    addc     _TBLPAG                         ; ZERO must be tied to 0

    tblrdl.w [w1],w5
    add      w1,#2,w1
    addc     _TBLPAG                         ; ZERO must be tied to 0

    clr      w4
    lsr      w5,#7,w6                  ; PAGE <- stored DSWPAG
    and      #0x7F,w5                    ; zero out upper bits
    cp.b     w5,#FMT_CLEAR
    bra nz,  2f

    ;; FMT_CLEAR - clear destination memory
9:  rcall do_init
	cp0		w7
	bra z, _else_clear
_yesdoit_clear:
	clr.b    [w2++]
	bra 	_endif_clear
_else_clear:
	inc 	w2,w2
_endif_clear:
    dec      w3,w3
    bra gtu, 9b                              ; loop if not done
    bra      4f

    ;; FMT_COPY2, FMT_COPY3
2:  cp       w5,#FMT_COPY2
    bra z,   3f

    setm     w4

    ;; standard memcpyd3
3:  rcall __memcpypd3_std

4:  tblrdl.w [w1],w2
	cp0      w2
	bra nz,  1b
	return
	
__memcpypd3_std:
1:
	rcall do_init
	cp0 w7
	bra z, _dnc1
   	 tblrdl.b [w1++],[w2++]  ; dst++ = lo byte
   	 bra _dnc1e
_dnc1:
	 inc w1,w1
	 inc w2,w2
_dnc1e:
    dec      w3,w3          ; num--
    bra      z,2f           ; br if done         ( add one )

	rcall do_init
	cp0 w7
	bra z, _dnc2
     tblrdl.b [w1--],[w2++]  ; dst++ = hi byte
     bra _dnc2e
_dnc2:
	 dec w1,w1
     inc w2,w2
_dnc2e:
    dec      w3,w3          ; num--
    bra      z,4f           ; br if done         ( add two )

    cp0      w4             ; test upper flag
    bra      nz,1f           ; br if false

3:  add      w1,#2,w1
    addc     _TBLPAG
    bra      1b

1:  rcall do_init
	cp0		w7
	bra 	z,_dnc3
		tblrdh.b [w1],[w2++]    ; dst++ = upper byte
		bra _dnc3e
_dnc3:
		inc w2,w2
_dnc3e:
    dec      w3,w3          ; num--
    bra      nz,3b          ; br if not done

4:  inc      w1,w1
2:  add      w1,#1,w1
    addc     _TBLPAG
    return                  ; exit
    


do_init:
	;w2 ptr
	;w7 return value
	; w8 min address of the hole
	; w9 max address of the hole

	cpsgt w2, w8 ; skip if w2 >= 0x1000
	bra _do_init_ok
	cpsgt w2, w9 ; skip if w2 >= end address
	bra _do_init_nok

_do_init_ok:
	setm w7
	return
_do_init_nok:
	clr w7
	return
	
