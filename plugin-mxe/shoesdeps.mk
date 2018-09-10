# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := shoesdeps
$(PKG)_WEBSITE  := https://walkabout.mvmanila.com
$(PKG)_IGNORE   :=
$(PKG)_VERSION  := 1.0
$(PKG)_CHECKSUM := be3cbfc7450fc2eaecd3a552c0acfde12577bd2060e00c247df572c7e116969f
$(PKG)_SUBDIR   := shoesdeps-$($(PKG)_VERSION)
$(PKG)_FILE     := shoesdeps-$($(PKG)_VERSION).tgz
$(PKG)_URL      := https://shoes.mvmanila.com/public/util/($(PKG)_FILE)
$(PKG)_URL_2    := https://shoes.mvmanila.com/public/util/shoesdeps-1.0.tgz
$(PKG)_DEPS     := cc gtk3 sqlite giflib librsvg ruby nsis

$(PKG)_DEPS_$(BUILD) := 

define $(PKG)_UPDATE
    echo 'TODO: write update script for $(PKG).' >&2;
    echo $($(PKG)_VERSION)
endef

define $(PKG)_BUILD
    # just make something exist 
    pwd
    touch '$(PREFIX)/$(TARGET)/bin/shoesdeps'
endef

