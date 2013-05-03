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

.ORG $0000
	;ident string is 6 bytes, followed by the version (high, low)
	.db	"HLEFDS",0,1

.ORG $0E24

reset:
	;disable irq and clear decimal mode bit
	sei
	cld

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

	lda	#0
	sta 	$FD
	sta	$FC
	sta	$FB

	lda	#$2E
	sta	$FA

	lda	#$FF
	sta	$F9

	;here we can do an intro...
	jsr	bios_intro

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

.ORG $1800
bios_intro:
	rts

.ORG $1F00
	.dw $FFFF
	.dw $FFFF

.ORG $1FFA
	.dw nmi
	.dw reset
	.dw irq
