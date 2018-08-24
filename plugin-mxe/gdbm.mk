# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := gdbm
$(PKG)_WEBSITE  := https://pyyaml.org/wiki/LibYAML
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 1.17
$(PKG)_CHECKSUM := 7cd8cc2e35b1aaede6084ea57cc9447752f498daaea854100a4bad567614977d
$(PKG)_SUBDIR   := gdbm-$($(PKG)_VERSION)
$(PKG)_FILE     := gdbm-$($(PKG)_VERSION).tar.gz
$(PKG)_URL      := https://ftp.gnu.org/gnu/gdbm/$($(PKG)_FILE)
$(PKG)_URL_2    := https://ftp.gnu.org/gnu/gdbm/gdbm-1.17.tar.gz
$(PKG)_DEPS     := cc 

$(PKG)_DEPS_$(BUILD) := 

define $(PKG)_UPDATE
    echo 'TODO: write update script for $(PKG).' >&2;
    echo $($(PKG)_VERSION)
endef

define $(PKG)_BUILD
    # build and install the library
    cd '$(BUILD_DIR)' && $(SOURCE_DIR)/configure \
        $(MXE_CONFIGURE_OPTS) \
        --enable-libgdbm-compat
    $(MAKE) -C '$(BUILD_DIR)' -j '$(JOBS)'
    $(MAKE) -C '$(BUILD_DIR)' -j 1 install
endef

