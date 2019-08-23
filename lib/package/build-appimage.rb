# appimage  build GUI - a lot like the linux GUI. 
Shoes.app(title: "Merge app into Shoes", width: 600, height: 900, resizable: false ) do

	
	#@edit_box_height, @edit_box_width = 28, 250 ### box dimmensions

  # template [ [yaml-name, option, required, prompt, tooltip], ...] in Visual order.
  @template = [
    ['packdir', 2, true, "Directory for output", "holds output"],
    ['app_name', 0, true,"Application Name", "Appname will be used to name the installer File"],
    ["rdomain",  0, true, "Reverse Domain","like com.rbshoes"],
    ["publisher", 0, true,"Publisher Name", "Publisher Name"],
    ["website", 0, true,"Website", "Website url"],
    ["app_start", 3, true,"Starting script", "App start script.rb"],
    ["app_loc", -1, true,"Application directory", "Start script is in here"],
    ["app_png", 0, true,"Icon for application", "Icon must be a png in the folder"],
    ['usexml', 4, true, "Do you want AppStream Metadata?", "App Stores will want this"],
    ['xmlpath', 3, false,"AppStream xml file", "Locate your appdata.xml"], 
    ["maintainer", 0, true,"Maintainer", "Your name of company"],
    ["purpose", 0, true,"Purpose", "Short description of your application"],
    ["no_integrate", 4, false,"Don't Create Desktop menus", "true (for most situations)"],
    ["category", 0, true,"Menu Category", "like Accessories or Office"],
    ["license_tag", 0, true,"Type of license", "MIT is accurate"],
    #["license", 1, true, "Your liscense", "in a txt file"]
  ]

  # this is our callback method
  def call_merge(opts)
    require 'package/merge-appimage'
    th = Thread.new do
      app.cursor = :watch_cursor
      opts['fpm_type'] = 'pacman'
      PackShoes::merge_appimage(opts) {|msg| @stmsg.text = msg }
      app.cursor = :arrow_cursor
    end
  end
	
	require 'package/mergegui'
  create_gui(@template)


end
