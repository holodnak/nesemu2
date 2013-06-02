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

.define PPUCTRL			$2000
.define PPUMASK			$2001
.define PPUSTATUS			$2002
.define OAMADDR			$2003
.define OAMDATA			$2004
.define PPUSCROLL			$2005
.define PPUADDR			$2006
.define PPUDATA			$2007

.define BIOSBANK			$3400
.define IRQENABLE			$3401		;;read:  acknowledge irq    -- write:  enable/disable irq
.define IRQLATCHLO		$3402
.define IRQLATCHHI		$3403
.define PLAYSPDLO			$3410		;;write:  set playspeed low byte (and update irqlatch with calculated speed)
.define PLAYSPDHI			$3411		;;write:  set playspeed high byte (and update irqlatch with calculated speed)
.define STRING				$3418		;;read:  get char, inc pos  -- write:  setup string position

.define DISASM				$3420		;;write:  enable/disable disassembly
.define DEBUG				$3421		;;write:  output debug

.define NSF_RAM			$3440

.define NSF_HEADER		$3480
.define NSF_NUMSONGS		NSF_HEADER+$06		;;total number of songs
.define NSF_STARTSONG	NSF_HEADER+$07		;;starting song number
.define NSF_INIT			NSF_HEADER+$0A		;;init routine address
.define NSF_PLAY			NSF_HEADER+$0C		;;play routine address
.define NSF_TITLE			NSF_HEADER+$0E		;;nsf title
.define NSF_PLAYSPEED	NSF_HEADER+$6E		;;play speed
.define NSF_NTSCPAL		NSF_HEADER+$7A		;;ntsc/pal bits
.define NSF_BANKS			NSF_HEADER+$70		;;bankswitch values

.macro swap args bank
	lda	#bank
	sta	BIOSBANK
.endm

.macro ldsta
	lda	#\1
	sta	\2
.endm

.macro ldsty
	ldy	#\1
	sty.w	\2
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

;;initialize the nes
initnes:
	ldsty	$40,$4017		;;disable frame irq
	ldsty	$0F,$4015		;;setup volume
	ldx	#0
	stx.w	PPUCTRL			;;disable nmi
	stx.w	PPUMASK			;;disable rendering
	stx.w	$4010				;;disable dmc irq
	lda	#0
	sta	PPUCTRL
	sta	PPUMASK
	rts

;;initialize the nsf banks
initbanks:
	ldx	#7
-	lda	NSF_BANKS,x
	sta	$5FF8,x
	dex
	bpl	-
	rts

;;copy the ascii chr to vram
copyascii:

	ldy	PPUSTATUS		;;reset the toggle
	ldsty	$02,PPUADDR		;;load high byte of address
	ldsty	$00,PPUADDR		;;load low byte of address

	ldsta	$3C,$01			;;setup source address
	ldsta	$00,$00
	swap	2					;;map the first bank of data to $3C00
	ldx	#4					; number of 256-byte pages to copy
	jsr	copyvram

	ldsta	$3C,$01			;;setup source address
	ldsta	$00,$00
	swap	3					;;map the second bank of data to $3C00
	ldx	#2					; number of 256-byte pages to copy
	jsr	copyvram

	rts						;;return

;;wait for vblank, missing it sometimes
ppuwait:
	bit PPUSTATUS
	bpl ppuwait
	rts

;;zero out a tile
zerotile:
	lda	#0

;;fill a tile with specific byte (A register)
filltile:
	ldy	#16
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
	ldsty	$3F,PPUADDR
	ldsty	$00,PPUADDR
-	lda	($00),y			; copy one byte
	sta	PPUDATA
	iny
	dex
	bne	-					; repeat until we finish
	rts

;;clear vram/nametables
;;y = number of bytes
;;x = number of pages
clearvram:
	lda	#0
-	sta	PPUDATA
	dey
	bne	-					; repeat until we finish the page
	dex
	bne	-					; repeat until we've copied enough pages
	rts

;;copy string from ($00) to ppu memory
copystring:
	ldy	#0
-	lda	($00),y
	iny
	cmp	#0
	beq	+
	sta	PPUDATA
	bne	-
+	rts

;;setup nsf play speed
setspeed:
	lda	NSF_PLAYSPEED
	sta	PLAYSPDLO
	lda	NSF_PLAYSPEED+1
	sta	PLAYSPDHI
	rts

reset:

	sei						;;disable irq
	cld						;;disable decimal mode
	ldx	#$FF
	txs						;;setup stack
	inx
