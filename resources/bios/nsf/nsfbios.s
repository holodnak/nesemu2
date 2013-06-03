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

;;nes registers
.define PPUCTRL			$2000
.define PPUMASK			$2001
.define PPUSTATUS			$2002
.define OAMADDR			$2003
.define OAMDATA			$2004
.define PPUSCROLL			$2005
.define PPUADDR			$2006
.define PPUDATA			$2007

;;nsf special registers
.define BIOSBANK			$3400
.define IRQENABLE			$3401		;;read:  acknowledge irq    -- write:  enable/disable irq
.define IRQLATCHLO		$3402
.define IRQLATCHHI		$3403
.define PLAYSPDLO			$3410		;;write:  set playspeed low byte (and update irqlatch with calculated speed)
.define PLAYSPDHI			$3411		;;write:  set playspeed high byte (and update irqlatch with calculated speed)
.define STRING				$3418		;;read:  get char, inc pos  -- write:  setup string position

;;debugging registers
.define DISASM				$3420		;;write:  enable/disable disassembly
.define DEBUG				$3421		;;write:  output debug

;;nsf ram address (64 bytes)
.define NSF_RAM			$3440

;;temp area (8 bytes)
.define TEMP				NSF_RAM+8

;;x scroll value for scrolling area
.define SCROLL_X			NSF_RAM+16

;;current song number
.define CURSONG			NSF_RAM+17

;;joypad data
.define PADDATA			NSF_RAM+18
.define PADDATA2			NSF_RAM+19

;;timer data
.define TIME_FRAMES		NSF_RAM+32
.define TIME_SECONDS		NSF_RAM+33
.define TIME_MINUTES		NSF_RAM+34

;;nsf header
.define NSF_HEADER		$3480
.define NSF_NUMSONGS		NSF_HEADER+$06		;;total number of songs
.define NSF_STARTSONG	NSF_HEADER+$07		;;starting song number
.define NSF_INIT			NSF_HEADER+$0A		;;init routine address
.define NSF_PLAY			NSF_HEADER+$0C		;;play routine address
.define NSF_TITLE			NSF_HEADER+$0E		;;nsf title
.define NSF_ARTIST		NSF_HEADER+$2E		;;nsf artist
.define NSF_COPYRIGHT	NSF_HEADER+$4E		;;nsf copyright
.define NSF_PLAYSPEED	NSF_HEADER+$6E		;;play speed
.define NSF_NTSCPAL		NSF_HEADER+$7A		;;ntsc/pal bits
.define NSF_BANKS			NSF_HEADER+$70		;;bankswitch values

;;save registers to stack
.macro pushregs
	pha
	tya
	pha
	txa
	pha
.endm

;;restore registers from stack
.macro popregs
	pla
	tax
	pla
	tay
	pla
.endm

;;swap in bank
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

playername:
	.db	"nesemu2 nsf player",0

nsfinit:
	jmp	(NSF_INIT)
nsfplay:
	jmp	(NSF_PLAY)

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

;;copy all vram/palette/sprites to ppu
setupppu:

	;;setup ppu for data copy
	ldy	PPUSTATUS		;;reset the toggle
	ldsty	$02,PPUADDR		;;load high byte of destination address
	ldsty	$00,PPUADDR		;;load low byte of destination address

	;;copy first 4kb of chr
	swap	2					;;map the first bank of data to $3C00
	ldsta	$3C,$01			;;high byte of source address
	ldsta	$00,$00			;;low byte of source address
	ldx	#4					; number of 256-byte pages to copy
	jsr	copyvram

	;;copy last 2kb of chr
	swap	3					;;map the second bank of data to $3C00
	ldsta	$3C,$01			;;setup source address
	ldsta	$00,$00
	ldx	#2					;;number of 256-byte pages to copy
	jsr	copyvram

	;;copy palette data
	ldsty	$3F,PPUADDR		;;setup destination ppu address
	ldsty	$00,PPUADDR
	ldx	#32				;;number of bytes to copy from the palette
-	lda	($00),y			;;load byte to copy to ppu
	sta	PPUDATA			;;write byte to ppu
	iny						;;increment source address
	dex						;;decrement counter
	bne	-					;;repeat until all copied

	;;copy sprite data
	lda	#0					;;sprite data address
	sta	OAMADDR
	lda	#$3F				;;page to dma from
	sta	$4014				;;execute sprite dma

	;;setup tile 0
	ldy	#0					;;starting index into the first page
	sty.w	PPUADDR			;;load the destination address into the PPU
	sty.w	PPUADDR
	jsr	zerotile			;;write all 0's to tile

	;;setup tile $FF for sprite0 hit
	ldsty	$0F,PPUADDR		;;upper byte of dest addr
	ldsty	$F0,PPUADDR		;;lower byte of dest addr
	lda	#$FF				;;byte to fill with
	jsr	filltile			;;fill tile with $FF

	;;clear nametables
	ldsty	$20,PPUADDR		;;high byte of destination
	ldsty	$00,PPUADDR		;;low byte of destination
	ldx	#4					;;number of 256 byte loops
	jsr	clearvram

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

	;;copy the info string into scrolling area
	ldsty	$23,PPUADDR			;;high byte of destination address
	ldsty	$62,PPUADDR			;;low byte of destination address
	ldsta	>playername,$01	;;high byte of source address
	ldsta	<playername,$00	;;low byte of source address
	jsr	copystring

	rts						;;return

