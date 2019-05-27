# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := hicolor-icons
$(PKG)_WEBSITE  := https://www.freedesktop.org/wiki/Software/icon-theme
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 0.17
$(PKG)_CHECKSUM := 317484352271d18cbbcfac3868eab798d67fff1b8402e740baa6ff41d588a9d8
#$(PKG)_PATCHES  := $(realpath $(sort $(wildcard $(dir $(lastword $(MAKEFILE_LIST)))/$(PKG)-[0-9]*.patch)))
$(PKG)_SUBDIR   := hicolor-icon-theme-$($(PKG)_VERSION)
$(PKG)_FILE     := hicolor-icon-theme-$($(PKG)_VERSION).tar.gz
$(PKG)_URL      := http://icon-theme.freedesktop.org/releases/hicolor-icon-theme-0.17.tar.xz/$($(PKG)_FILE)
$(PKG)_URL_2    := https://icon-theme.freedesktop.org/releases/hicolor-icon-theme-0.17.tar.xz
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
    $(MAKE) -C '$(BUILD_DIR)' -j 1 install
endef

