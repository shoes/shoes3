#  Build a fpm for .deb 
Shoes.app(title: "Deb Merge", width: 600, height: 900, resizable: false ) do
  #require("yaml")
	
	#@edit_box_height, @edit_box_width = 28, 250 ### box dimmensions

  # template [ [yaml-name, option, required, prompt, tooltip], ...] in Visual order.
  @template = [
    ['packdir', 2, true, "Directory for output", "holds output"],
    ['app_name', 0, true,"Application Name", "Appname will be used to name the installer File"],
    ["app_version",  0, true, "Application subname","version string or code name"],
    ["publisher", 0, true,"Publisher Name", "Publisher Name"],
    ["website", 0, true,"Website", "Website url"],
    ["app_start", 3, true,"Starting script", "App start script.rb"],
    ["app_loc", -1, true,"Application directory", "Start script is in here"],
    ["app_png", 0, true,"Icon for application", "Icon must be a png in the folder"],
    ["maintainer", 0, true,"Maintainer", "Your name of company"],
    ["purpose", 0, true,"Purpose", "Short description of your application"],
    ["linux_where", 0, true, "Install into", "/usr/local is a very good choice"],
    ["create_menu", 4, true,"Create Desktop menu", "true is the best answer"],
    ["category", 0, true,"Menu Category", "like Accessories or Office"],
    ["license_tag", 0, true,"Type of license", "LGPL, Berkely, Commercial..."],
    ["license", 1, true, "Your liscense", "in a txt file"]
  ]

  # this is our callback method
  def call_merge(opts)
    require 'package/merge-fpm'
    th = Thread.new do
      app.cursor = :watch_cursor
      opts['fpm_type'] = 'deb'
      PackShoes::merge_fpm(opts) {|msg| @stmsg.text = msg }
      app.cursor = :arrow_cursor
    end
  end
	
	require 'package/mergegui'
  create_gui(@template)


end