;;copy string from ($00) to ppu memory (helper for init)
copystring:
	ldy	#0
-	lda	($00),y
	iny
	cmp	#0
	beq	+
	sta	PPUDATA
	bne	-
+	rts

;;initialize the nsf banks
initbanks:
	ldx	#7
-	lda	NSF_BANKS,x
	sta	$5FF8,x
	dex
	bpl	-
	rts

;;setup nsf play speed
setspeed:
	lda	NSF_PLAYSPEED
	sta	PLAYSPDLO
	lda	NSF_PLAYSPEED+1
	sta	PLAYSPDHI
	rts

setupnsf:
	;;copy the song title
	ldsty	$20,PPUADDR				;;high byte of destination address
	ldsty	$A1,PPUADDR				;;low byte of destination address
	ldsta	>NSF_TITLE,$01			;;high byte of source address
	ldsta	<NSF_TITLE,$00			;;low byte of source address
	jsr	copystring				;;copy nsf title

	;;copy the song artist
	ldsty	$20,PPUADDR				;;high byte of destination address
	ldsty	$C1,PPUADDR				;;low byte of destination address
	ldsta	>NSF_ARTIST,$01		;;high byte of source address
	ldsta	<NSF_ARTIST,$00		;;low byte of source address
	jsr	copystring				;;copy nsf artist

	;;copy the song copyright
	ldsty	$20,PPUADDR				;;high byte of destination address
	ldsty	$E1,PPUADDR				;;low byte of destination address
	ldsta	>NSF_COPYRIGHT,$01	;;high byte of source address
	ldsta	<NSF_COPYRIGHT,$00	;;low byte of source address
	jsr	copystring				;;copy nsf copyright
	
	;;setup CURSONG variable
	ldx	NSF_STARTSONG			;;load starting song
	dex
	stx.w	CURSONG					;;save it in the cursong variable

	;;other nsf init stuff
	jsr	setspeed					;;setup play speed
	jsr	initbanks				;;initialize nsf banks
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
	
	;;perform initialization
	jsr	initnes					;;initialize the nes
	jsr	ppuwait					;;let ppu warm up, wait for vblank
	jsr	ppuwait
	jsr	setupppu					;;copy all ppu data and setup other stuff
	jsr	setupnsf					;;setup nsf and copy nsf strings to nametable

	swap	1							;;map in the other bank, nmi will use it

	ldsty	$00,PPUSCROLL			;;reset scroll to 0,0
	ldsty	$00,PPUSCROLL
	ldsty	$1E,PPUMASK				;;enable rendering

	ldsty	$00,PADDATA
	ldsty	$00,PADDATA2

	jsr	startsong

	ldsty	$01,IRQENABLE			;;enable irq counter
	ldsty	$80,PPUCTRL				;;enable nmi
	cli								;;enable irq

mainloop:
-	lda	$2002						;;wait until sprite0 flag clears
	and	#%01000000
	bne	-

	ldx	PPUSTATUS				;;reset toggle
	ldx	#0							;;restore scroll to 0,0
	stx.w	PPUSCROLL
	stx.w	PPUSCROLL

-	lda	$2002						;;wait for sprite0 hit
	and	#%01000000
	beq	-

	lda	SCROLL_X					;;update scroll register with new x value
	sta	$2005
	lda	#$00
	sta	$2005

	lda	#%10000000				;;enable nmi
	sta	PPUCTRL
 
	jmp	mainloop					;;keep looping

;;irq handler (just play the music)
irq:
	pushregs
	lda	IRQENABLE				;;acknowledge irq
	jsr	nsfplay					;;execute play routine
	popregs
	rti

;;nmi handler (update player stuff)
nmi:
	pushregs					;;save registers

	lda	#%00000000		;;disable NMI
	sta	PPUCTRL

	jsr	drawtimer
	jsr	drawsong

	ldsty	$00,PPUSCROLL	;;scroll x offset
	ldsty	$00,PPUSCROLL	;;scroll y offset

	jsr	scrollinc		;;increment scrolling
	jsr	timerinc			;;increment nsf play timer
	jsr	readpads2		;;read controller buttons

	lda	PADDATA			;;load controller data
	and	#$01				;;mask off right
	beq	+					;;if 0, then branch
	jsr	nextsong			;;go to next song
+	lda	PADDATA			;;load controller data
	and	#$02				;;mask off left
	beq	+					;;go to previous song
	jsr	prevsong

+	popregs
	rti

