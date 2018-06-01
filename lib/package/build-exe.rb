Shoes.app(title: "Package app in exe", width: 600, height: 900, resizable: false ) do

  # template [ [yaml-name, option, prompt, tooltip], ...] in Visual order.
  @template = [
    ['packdir', 2, true, "Folder for output", "path to output folder"],
    ['app_name', 0, true, "Application Name", "Appname and the Subname string will be used to name the installer File"],
    ["app_version", 0, true, "Application subname","Appname and the Subname string will be used to name the installer File"],
    ["app_startmenu", 0, false,"Start Menu folder name", "Start Menu folder name. If not set Appname will be chosen as default value."],
    ["publisher", 0, false,	"Publisher name", "Company or organization name"],
    ["website", 0, false, "Website", "Publisher website URL"],
    ["app_start", 3, true, "Starting script source", "The first script to run for your Shoes app"],
    ["app_loc", -1, true, "Application folder", "Application directory. Chosen automatically based on starting script selection"],
    ["app_ico", 1, false,  "Exe icon (.ico)" ,"Window app icon for your Shoes app"],
    ["installer_header", 0, false, "Installer window name", "Installation wizard header name. If empty Appname is chosen as default value."],
    ["installer_sidebar_bmp", 1, false, "Installer side pic (164 x 309) .bmp", "The sidebar image (old bmp) for the installer"],
    ["installer_header_bmp", 1, false, "Installer header pic (150 x 57) .bmp", "The header image for the installer"],
    ["app_installer_ico", 1, false, "Installer icon (.ico)", "The icon for the installer exe file - NOT the icon for app desktop"],
    ["license", 1, "License", false, "Prepend the contents to the Shoes LICSENSE.txt file\nwill be shown to the User at install time, so make it pretty." ]
  ]

  # Our callback method
  def call_merge(opts)
    require 'package/merge-exe'
    appdata = ENV['LOCALAPPDATA']
    appdata  = ENV['APPDATA'] if ! appdata
    exes = YAML.load_file("#{appdata}\\Shoes\\package\\util.yaml")
    opts['NSIS'] = exes['nsp']
    opts['RESH'] = exes['rhp']
    th = Thread.new do
      app.cursor = :watch_cursor
      PackShoes::merge_exe(opts) {|msg| @stmsg.text = msg }
      app.cursor = :arrow_cursor
    end
  end
	
	require 'package/mergegui'
  create_gui(@template)

end
