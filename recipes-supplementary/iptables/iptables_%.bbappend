SYSTEMD_AUTO_ENABLE = "enable"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://iptables.rules \
            file://iptables.service \
            file://flush-iptables.sh \
           "

do_install_append () {
    install -m 0755 ${WORKDIR}/flush-iptables.sh ${D}${sbindir}/flush-iptables.sh
}
