;;hle fds defines

;;the hlefds register
.define FDSHLECALL					$4220
.define FDSHLEREG						$4222

;;hle call indices
.define i_loadfiles					$00
.define i_writefile					$01
.define i_appendfile					$02

.define i_readpads					$10
.define i_orpads						$11
.define i_downpads					$12
.define i_readordownpads			$13
.define i_readdownverifypads		$14
.define i_readordownverifypads	$15
.define i_readdownexppads			$16

.define i_vramfill					$18
.define i_vramstructwrite			$19
.define i_writevrambuffer			$1A
.define i_readvrambuffer			$1B
.define i_preparevramstring		$1C
.define i_preparevramstrings		$1D
.define i_getvrambufferbyte		$1E
.define i_spritedma					$1F


.define i_loadtileset				$20
.define i_inc00by8					$21
.define i_inc00bya					$22

.define i_enpf							$28
.define i_dispf						$29
.define i_enobj						$2A
.define i_disobj						$2B
.define i_enpfobj						$2C
.define i_dispfobj					$2D
.define i_setscroll					$2E
.define i_vintwait					$2F

.define i_counterlogic				$30
.define i_random						$31
.define i_fetchdirectptr			$32
.define i_jumpengine					$33
.define i_memfill						$34

.define i_nmi							$38
.define i_irq							$39
.define i_reset						$3A

.define i_loadbootfiles				$3C

;;macro for calling hle function
.macro hlecall
	pha
	lda	#\1
	sta	FDSHLEREG
	pla
	sta	FDSHLECALL
.endm
