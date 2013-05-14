;;hle fds defines

;;the hlefds register
.define FDSHLE				$4222

;;hle call indices
.define i_loadfiles		$00

.define i_readpads		$10
.define i_orpads			$11
.define i_downpads		$12

.define i_vramfill		$18
.define i_spritedma		$19

.define i_memfill			$20

.define i_forceinsert	$38

;;macro for calling hle function
.macro hlecall
	lda	#\1
	sta	FDSHLE
.endm
