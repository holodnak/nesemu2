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
	.db	"HLEFDS",0,7

.ORG $0100
delay132:
	hle	i_delay132
	rts

.ORG $0149
	jmp	delay132

.ORG $0153
	hle	i_delayms
	rts

.ORG $0161
	hle2	i_dispfobj
	rts

.ORG $016B
	hle2	i_enpfobj
	rts

.ORG $0171
	hle2	i_disobj
	rts

.ORG $0178
	hle2	i_enobj
	rts

.ORG $017E
	hle2	i_dispf
	rts

.ORG $0185
	hle2	i_enpf
	rts

.ORG $018B
nmi:
	hle	i_nmi
	jmp $4280

.ORG $01B2
	hle	i_vintwait
forever:
	bne	forever

.ORG $01C7
irq:
	hle	i_irq
	jmp $4288

.ORG $01F8
	hle	i_loadfiles
	rts

.ORG $0237
	lda	#$FF
	hle	i_writefile
	rts

.ORG $03E7
	sec					;;no write protect check
	bcs	gethcparam	;;always branch
	clc					;;write protection check
gethcparam:
	hle	i_gethcparam
	rts

.ORG $0445
	hle	i_checkdiskheader
	rts

.ORG $0484
	hle	i_getnumfiles
	rts

.ORG $04A0
	hle	i_filematchtest
	rts

.ORG $04F9
	hle	i_loaddata
	rts

.ORG $068F
	hle	i_checkblocktype
	rts

.ORG $0778
	hle	i_xferdone
	rts

.ORG $07BB
	hle	i_vramstructwrite
	rts

.ORG $0844
	hle	i_fetchdirectptr
	rts

.ORG $086A
	hle	i_writevrambuffer
	rts

.ORG $08B3
	hle	i_readvrambuffer
	rts

.ORG $08D2
	hle	i_preparevramstring
	rts

.ORG $08E1
	hle	i_preparevramstrings
	rts

.ORG $094F
	hle	i_getvrambufferbyte
	rts

.ORG $097D
	hle	i_pixel2nam
	rts

.ORG $0997
	hle	i_nam2pixel
	rts

.ORG $09B1
	hle	i_random
	rts

.ORG $09C8
	hle	i_spritedma	
	rts

.ORG $09D3
	hle	i_counterlogic
	rts

.ORG $09EB
readpads:
	hle	i_readpads
	rts

.ORG $0A0D
orpads:
	hle	i_orpads
	rts

.ORG $0A1A
	jsr	readpads
	beq	downpads		;always branches

.ORG $0A1F
readordownpads:
	jsr	readpads
	jsr	orpads
downpads:
	hle	i_downpads
	rts

.ORG $0A36
	hle	i_readdownverifypads
	rts

.ORG $0A4C
	nop
	nop
	nop
	nop
	nop
	nop
	hle	i_readordownverifypads
	rts

.ORG $0A68
	hle	i_readdownexppads
	rts

.ORG $0A84
	hle	i_vramfill
	rts

.ORG $0AD2
	hle	i_memfill
	rts

.ORG $0AEA
	hle	i_setscroll
	rts

.ORG $0AFD
	hle	i_jumpengine
	jmp	$4290

.ORG $0B13
	hle	i_readkeyboard
	rts

.ORG $0BAF
	hle	i_loadtileset
	rts

.ORG $0C22
	hle	i_unk_ec22
	rts

;;reset vector
.ORG $0E24
reset:
	hle	i_reset
	jmp	($DFFC)

.ORG $1F00
	.dw $FFFF
	.dw $FFFF

.ORG $1FFA
	.dw nmi
	.dw reset
	.dw irq
