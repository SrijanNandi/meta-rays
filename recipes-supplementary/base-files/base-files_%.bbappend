FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://motd"

do_install_append () {
    install -d ${D}${sysconfdir}
    install -c -m 0644 ${WORKDIR}/motd ${D}${sysconfdir}/motd
}
