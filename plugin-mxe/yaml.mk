# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := yaml
$(PKG)_WEBSITE  := https://pyyaml.org/wiki/LibYAML
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 0.1.7
$(PKG)_CHECKSUM := 8088e457264a98ba451a90b8661fcb4f9d6f478f7265d48322a196cec2480729
#$(PKG)_PATCHES  := $(realpath $(sort $(wildcard $(dir $(lastword $(MAKEFILE_LIST)))/$(PKG)-[0-9]*.patch)))
$(PKG)_SUBDIR   := yaml-$($(PKG)_VERSION)
$(PKG)_FILE     := yaml-$($(PKG)_VERSION).tar.gz
$(PKG)_URL      := https://pyyaml.org/download/libyaml/$($(PKG)_FILE)
$(PKG)_URL_2    := https://pyyaml.org/download/libyaml/yaml-0.1.7.tar.gz
$(PKG)_DEPS     := cc

$(PKG)_DEPS_$(BUILD) := autotools 

define $(PKG)_UPDATE
    echo 'TODO: write update script for $(PKG).' >&2;
    echo $($(PKG)_VERSION)
endef

define $(PKG)_BUILD
    # build and install the library
    cd '$(SOURCE_DIR)' && NOCONFIGURE=true autoreconf -fi
    cd '$(BUILD_DIR)' && $(SOURCE_DIR)/configure \
        $(MXE_CONFIGURE_OPTS)
    $(MAKE) -C '$(BUILD_DIR)' -j '$(JOBS)'
    $(MAKE) -C '$(BUILD_DIR)' -j 1 install
endef

