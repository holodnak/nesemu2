;;fds defines

;;writeable registers
.define FDSIRQLOW			$4020
.define FDSIRQHIGH		$4021
.define FDSIRQENABLE		$4022
.define FDSIOENABLE		$4023
.define FDSWRITE			$4024
.define FDSCONTROL		$4025
.define FDSEXTWRITE		$4026

;;readable registers
.define FDSSTATUS			$4030
.define FDSREAD			$4031
.define FDSDISKSTATUS	$4032
.define FDSEXTREAD		$4033

;;sound registers
