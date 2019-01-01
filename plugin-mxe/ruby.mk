# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := ruby
$(PKG)_WEBSITE  := https://ruby-lang.org
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 2.3.8
$(PKG)_CHECKSUM := b5016d61440e939045d4e22979e04708ed6c8e1c52e7edb2553cf40b73c59abf
$(PKG)_SUBDIR   := ruby-$($(PKG)_VERSION)
$(PKG)_FILE     := ruby-$($(PKG)_VERSION).tar.gz
$(PKG)_URL      := https://cache.ruby-lang.org/pub/ruby/2.3/$($(PKG)_FILE)
$(PKG)_URL_2    := https://cache.ruby-lang.org/pub/ruby/2.3/ruby-2.3.8.tar.gz
$(PKG)_DEPS     := cc yaml libffi openssl1.0  gdbm 

$(PKG)_DEPS_$(BUILD) := autotools 

define $(PKG)_UPDATE
    echo 'TODO: write update script for $(PKG).' >&2;
    echo $($(PKG)_VERSION)
endef

define $(PKG)_BUILD
    # remove previous install - ruby has permissions that don't
    # allow overwriting. 
    rm -rfv '$(PREFIX)/$(TARGET)/include/ruby*'
    rm -rf '$(PREFIX)/$(TARGET)/lib/ruby' 
    rm -rfv '$(PREFIX)/$(TARGET)/lib/pkgconfig/ruby*.pc'
    rm -rf '$(PREFIX)/$(TARGET)/bin/rake.bat'
    rm -rf '$(PREFIX)/$(TARGET)/bin/rake'
    # build and install the library
    export LDFLAGS="-L $(PREFIX)/$(TARGET)/bin -lyaml"
    cd '$(BUILD_DIR)' && $(SOURCE_DIR)/configure \
        $(MXE_CONFIGURE_OPTS) \
        --enable-shared \
        --enable-load-relative \
        --disable-install-doc \
        --without-tk --without-tcllib --without-tcltk 
    $(MAKE) -C '$(BUILD_DIR)' -j '$(JOBS)'
    $(MAKE) -C '$(BUILD_DIR)' -j 1 install
endef

