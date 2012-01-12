; All the write access to the leds array MUST be atomic
; otherwise some data corruption might happens.

; near pointer, allocated & initialized in leds.c
.global _leds_index

.section .text
.global __leds_set
.global __leds_clr
.global _leds_tick_cb


; void leds_tick_cb(void)
_leds_tick_cb:
	bclr LATC,#13			; LED_CS = 0;
	mov _led_index,w0		; w0 = led_index
	mov #SPI1BUF, w1		; w1 = &SPI1BUF
	mov.b [w0++],[w1]		; SPI1BUF = *led_index++
	mov.b [w0++],[w1]		;  	''
	mov.b [w0++],[w1]		; 	''
	mov.b [w0++],[w1]		; 	''
	mov.b [w0++],[w1]		; 	''

	mov #(_led + 32 * 5),w1	; w1 = &led[MAX_BRIGHTNESS * LED_BANK]
	cpsne w1,w0				; if(index == MAX_BRIGHTNESS * LED_BANK)
	mov #_led,w0			;	w0 = &led[0]

	mov w0, _led_index		; led_index = w0
	; The leds will be latched later in the interrupt
	return
	
; w0: pointer to leds array
; w1: count (0-32)
; w2: mask
__leds_set:
	
; 2 instructions to set one
	subr w1,#31,w1
	inc w1,w1
	sl w1,#1,w1
	bra w1

	ior.b w2,[w0],[w0]	;1
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;2
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;3
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;4
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;5
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;6
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;7
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;8
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;9
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;10
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;11
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;12
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;13
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;14
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;15
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;16
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;17
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;18
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;19
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;20
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;21
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;22
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;23
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;24
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;25
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;26
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;27
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;28
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;29
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;30
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;31
	add w0,#5,w0
	ior.b w2,[w0],[w0]	;32
	add w0,#5,w0
	
	return;
	
; w0: pointer to leds array
; w1: count (0-32)
; w2: mask
__leds_clr:
	
; 2 instructions to set one
	subr w1,#31,w1
	inc w1,w1
	sl w1,#1,w1
	bra w1

	and.b w2,[w0],[w0]	;1
	add w0,#5,w0
	and.b w2,[w0],[w0]	;2
	add w0,#5,w0
	and.b w2,[w0],[w0]	;3
	add w0,#5,w0
	and.b w2,[w0],[w0]	;4
	add w0,#5,w0
	and.b w2,[w0],[w0]	;5
	add w0,#5,w0
	and.b w2,[w0],[w0]	;6
	add w0,#5,w0
	and.b w2,[w0],[w0]	;7
	add w0,#5,w0
	and.b w2,[w0],[w0]	;8
	add w0,#5,w0
	and.b w2,[w0],[w0]	;9
	add w0,#5,w0
	and.b w2,[w0],[w0]	;10
	add w0,#5,w0
	and.b w2,[w0],[w0]	;11
	add w0,#5,w0
	and.b w2,[w0],[w0]	;12
	add w0,#5,w0
	and.b w2,[w0],[w0]	;13
	add w0,#5,w0
	and.b w2,[w0],[w0]	;14
	add w0,#5,w0
	and.b w2,[w0],[w0]	;15
	add w0,#5,w0
	and.b w2,[w0],[w0]	;16
	add w0,#5,w0
	and.b w2,[w0],[w0]	;17
	add w0,#5,w0
	and.b w2,[w0],[w0]	;18
	add w0,#5,w0
	and.b w2,[w0],[w0]	;19
	add w0,#5,w0
	and.b w2,[w0],[w0]	;20
	add w0,#5,w0
	and.b w2,[w0],[w0]	;21
	add w0,#5,w0
	and.b w2,[w0],[w0]	;22
	add w0,#5,w0
	and.b w2,[w0],[w0]	;23
	add w0,#5,w0
	and.b w2,[w0],[w0]	;24
	add w0,#5,w0
	and.b w2,[w0],[w0]	;25
	add w0,#5,w0
	and.b w2,[w0],[w0]	;26
	add w0,#5,w0
	and.b w2,[w0],[w0]	;27
	add w0,#5,w0
	and.b w2,[w0],[w0]	;28
	add w0,#5,w0
	and.b w2,[w0],[w0]	;29
	add w0,#5,w0
	and.b w2,[w0],[w0]	;30
	add w0,#5,w0
	and.b w2,[w0],[w0]	;31
	add w0,#5,w0
	and.b w2,[w0],[w0]	;32
	add w0,#5,w0
	
	return;

	
