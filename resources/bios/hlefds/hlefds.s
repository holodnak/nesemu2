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

.ORG $0100
delay132:
	hlecall i_delay132
	rts

.ORG $0149
	jmp	delay132

.ORG $0153
	hlecall i_delayms
	rts

.ORG $0161
	hlecall2	i_dispfobj
	rts

.ORG $016B
	hlecall2	i_enpfobj
	rts

.ORG $0171
	hlecall2	i_disobj
	rts

.ORG $0178
	hlecall2	i_enobj
	rts

.ORG $017E
	hlecall2	i_dispf
	rts

.ORG $0185
	hlecall2	i_enpf
	rts

.ORG $018B
nmi:
	hlecall	i_nmi
	rts

.ORG $01B2
	hlecall	i_vintwait
forever:
	bne	forever

.ORG $01C7
irq:
	hlecall	i_irq
	jmp $4222

.ORG $01F8
	hlecall	i_loadfiles
	rts

.ORG $0237
	lda	#$FF
	hlecall	i_writefile
	rts

.ORG $07BB
	hlecall	i_vramstructwrite
	rts

.ORG $0844
	hlecall	i_fetchdirectptr
	rts

.ORG $086A
	hlecall	i_writevrambuffer
	rts

.ORG $08B3
	hlecall	i_readvrambuffer
	rts

.ORG $08D2
	hlecall	i_preparevramstring
	rts

.ORG $08E1
	hlecall	i_preparevramstrings
	rts

.ORG $094F
	hlecall	i_getvrambufferbyte
	rts

.ORG $097D
	hlecall	i_pixel2nam
	rts

.ORG $0997
	hlecall	i_nam2pixel
	rts

.ORG $09B1
	hlecall	i_random
	rts

.ORG $09C8
	hlecall	i_spritedma	
	rts

.ORG $09D3
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
	hlecall	i_vramfill
	rts

.ORG $0AD2
	hlecall	i_memfill
	rts

.ORG $0AEA
	hlecall	i_setscroll
	rts

.ORG $0AFD
	hlecall	i_jumpengine
	rts

.ORG $0B13
	hlecall	i_readkeyboard
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

.ORG $0C22
	hlecall	i_unk_ec22
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
