;»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
; HLE FDS BIOS 6502 implementation
;»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»

.MEMORYMAP
SLOTSIZE $2000
DEFAULTSLOT 0
SLOT 0 $E000
.ENDME

.ROMBANKMAP
BANKSTOTAL 1
BANKSIZE $2000
BANKS 1
.ENDRO

.BANK 0 SLOT 0

.include "nes.i"
.include "fds.i"
.include "hle.i"

.ORG $0000
	;ident string is 6 bytes, followed by the version (high, low)
	.db	"HLEFDS",0,1

.ORG $018B
nmi:
	hlecall	i_nmi
	rts

.ORG $01B2
vintwait:
	hlecall	i_vintwait
forever:
	bne	forever

;($DFFE): disk game IRQ vector    (if [$0101] = 11xxxxxxB)
.ORG $01C7
irq:
	bit $0101
	bmi game_irq
	bvc disk_byte_skip

;disk transfer routine ([$0101]	= 01xxxxxx)
	ldx $4031
	sta $4024
	pla
	pla
	pla
	txa
	rts

disk_byte_skip:
;disk byte skip	routine ([$0101] = 00nnnnnn; n is # of bytes to	skip)
;this is mainly	used when the CPU has to do some calculations while bytes
;read off the disk need to be discarded.
	pha
	lda $0101
	sec
	sbc #$01
	bcc end_irq
	sta $0101
	lda $4031
	pla
end_irq:
	rti

game_irq:
;[$0101] = 1Xxxxxxx
	bvc disk_irq
	jmp ($DFFE);	11xxxxxx

disk_irq:
;disk IRQ acknowledge routine ([$0101] = 10xxxxxx).
;don't know what this is used for, or why a delay is put here.
	pha
	lda $4030
;	jsr Delay131
	pla
	rti

.ORG $01F8
loadfiles:
	hlecall	i_loadfiles
	rts

.ORG $07BB
vramstructwrite:
	hlecall	i_vramstructwrite
	rts

.ORG $09B1
random:
	hlecall	i_random
	rts

.ORG $09C8
spritedma:
	hlecall	i_spritedma	
	rts

.ORG $09D3
counterlogic:
	hlecall	i_counterlogic
	rts

.ORG $09EB
readpads:
	hlecall	i_readpads
	rts

.ORG $0A0D
orpads:
	hlecall	i_orpads
	rts

.ORG $0A1A
readdownpads:
	jsr		readpads
	beq		downpads		;always branches

.ORG $0A1F
readordownpads:
	jsr		readpads
	jsr		orpads
downpads:
	hlecall	i_downpads
	rts

.ORG $0A36
	JSR readpads
EA39:
	LDY $F5
	LDA $F6
	PHA
	JSR readpads
	PLA
	CMP $F6
	BNE EA39
	CPY $F5
	BNE EA39
	BEQ downpads

	JSR readpads
	JSR orpads
EA52:
	LDY $F5
	LDA $F6
	PHA
	JSR readpads
	JSR orpads
	PLA
	CMP $F6
	BNE EA52
	CPY $F5
	BNE EA52
	BEQ downpads
	JSR readpads
	LDA $00
	STA $F7
	LDA $01
	STA $F8
	LDX #$03
EA75:
	LDA $F5,X
	TAY
	EOR $F1,X
	AND $F5,X
	STA $F5,X
	STY $F1,X
	DEX
	BPL EA75
	RTS

;.ORG $0A36
;readdownverifypads:
;	hlecall	$14
;	rts

;.ORG $0A4C
;readordownverifypads:
;	hlecall	$15
;	rts

;.ORG $0A68
;readdownexppads:
;	hlecall	$16
;	rts

.ORG $0A84
vramfill:
	hlecall	i_vramfill
	rts

.ORG $0AD2
memfill:
	hlecall	i_memfill
	rts

.ORG $0AEA
setscroll:
	hlecall	i_setscroll
	rts

.ORG $0B66
inc00by8:
	lda	#8
inc00bya:
	hlecall	i_inc00bya
	rts

.ORG $0BAF
	hlecall	i_loadtileset
	rts

;;reset vector
.ORG $0E24
reset:
	;;disable irq and clear decimal mode bit
	sei
	cld

	;;ppu reg init
	lda	#$10
	sta	PPUCONTROL
	sta	$FF

	lda	#$06
	sta	PPUMASK
	sta	$FE

	;;wait two frames
	ldx	#2
;vblankwait:
;	lda	PPUSTATUS
;	bpl	vblankwait
;	dex
;	bne	vblankwait
	
	;;setup fds bios register/fds registers
	stx	$4022.w
	stx	$4023.w
	stx	$FD
	stx	$FC
	stx	$FB
	stx	$4016.w

	lda	#$83
	sta	$4023

	lda	#$2E
	sta	$4025
	sta	$FA

	lda	$FF
	sta	$4026.w
	sta	$F9

	stx	$4010.w
	
	lda	#$C0
	sta	$4017

	lda	#$0F
	sta	$4015

	;init stack
	ldx	#$FF
	txs

	;initialize bios vars
	lda	#$35
	sta	$103

	lda	#$AC
	sta	$102

	lda	#$80
	sta	$101

	lda	#$C0
	sta	$100

	lda	#$80
	sta	$FF

	lda	#$06
	sta	$FE

	;force insert the disk
	hlecall	i_forceinsert

	;do boot (load boot files)
	lda	#0
	jsr	loadfiles
	.dw	$FF00,$FF00

	;enable interrupts
	cli

	;transfer control to the disk program
	jmp	($DFFC)

	;loop forever
loop:
	jmp	loop


.ORG $1F00
	.dw $FFFF
	.dw $FFFF

.ORG $1FFA
	.dw nmi
	.dw reset
	.dw irq
