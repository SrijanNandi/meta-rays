SUMMARY = "Miscellaneous files for GrayS initial subsystem"
DESCRIPTION = "The rc.initial files package creates the initial menu option for GrayS"
SECTION = "base"
LICENSE="CLOSED"

SRC_URI = "file://rc.initial \
           file://rc.initial.toggle_ips \
           file://rc.initial.ping \
           file://rc.initial.showports \
           file://rc.initial.toggle_sshd \
          "

do_compile() {
}

do_install() {
    install -d ${D}${sysconfdir}
    install -c -m 0755 ${WORKDIR}/rc.initial ${D}${sysconfdir}/rc.initial
    install -c -m 0755 ${WORKDIR}/rc.initial.toggle_ips ${D}${sysconfdir}/rc.initial.toggle_ips
    install -c -m 0755 ${WORKDIR}/rc.initial.ping ${D}${sysconfdir}/rc.initial.ping
    install -c -m 0755 ${WORKDIR}/rc.initial.showports ${D}${sysconfdir}/rc.initial.showports
    install -c -m 0755 ${WORKDIR}/rc.initial.toggle_sshd ${D}${sysconfdir}/rc.initial.toggle_sshd
}

INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INSANE_SKIP_${PN} += "already-stripped ldflags host-user-contaminated libdir arch"
