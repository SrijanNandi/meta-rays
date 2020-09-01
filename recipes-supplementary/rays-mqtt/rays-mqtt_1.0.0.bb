SUMMARY = "Mqtt files for RayS"
DESCRIPTION = "The mqtt files package creates the mqrr subsystem for RayS" 
SECTION = "base"
LICENSE="CLOSED"

DEPENDS = "mosquitto jansson"

SRC_URI = "file://commonlib.cpp \
           file://commonlib.h \
           file://main_publisher.cpp \
           file://main_receiver.cpp \
           file://mqtt_client.h \
           file://mqtt_publisher.cpp \
           file://mqtt_receiver.cpp \
          "

S = "${WORKDIR}"

LDFLAGS_append += "-L${WORKDIR}/lib -lpthread -lmosquitto -ljansson"

do_compile() {
         ${CXX} main_publisher.cpp mqtt_publisher.cpp commonlib.cpp ${CFLAGS} ${LDFLAGS} -o mqtt_publisher
         ${CXX} main_receiver.cpp mqtt_receiver.cpp commonlib.cpp ${CFLAGS} ${LDFLAGS} -o mqtt_receiver
}

do_install() {
         install -d ${D}${bindir}
         install -c -m 0755 mqtt_publisher ${D}${bindir}
         install -c -m 0755 mqtt_receiver ${D}${bindir}
}

