include FileUtils

class MakeLinux
  extend Make

  class << self

    # appstream metadata is not useable? glibc dependcies on my system
    def make_installer_appimage(name, arch, iconpath)
      puts "Creating AppImage Appdir"
      pkg = ""
      if APP['Bld_Pre']
        pkg = "#{APP['Bld_Pre']}pkg/#{TGT_ARCH}/AppDir"
        mkdir_p pkg
      else
        pkg = "pkg/#{TGT_ARCH}/AppDir"
      end
      rm_rf pkg
      mkdir_p pkg               
      # 'AppRun' link to launch shoes script
      Dir.chdir(pkg) do
        ln_s "usr/lib/#{TGT_ARCH}/#{name}", "AppRun"
      end
      # <id> is a big deal with AppImage and AppStream
      id = "com.mvmanila.#{name}_#{arch}"
      mkdir_p "#{pkg}/usr/bin"
      mkdir_p "#{pkg}/usr/lib"
      mkdir_p "#{pkg}/usr/share/applications"
      mkdir_p "#{pkg}/usr/share/icons/hicolor/256x256"
      mkdir_p "#{pkg}/usr/share/metainfo"
      sh "cp -r #{TGT_DIR} #{pkg}/usr/lib"
      rm_rf "#{pkg}/usr/lib/#{TGT_ARCH}/tmp"
      make_desktop_appimg pkg, id
      cp "#{pkg}/#{id}.desktop", "#{pkg}/usr/share/applications/"
      cp "#{TGT_DIR}/static/app-icon.png", "#{pkg}/#{name}.png"
      cp "#{TGT_DIR}/static/app-icon.png", "#{pkg}/usr/share/icons/hicolor/256x256/#{name}.png"
      # rewrite XML template
      rewrite "static/stubs/shoes.appdata.xml", "#{pkg}/usr/share/metainfo/#{id}.appdata.xml",
          %r!APPID!, id
      #cp "static/stubs/shoes.appdata.xml", "#{pkg}/usr/share/metainfo/#{id}.appdata.xml"
      dest = "pkg/Shoes-#{APP['VERSION']}-#{arch}.appimage"
      rm_rf dest
      #sh "mksquashfs #{pkg} pkg/shoes.squashfs -root-owned -noappend"
      #sh "cat bin/runtime.x86_64 >>#{dest}"
      #sh "cat pkg/shoes.squashfs >>#{dest}"
      #rm_rf "pkg/shoes.squashfs"
      #sh "chmod +x #{dest}"
      #sh "appimagetool  #{pkg} #{dest}"
      sh "appimagetool #{pkg} #{dest} --runtime-file bin/runtime.#{arch}"
      #rm_rf pkg
   end

   def make_desktop_appimg(dir, id)
      File.open("#{dir}/#{id}.desktop",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec=shoes %f\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Ruby Graphical Programming\n"
        f << "Icon=shoes\n"
        f << "Categories=Development;\n"
        f << "X-AppImage-Integrate=false\n"
      end
    end
     

    # make a .install with all the bits and pieces. 
    def make_installer_makeself(arch)
      gtkv = '3'
      #arch = 'x86_64'
      appname =  "#{APP['name'].downcase}"
      rlname = "#{appname}-#{APP['VERSION']}-gtk#{gtkv}-#{arch}"
      #puts "Creating Pkg for #{rlname}"
      pkg = ""
      if APP['Bld_Pre']
        pkg = "#{APP['Bld_Pre']}pkg"
        mkdir_p pkg
      else
        pkg = 'pkg'
      end
      rm_r "#{pkg}/#{rlname}" if File.exists? "#{pkg}/#{rlname}"
      cp_r "VERSION.txt", "#{TGT_DIR}"
      mkdir_p "#{pkg}/#{rlname}"
      sh "cp -r #{TGT_DIR}/* #{pkg}/#{rlname}"
      Dir.chdir "#{pkg}/#{rlname}" do
        rm_r "#{APP['Bld_Tmp']}"
        rm_r "pkg" if File.exist? "pkg"
        make_desktop_makeself
        make_uninstall_script
        make_install_script
        make_smaller unless APP['GDB']
      end
      Dir.chdir "#{pkg}" do
        puts `pwd`
        sh "makeself #{rlname} #{rlname}.install #{appname} ./shoes-install.sh "
      end
      if APP['Bld_Pre']
        # copy installer to the shoes3 source pkg/ dir (on an nfs server?)
        cp "#{pkg}/#{rlname}.install", "pkg"
      end
    end
    
    def make_desktop_makeself
      File.open("Shoes.desktop.tmpl",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec={hdir}/.shoes/#{APP['NAME']}/shoes\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Ruby Graphical Programming\n"
        f << "Icon={hdir}/.shoes/#{APP['NAME']}/static/app-icon.png\n"
        f << "Categories=Development;\n"
      end
      File.open("Shoes.remove.tmpl",'w') do |f|
        f << "[Desktop Entry]\n"
        f << "Name=Uninstall Shoes #{APP['NAME'].capitalize}\n"
        f << "Exec={hdir}/.shoes/#{APP['NAME']}/shoes-uninstall.sh\n"
        f << "StartupNotify=true\n"
        f << "Terminal=false\n"
        f << "Type=Application\n"
        f << "Comment=Delete Shoes\n"
        f << "Icon={hdir}/.shoes/#{APP['NAME']}/static/app-icon.png\n"
        f << "Categories=Development;\n"
      end
    end
    
    def make_uninstall_script
      File.open("shoes-uninstall.sh", 'w') do |f|
        f << "#!/bin/bash\n"
        f << "#pwd\n"
        f << "cd $HOME/.shoes/#{APP['NAME']}\n"
        f << "xdg-desktop-menu uninstall Shoes.remove.desktop\n"
        f << "xdg-desktop-menu uninstall Shoes.desktop\n"
        f << "cd ../\n"
        f << "rm -rf #{APP['NAME']}\n"
      end
      chmod "+x", "shoes-uninstall.sh"
    end
    
    # the install script that runs on the user's system can be simple. 
    # Copy things from where it's run to ~/.shoes/federales/ and then
    # sed the desktop file and copy it with xdg-desktop-menu
    def make_install_script
      File.open("shoes-install.sh", 'w') do |f|
        f << "#!/bin/bash\n"
        f << "#pwd\n"
        f << "ddir=$HOME/.shoes/#{APP['NAME']}\n"
        f << "#echo $ddir\n"
        f << "mkdir -p $ddir\n"
        f << "cp -r * $ddir/\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.desktop.tmpl >Shoes.desktop\n"
        f << "cp Shoes.desktop $ddir/Shoes.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.desktop\n"
        f << "sed -e \"s@{hdir}@$HOME@\" <Shoes.remove.tmpl >Shoes.remove.desktop\n"
        f << "cp Shoes.remove.desktop $ddir/Shoes.remove.desktop\n"
        f << "xdg-desktop-menu install --novendor Shoes.remove.desktop\n"
        f << "echo \"Shoes has been copied to $ddir. and menus created\"\n"
        f << "echo \"If you don't see Shoes in the menu, logout and login\"\n"
      end
      chmod "+x", "shoes-install.sh"
    end
    
    # run strip on the libraries, remove unneeded ruby code (tk,
    #  readline and more)
    def make_smaller
      puts "Shrinking #{`pwd`}"
      sh "strip *.so"
      sh "strip *.so.*"
      Dir.glob("lib/ruby/**/*.so").each {|lib| sh "strip #{lib}"}
    end
  end
end
