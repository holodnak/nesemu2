;;hle fds defines

;;the hlefds register
.define FDSHLE					$4222

;;hle call indices
.define i_loadfiles			$00

.define i_readpads			$10
.define i_orpads				$11
.define i_downpads			$12

.define i_vramfill			$18
.define i_vramstructwrite	$19
.define i_spritedma			$1A
.define i_setscroll			$1B
.define i_loadtileset		$1C
.define i_inc00bya			$1D

.define i_memfill				$20

.define i_counterlogic		$28
.define i_random				$29

.define i_nmi					$38
.define i_irq					$39
.define i_reset				$3A

.define i_forceinsert		$3C

;;macro for calling hle function
.macro hlecall
	lda	#\1
	sta	FDSHLE
.endm
