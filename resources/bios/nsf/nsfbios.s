;»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
; NSF BIOS 6502 implementation
;»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»

.MEMORYMAP
	SLOTSIZE $400
	DEFAULTSLOT 0
	SLOT 0 $4400
	SLOT 1 $4800
	SLOT 2 $4C00
	SLOT 3 $FC00
.ENDME

.ROMBANKMAP
	BANKSTOTAL 4
	BANKSIZE $400
	BANKS 4
.ENDRO

;;nmi/irq/reset handing
.BANK 0 SLOT 0

.ORG $000
reset:
	nop
	nop
	nop
loop:
	jmp	loop

nmi:
	jmp	nmi

irq:
	jmp	irq

;;data
.BANK 0 SLOT 1

;;more data
.BANK 0 SLOT 2

;;this slot is just for vectors
.BANK 0 SLOT 3

.ORG $3FA
	.dw	nmi
	.dw	reset
	.dw	irq
