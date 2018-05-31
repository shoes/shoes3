Shoes.app(title: "Package app with shoes in .dmg", width: 600, height: 900, resizable: false ) do

  @template = [
    ['packdir', 2, true, "Where to place output", "Directory for output"],
    ['app_name', 0, true, "Application Name", "Appname and the Subname string will be used to name the installer File"],
    ["app_version",  0, true, "Application subname","Appname and the Subname string will be used to name the installer File"],
    ["publisher", 0, false, "Publisher Name", "Publisher Name"],
    ["website", 0, false, "Website", "Website url"],
    ["app_start", 3,true, "Starting script", "App start script.rb"],
    ["app_loc", -1, true, "Application directory", "Start script is in here"],
    ["app_png", 0, true, "Icon for application", "Icon must be a png in the folder"],
    ["maintainer", 0, false, "Maintainer", "Your name or email"],
    ['osx_identifier', 0, true, "CFBundleIdentifier", "com.mvmanila is default"],
    ["license", 1, true, "Your license", "in a txt file"],
    ['app_icns', 1, true, "OSX icns file", "osx icns file"],
    ['dmg_background', 1, true, "Installer background PNG", "background png file"]
  ]
   
   # this is our callback 
  def call_merge(opts)
    require 'package/merge-osx'
 	opts['shoes_at'] = DIR
	opts['target_ruby'] = RUBY_VERSION
	opts['target_ruby_arch'] = RbConfig::CONFIG['arch'] # 'x86_64-darwin-14'
    th = Thread.new do
      app.cursor = :watch_cursor
      PackShoes::merge_osx(opts) {|msg| @stmsg.text = msg }
      app.cursor = :arrow_cursor
    end
  end
	
  require 'package/mergegui'
  create_gui(@template)
  
end