;;misc stuff (nmi handler uses most of this)
.BANK 1 SLOT 1
.ORG $000

;;draw the play timer
drawtimer:

	ldsty	$23,PPUADDR		;;high byte of destination address
	ldsty	$19,PPUADDR		;;low byte of destination address

	lda	#0
	sta	PPUDATA

	lda	TIME_MINUTES
	and	#$F0
	cmp	#0
	beq	+
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	clc
	adc	#$30
+	sta	PPUDATA
	lda	TIME_MINUTES
	and	#$0F
	clc
	adc	#$30
	sta	PPUDATA

	lda	#$3A
	sta	PPUDATA

	lda	TIME_SECONDS
	and	#$F0
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	clc
	adc	#$30
	sta	PPUDATA
	lda	TIME_SECONDS
	and	#$0F
	clc
	adc	#$30
	sta	PPUDATA

	rts

drawsong:

	ldsty	$21,PPUADDR		;;high byte of destination address
	ldsty	$64,PPUADDR		;;low byte of destination address

	lda	CURSONG
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	clc
	adc	#$30
	sta	PPUDATA

	lda	CURSONG
	and	#$0F
	clc
	adc	#$30
	sta	PPUDATA
	
	lda	#0
	sta	PPUDATA

	lda	NSF_NUMSONGS
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	clc
	adc	#$30
	sta	PPUDATA

	lda	NSF_NUMSONGS
	and	#$0F
	clc
	adc	#$30
	sta	PPUDATA

	rts

;;start song pointed by CURSONG
startsong:
	lda	CURSONG
initsong:
	ldx	#0							;;this is ntsc/pal byte (shouldn't always be 0)
	jsr	nsfinit					;;init song
	jmp	timerreset				;;reset timer

;;go to the next song
nextsong:
	lda	CURSONG			;;load current song number
	tax
	inx
	cpx	NSF_NUMSONGS	;;check if we already at last song
	beq	+					;;branch if already 0
	clc						;;clear carry
	adc	#1					;;add 1
	sta	CURSONG
+	jmp	initsong

;;go to previous song
prevsong:
	lda	CURSONG			;;load current song number
	beq	+					;;branch if already 0
	sec						;;set carry
	sbc	#1					;;subtract 1
	sta	CURSONG
+	jmp	initsong			;;initialize song

;;read controller buttons, stores them in PADDATA
readpads:
	ldsty	1,$4016			;;strobe controllers
	ldsty	0,$4016
	lda	#0
	sta	PADDATA
	ldx	#8
-	lda	$4016
	lsr	a
	rol	PADDATA
	dex
	bne	-
	rts

;;read controller buttons, store up->down changes in PADDATA
readpads2:
	jsr	readpads
	lda	PADDATA
	tay
	eor	PADDATA2
	and	PADDATA
	sta	PADDATA
	sty.w	PADDATA2
	rts

;;increment bottom scrolling area
scrollinc:
	ldx	SCROLL_X			;;increment scroll x
	inx
	stx.w	SCROLL_X
	rts

;;increment the play timer
timerinc:
	jsr	incframes
	cpx	#0
	bne	+
	jsr	incseconds
	cpx	#0
	bne	+
	jsr	incminutes
+	rts

;;reset play timer
timerreset:
	lda	#0
	sta	TIME_FRAMES
	sta	TIME_SECONDS
	sta	TIME_MINUTES
	rts

;;increment frame counter
;;leaves frame count in x
incframes:
	ldx	TIME_FRAMES				;;load frame count
	inx								;;increment
	cpx	#60						;;check if it is 60
	bne	+							;;branch if not equal
	ldx	#0							;;reset to 0
+	stx.w	TIME_FRAMES				;;save frame count
	rts

;;increment seconds counter
;;leaves seconds in x
incseconds:
	ldx	TIME_SECONDS			;;load seconds counter
	inx								;;increment
	txa
	and	#$0F						;;keep lower four bits
	cmp	#$0A						;;check for overflow
	bne	+

	;;process upper nibble
	txa
	clc
	adc	#6							;;add 6 to $A, making lower nibble 0 and incrementing upper
	tax
	and	#$F0						;;keep upper bits
	cmp	#$60						;;check for overflow
	bne	+
	lda	#0							;;zero
	tax
+	stx.w	TIME_SECONDS
	rts

;;increment minutes counter
;;leaves minutes in x
incminutes:
	ldx	TIME_MINUTES
	inx
	txa
	and	#$0F						;;keep lower four bits
	cmp	#$0A						;;check for overflow
	bne	+
	txa								;;process upper nibble
	clc
	adc	#6							;;add 6 to $A, making lower nibble 0 and incrementing upper
	tax
	and	#$F0						;;keep upper bits
	cmp	#$A0						;;check for overflow
	bne	+
	lda	#0							;;zero
	tax
+	stx.w	TIME_MINUTES
	rts

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
	.db	$CE,$FF,$20,$00,
.repeat 63
	.db	$FF,$FF,$FF,$FF,
.endr