-	sta	$000,x			;;clear ram
	sta	$100,x
	sta	$200,x
	sta	$300,x
	sta	$400,x
	sta	$500,x
	sta	$600,x
	sta	$700,x
	inx
	bne	-
	jsr	initnes

	jsr	ppuwait
	jsr	ppuwait			;;let ppu warm up, wait for vblank

	jsr	copyascii		;;copy the ascii chr to vram
	ldx	#32				;;number of bytes to copy from the palette
	jsr	copypalette		;;now copy palette, it is after the chr

	;;setup tile 0
	ldy	#0					; starting index into the first page
	sty.w	PPUADDR			; load the destination address into the PPU
	sty.w	PPUADDR
	jsr	zerotile

	;;setup tile $FF
	ldsty	$0F,PPUADDR		;;upper byte of dest addr
	ldsty	$F0,PPUADDR		;;lower byte of dest addr
	lda	#$FF				;;byte to fill with
	jsr	filltile

	;;clear nametables
	ldsty	$20,PPUADDR		;;high byte of destination
	ldsty	$00,PPUADDR		;;low byte of destination
	ldx	#4					;;number of 256 byte loops
	jsr	clearvram

	swap	1					;;map in the other bank

	;;write sprite0 hit tile
	ldsty	$23,PPUADDR		;;high byte of destination address
	ldsty	$40,PPUADDR		;;low byte of destination address
	ldsta	$FF,PPUDATA

	;;write sprite0 hit tile attribute
	ldsty	$23,PPUADDR		;;high byte of destination address
	ldsty	$F0,PPUADDR		;;low byte of destination address
	ldsta	$70,PPUDATA
	;;write attributes for the status bar
	ldsta	$50,PPUDATA
	ldsta	$50,PPUDATA
	ldsta	$50,PPUDATA
	ldsta	$50,PPUDATA

	;;copy the info string
	ldsty	$23,PPUADDR		;;high byte of destination address
	ldsty	$62,PPUADDR		;;low byte of destination address
	ldsta	>title,$01		;;high byte of source address
	ldsta	<title,$00		;;low byte of source address
	jsr	copystring

	;;copy the song title
	ldsty	$20,PPUADDR		;;high byte of destination address
	ldsty	$A2,PPUADDR		;;low byte of destination address
	ldsta	>NSF_TITLE,$01	;;high byte of source address
	ldsta	<NSF_TITLE,$00	;;low byte of source address
	jsr	copystring		;;copy nsf title

	;;setup sprite0
	lda	#0					;;sprite data
	sta	OAMADDR
	ldsty	$CE,OAMDATA
	ldsty	$FF,OAMDATA
	ldsty	$20,OAMDATA
	ldsty	$00,OAMDATA

	jsr	setspeed			;;setup play speed
	jsr	initbanks		;;initialize nsf banks

	ldsty	$00,PPUSCROLL	;;reset scroll to 0,0
	ldsty	$00,PPUSCROLL
	ldsty	$1E,PPUMASK		;;enable rendering

	lda	NSF_STARTSONG	;;load starting song
	tax
	dex
	txa
	ldx	#0					;;this is ntsc/pal byte (shouldn't always be 0)
	jsr	nsfinit			;;init song

	ldsty	$01,IRQENABLE	;;enable irq counter
	ldsty	$80,PPUCTRL		;;enable nmi
	cli						;;enable irq

mainloop:


-	lda	$2002				;;wait until sprite0 flag clears
	and	#%01000000
	bne	-

	ldx	PPUSTATUS		;;reset toggle
	ldx	#0					;;restore scroll to 0,0
	stx.w	PPUSCROLL
	stx.w	PPUSCROLL

-	lda	$2002				;;wait for sprite0 hit
	and	#%01000000
	beq	-

	lda	NSF_RAM			;;update scroll register with new x value
	sta	$2005
	lda	#$00
	sta	$2005

	lda	#%10000000		;;enable nmi
	sta	PPUCTRL
 
	jmp	mainloop			;;keep looping

nsfinit:
	jmp	(NSF_INIT)
nsfplay:
	jmp	(NSF_PLAY)

irq:
	pha						;;save registers
	tya
	pha
	txa
	pha
	lda	IRQENABLE		;;acknowledge irq
	jsr	nsfplay			;;execute play routine
	pla						;;restore registers
	tax
	pla
	tay
	pla
	rti

nmi:
	rti
	;;this code causes nsf's to lock up!

	pha						;;save registers
	txa
	pha
	
	lda	#%00000000		;;disable NMI
	sta	PPUCTRL

	ldx	NSF_RAM			;;increment scroll x
	inx
	stx.w	NSF_RAM

	jsr	counterinc		;;increment timer

	ldsty	$21,PPUADDR		;;high byte of destination address
	ldsty	$42,PPUADDR		;;low byte of destination address
	lda	NSF_RAM+33
	and	#$F0
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	clc
	adc	#$30
	sta	PPUDATA
	lda	NSF_RAM+33
	and	#$0F
	clc
	adc	#$30
	sta	PPUDATA
	ldsty	$00,PPUSCROLL		;;high byte of destination address
	ldsty	$00,PPUSCROLL		;;low byte of destination address


	pla						;;restore registers
	tax
	pla

	rti						;;return from interrupt

counterinc:

	;;increment frame count
	ldx	(NSF_RAM+32)
	inx
	cpx	#60
	bne	++
	ldx	#0

	;;increment seconds
	ldy	(NSF_RAM+33)
	iny
	tya
	and	#$0F
	cmp	#$0A
	bne	+
	tya
	clc
	adc	#6
	tay
+	sty.w	(NSF_RAM+33)
++	stx.w	(NSF_RAM+32)
	rts

title:
	.db	"nesemu2 nsf player",0

nowplaying:
	.db	"Now Playing:",0

;;misc
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
palette:
	.db	$0f,$00,$10,$20,  $0f,$1a,$2a,$3a,  $0f,$0f,$0f,$0f,  $0f,$0f,$0f,$0f
	.db	$0f,$0f,$0f,$0f,  $0f,$1a,$2a,$3a,  $0f,$12,$22,$32,  $0f,$1d,$2d,$3d

;;sprite data
;;y,tile,attrib,x
.ORG $300
sprites:
	.db	$80,$88,$00,$80,  $C0,$60,$00,$00