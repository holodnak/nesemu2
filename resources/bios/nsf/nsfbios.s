;»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
; NSF BIOS 6502 implementation
;»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»

.MEMORYMAP
	SLOTSIZE $400
	DEFAULTSLOT 0
	
	;;fixed bank
	SLOT 0 $3800

	;;swappable bank
	SLOT 1 $3C00

.ENDME

.ROMBANKMAP
	BANKSTOTAL 4
	BANKSIZE $400
	BANKS 4
.ENDRO

.define PPUCTRL				$2000
.define PPUMASK				$2001
.define PPUSTATUS				$2002
.define OAMADDR				$2003
.define OAMDATA				$2004
.define PPUSCROLL				$2005
.define PPUADDR				$2006
.define PPUDATA				$2007
.define NSFREG_BANK			$3400
.define NSFREG_DISASM		$3408
.define NSFREG_DEBUG			$3409

.macro swap
	lda	#\1
	sta	NSFREG_BANK
.endm

.macro showdisasm
	lda	#\1
	sta	NSFREG_DISASM
.endm

;;this slot is just for init code and vectors
.BANK 0 SLOT 0

.ORG $000
	;ident string is 7 bytes, followed by the version (high, low), then a 0
	.db	"NSFBIOS",0,1
	.db	0

.ORG $00A
	;;vectors stored at address $00A in the first bank
	.dw	nmi
	.dw	reset
	.dw	irq

;;wait for vblank, missing it sometimes
ppuwait:
	bit PPUSTATUS
	bpl ppuwait
	rts

zerotile:
	ldy	#16
	lda	#0
-	sta	PPUDATA
	dey
	bne	-
	rts

;;copy data to vram
copyvram:
	lda	($00),y		; copy one byte
	sta	PPUDATA
	iny
	bne	copyvram		; repeat until we finish the page
	inc	$01			; go to the next page
	dex
	bne	copyvram		; repeat until we've copied enough pages
	rts
	
;;copy palette data
copypalette:
	ldy	#$3F
	sty.w	PPUADDR		; load the destination address into the PPU
	ldy	#$00
	sty.w	PPUADDR
-	lda	($00),y		; copy one byte
	sta	PPUDATA
	iny
	dex
	bne	-	; repeat until we finish
	rts

;;clear vram/nametables
;;y = number of bytes
;;x = number of pages
clearvram:
	sta	PPUDATA
	dey
	bne	clearvram	; repeat until we finish the page
	dex
	bne	clearvram	; repeat until we've copied enough pages
	rts

copystring:
	ldy	#0
-	lda	($00),y
	iny
	cmp	#0
	beq	+
	sta	PPUDATA
	sta	NSFREG_DEBUG
	bne	-
+	rts

reset:

	sei					;;disable irq
	cld					;;disable decimal mode
	lda	#$40
	sta	$4017			;;disable frame irq
	ldx	#$FF
	txs					;;setup stack
	inx
	stx.w	PPUCTRL		;;disable nmi
	stx.w	PPUMASK		;;disable rendering
	stx.w	$4010			;;disable dmc irq
	lda	#0
	sta	PPUCTRL
	sta	PPUMASK

	;;let ppu warm up, wait for vblank
	jsr	ppuwait
	jsr	ppuwait

	ldy	PPUSTATUS	;;reset the toggle
	ldy	#$02			; starting index into the first page
	sty.w	PPUADDR		; load the destination address into the PPU
	ldy	#$00
	sty.w	PPUADDR

	;;setup source address
	lda	#$3C
	sta	$01
	lda	#$00
	sta	$00

	swap	2				;;map the first bank of chr to $3C00
	ldx	#4				; number of 256-byte pages to copy
	jsr	copyvram

	;;setup source address
	lda	#$3C
	sta	$01
	lda	#$00
	sta	$00

	swap	3				;;map the second bank of chr to $3C00
	ldx	#2				; number of 256-byte pages to copy
	jsr	copyvram

	;;now copy palette, it is after the chr
	ldx	#4
	jsr	copypalette

	;;setup tile 0
	ldy	#0				; starting index into the first page
	sty.w	PPUADDR		; load the destination address into the PPU
	sty.w	PPUADDR
	jsr	zerotile

	;;setup tile 0
	ldy	#$20			; starting index into the first page
	sty.w	PPUADDR		; load the destination address into the PPU
	ldy	#0				; starting index into the first page
	sty.w	PPUADDR
	ldx	#4
	jsr	clearvram

	swap	1				;;map in the unused bank

	ldy	#$20
	sty.w	PPUADDR
	ldy	#$42
	sty.w	PPUADDR
	lda	#>ntdata
	sta	$01
	lda	#<ntdata
	sta	$00
	jsr	copystring

	ldy	#0
	sty.w	PPUSCROLL
	ldy	#0
	sty.w	PPUSCROLL
	lda	#$1A
	sta	PPUMASK		;;enable rendering

loop:
	jmp	loop

nmi:
	jmp	nmi

irq:
	jmp	irq

ntdata:
	.db	"nesemu2 nsf player",0

;;misc data
.BANK 1 SLOT 1
.ORG $000

;;chr data
.BANK 2 SLOT 1
.ORG $000
.INCBIN "ascii.chr" READ 1024

.BANK 3 SLOT 1
.ORG $000
.INCBIN "ascii.chr" SKIP 1024 READ 512
;;palette data
	.db	$0f,$00,$00,$30
