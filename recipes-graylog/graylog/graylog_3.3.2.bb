SUMMARY = "Graylog"
DESCRIPTION = "Graylog"
HOMEPAGE = "https://www.graylog.org"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504"

REQUIRED_DISTRO_FEATURES = "systemd"
DEPENDS = "openjdk-8 elasticsearch"
RDEPENDS_${PN} += "bash"

SRC_URI = "https://downloads.graylog.org/releases/${BPN}/${BPN}-${PV}.tgz \
           file://server.conf \
           file://graylog.service \
           file://default-graylog-server \
           file://graylog-server \
           file://GeoLite2-City.mmdb \
           file://log4j2.xml \
           "

SRC_URI[md5sum] = "2fb9b91ffa8a4a1ddafd361095d229b1"
SRC_URI[sha256sum] = "627486bc6f1ddd00d3bda5243ea7c642c175cec052df71b55b01996824b78057"

S = "${WORKDIR}/${BPN}-${PV}"

inherit autotools systemd pkgconfig useradd

USERADD_PACKAGES = "${PN}"
GROUPADD_PACKAGES = "${PN}"
GROUPADD_PARAM_${PN} = "${PN}"
USERADD_PARAM_${PN} = " \
    --system --no-create-home \
    --shell /bin/false \
    -g ${PN} \
    ${PN}"

SYSTEMD_PACKAGES += "${BPN}"
SYSTEMD_SERVICE_${PN} = "graylog.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_compile() {
}

do_install() {
        install -d ${D}${datadir} ${D}${datadir}/${PN}
        install -d ${D}${sysconfdir} ${D}${sysconfdir}/${PN} ${D}${sysconfdir}/${PN}/server
        install -d ${D}${sysconfdir} ${D}${sysconfdir}/default
        install -d ${D}${localstatedir} ${D}${localstatedir}/lib ${D}${localstatedir}/lib/${PN}
        install -d ${D}${localstatedir} ${D}${localstatedir}/log ${D}${localstatedir}/log/${PN}
        chown -R graylog:graylog ${D}${localstatedir}/lib/${PN}
        chown -R graylog:graylog ${D}${localstatedir}/log/${PN}
        cp -r ${S}/* ${D}${datadir}/${PN}
        install -c -m 0755 ${WORKDIR}/graylog-server ${D}${datadir}/${PN}/bin
        chown -R graylog:graylog ${D}${datadir}/${PN}
        install -c -m 0644 ${WORKDIR}/server.conf ${D}${sysconfdir}/graylog/server
        install -c -m 0644 ${WORKDIR}/GeoLite2-City.mmdb ${D}${sysconfdir}/graylog/server
        install -c -m 0644 ${WORKDIR}/log4j2.xml ${D}${sysconfdir}/graylog/server
        chown -R graylog:graylog ${D}${sysconfdir}/${PN}
        install -c -m 0644 ${WORKDIR}/default-graylog-server ${D}${sysconfdir}/default/graylog-server
        chown graylog:graylog ${D}${sysconfdir}/default/graylog-server

        if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
            install -d ${D}${systemd_system_unitdir}
            install -c -m 0644 ${WORKDIR}/graylog.service ${D}${systemd_system_unitdir}
        fi
}


FILES_${PN} += "${systemd_system_unitdir}"
FILES_${PN} += "${datadir}"

INHIBIT_PACKAGE_STRIP = "1"
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"
INSANE_SKIP_${PN} += "already-stripped ldflags host-user-contaminated libdir arch"

