;;hle fds defines

;;the hlefds register
.define FDSHLECALL					$4220
.define FDSHLECALL2					$4221
.define FDSHLEREG						$4222
.define FDSHLEREG2					$4223
.define FDSHLEREG3					$4224
.define FDSHLEREG4					$4225

;;hle call indices
.define i_loadfiles					$00
.define i_writefile					$01

.define i_xferdone					$08

.define i_readpads					$10
.define i_orpads						$11
.define i_downpads					$12
.define i_readordownpads			$13
.define i_readdownverifypads		$14
.define i_readordownverifypads	$15
.define i_readdownexppads			$16
.define i_readkeyboard				$17

.define i_vramfill					$18
.define i_vramstructwrite			$19
.define i_writevrambuffer			$1A
.define i_readvrambuffer			$1B
.define i_preparevramstring		$1C
.define i_preparevramstrings		$1D
.define i_getvrambufferbyte		$1E
.define i_loadtileset				$1F

;;.define i_inc00by8					$21
;;.define i_inc00bya					$22

.define i_enpf							$28
.define i_dispf						$29
.define i_enobj						$2A
.define i_disobj						$2B
.define i_enpfobj						$2C
.define i_dispfobj					$2D
.define i_setscroll					$2E
.define i_spritedma					$2F

.define i_counterlogic				$30
.define i_random						$31
.define i_fetchdirectptr			$32
.define i_jumpengine					$33
.define i_memfill						$34
.define i_pixel2nam					$35
.define i_nam2pixel					$36
.define i_unk_ec22					$37

.define i_delay132					$38
.define i_delayms						$39
.define i_vintwait					$3A

.define i_nmi							$3C
.define i_irq							$3D
.define i_reset						$3E

;;macro for calling hle function
.macro hlecall
	pha
	lda	#\1
	sta	FDSHLEREG
	pla
	sta	FDSHLECALL
.endm

.macro hlecall2
	lda	#\1
	sta	FDSHLECALL2
.endm
