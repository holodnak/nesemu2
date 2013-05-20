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

.ORG $01C7
irq:
	hlecall	i_irq
	jmp $4222

.ORG $01F8
loadfiles:
	hlecall	i_loadfiles
	rts

.ORG $0237
appendfile:
	lda	#$FF
writefile:
	hlecall	i_writefile
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
readdownverifypads:
	hlecall	$14
	rts

.ORG $0A4C
readordownverifypads:
	hlecall	$15
	rts

.ORG $0A68
readdownexppads:
	hlecall	$16
	rts

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
	hlecall	i_reset
	jmp	($DFFC)

.ORG $1F00
	.dw $FFFF
	.dw $FFFF

.ORG $1FFA
	.dw nmi
	.dw reset
	.dw irq
