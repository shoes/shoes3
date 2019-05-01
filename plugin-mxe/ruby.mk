# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := ruby
$(PKG)_WEBSITE  := https://ruby-lang.org
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 2.5.5
$(PKG)_CHECKSUM := 28a945fdf340e6ba04fc890b98648342e3cccfd6d223a48f3810572f11b2514c
$(PKG)_SUBDIR   := ruby-$($(PKG)_VERSION)
$(PKG)_FILE     := ruby-$($(PKG)_VERSION).tar.gz
$(PKG)_URL      := https://cache.ruby-lang.org/pub/ruby/2.5/$($(PKG)_FILE)
$(PKG)_URL_2    := https://cache.ruby-lang.org/pub/ruby/2.5/ruby-2.5.5.tar.gz
$(PKG)_DEPS     := cc yaml libffi openssl  gdbm 

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

