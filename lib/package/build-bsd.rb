Shoes.app(title: "Package app with shoes in .deb", width: 600, height: 900, resizable: false ) do
  require("yaml")
	
	@edit_box_height, @edit_box_width = 28, 250 ### box dimmensions
  # template [ [yaml-name, option, prompt, tooltip], ...] in Visual order.
  @template = [
    ['packdir', 2, "Directory for output", "holds output"],
    ['app_name', 0, "Application Name", "Appname and the Subname string will be used to name the installer File"],
    ["app_version",  0,  "Application subname","Appname and the Subname string will be used to name the installer File"],
    ["publisher", 0, "Publisher Name", "Publisher Name"],
    ["website", 0, "Website", "Website url"],
    ["app_start", 3, "Starting script", "App start script.rb"],
    ["app_loc", -1, "Application directory", "Start script is in here"],
    ["app_png", 0, "Icon for application", "Icon must be a png in the folder"],
    ["maintainer", 0, "Maintainer", "Your name of company"],
    ["purpose", 0, "Purpose", "Short description of your application"],
    ["linux_where", 0, "Install into", "/usr/local is a very good choice"],
    ["create_menu", 4, "Create Desktop menu", "true is the best answer"],
    ["category", 0, "Menu Category", "like Accessories or Office"],
    ["license_tag", 0, "Type of license", "LGPL, Berkely, Commercial..."],
    ["license", 1, "Your liscense", "in a txt file"]
  ]
  # decompose template into @output, @options, @prompts, @tooltips, @must_haves=true
  @output = []; @options = []; @prompts = []; @tooltips = []; @must_have = [];
  @template.each do |fld|
    @output << fld[0]
    @options << fld[1]
    @prompts << fld[2]
    @tooltips << fld[3]
    @must_have << fld[0]  # all are required
  end
  # add include_gems to @output ?
	@database = []  ##array of all page 1 fields
	@values = Hash[@output.map {|x| [x, ""]}] ##Hash of all user input taken from the database fields
	@values['include_gems']	= [] ##defining an array that will hold all gems
	
	require 'package/bld'
  
	background dimgray
	@pages = [ method(:page1),method(:page2), method(:page3) ]
	@page = 0
	@frame = flow(height: 800) { @pages[@page].call }
	@previous = button "Previous", left: 200, top: 85, width: 80 do
		turn_page "down"
		@next.show
		@page == 0 ? @previous.hide : nil
	end
	start { @previous.hide } ##hiding previous button on page 1
	@next = button "Next", left: 295, top: 85, width: 80 do
		if @page == 0 && prerequisites == 1 then
			alert "One or more required fields are empty!"
		else
			turn_page "up"
			@previous.show
			@page == @pages.length-1 ? @next.hide : nil
		end
	end
	@previous.hide
end
