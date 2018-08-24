# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := ruby
$(PKG)_WEBSITE  := https://ruby-lang.org
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 2.3.7
$(PKG)_CHECKSUM := 35cd349cddf78e4a0640d28ec8c7e88a2ae0db51ebd8926cd232bb70db2c7d7f
$(PKG)_SUBDIR   := ruby-$($(PKG)_VERSION)
$(PKG)_FILE     := ruby-$($(PKG)_VERSION).tar.gz
$(PKG)_URL      := https://cache.ruby-lang.org/pub/ruby/2.3/$($(PKG)_FILE)
$(PKG)_URL_2    := https://cache.ruby-lang.org/pub/ruby/2.3/ruby-2.3.7.tar.gz
$(PKG)_DEPS     := cc 

$(PKG)_DEPS_$(BUILD) := autotools 

define $(PKG)_UPDATE
    echo 'TODO: write update script for $(PKG).' >&2;
    echo $($(PKG)_VERSION)
endef

define $(PKG)_BUILD
    # build and install the library
    cd '$(BUILD_DIR)' && $(SOURCE_DIR)/configure \
        $(MXE_CONFIGURE_OPTS) \
        --enable-shared \
        --enable-load-relative \
        --disable-install-doc \
        --without-tk --without-tcllib --without-tcltk 
    $(MAKE) -C '$(BUILD_DIR)' -j '$(JOBS)'
    $(MAKE) -C '$(BUILD_DIR)' -j 1 install
endef

