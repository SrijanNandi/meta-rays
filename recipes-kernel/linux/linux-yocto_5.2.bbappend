KBRANCH_genericx86  = "v5.2/standard/base"
KBRANCH_genericx86-64  = "v5.2/standard/base"

KMACHINE_genericx86 ?= "common-pc"
KMACHINE_genericx86-64 ?= "common-pc-64"

SRCREV_machine_genericx86    ?= "25b14cdf9659c68ab32d66616dfdbbe2ece3fd94"
SRCREV_machine_genericx86-64 ?= "25b14cdf9659c68ab32d66616dfdbbe2ece3fd94"

COMPATIBLE_MACHINE_genericx86 = "genericx86"
COMPATIBLE_MACHINE_genericx86-64 = "genericx86-64"

LINUX_VERSION_genericx86 = "5.2.17"
LINUX_VERSION_genericx86-64 = "5.2.17"

LINUX_VERSION_EXTENSION = "-rays-${LINUX_KERNEL_TYPE}"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SRC_URI += "file://defconfig"
