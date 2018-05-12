Shoes.app(title: "Package app in exe", width: 600, height: 900, resizable: false ) do
require("yaml")
	
	@edit_box_height, @edit_box_width = 28, 250 ### box dimmensions
	@options = [ 0, 0, 0, 0, 0, 3, -1, 1, 0, 1, 1, 1, 1 ]
	@output = [ "app_name", "app_version", "app_startmenu", "publisher", "website", "app_start", "app_loc", "app_ico", "installer_header", "installer_sidebar_bmp", "installer_header_bmp", "app_installer_ico", "license", "include_gems" ]
	@database = []  ##array of all page 1 fields
	@values = Hash[@output.map {|x| [x, ""]}] ##Hash of all user input taken from the database fields
	@values['include_gems']	= [] ##defining an array that will hold all gems
	@must_have = [ 'app_name', 'app_version','app_loc', 'app_start' ] ### @must_have consists of mandatory app options
	@tooltips = [ "Appname and the Subname string will be used to name the installer File",
"Appname and the Subname string will be used to name the installer File",
"Start Menu folder name. If not set Appname will be chosen as default value.",
"Company or organization name",
"Publisher website URL",
"The first script to run for your Shoes app",
"Application directory. Chosen automatically based on starting script selection",
"Window app icon for your Shoes app",
"Installation wizard header name. If empty Appname is chosen as default value.",
"The sidebar image (old bmp) for the installer",
"The header image for the installer",
"The icon for the #{@values['app_name']}-#{@values['app_version']}.exe file - NOT the icon for app desktop",
"Prepend the contents to the Shoes LICSENSE.txt file\nwill be shown to the User at install time, so make it pretty." ]
	
	
	def fix_string str
		length, count, new_str = str.length, 0, ""
		str.each_char { |c| c == "\\" ? new_str << "/" : new_str << c }
		return new_str
	end

	def turn_page direction = "up"
		direction == "up" ? @page+=1 : @page-=1 
		( @page > -1 && @page < @pages.length ) ? ( @frame.clear { @pages[@page].call } ) : nil
	end
	
	def help
		@other_win = window tittle:"Exe builder help" do
			tagline "When installing the app", align: "center"
			image "gui_files/Wizard options.png", left: 10, top: 35, width: 280, height: 240 
			image "gui_files/Wizard options2.png", left: 300, top: 35, width: 280, height: 240 
			tagline "When using the app", align: "center", top: 280
			image "gui_files/exe options.png", left: 50, top: 310, width: 50
			para "<- App icon", left: 100, top: 325
			para "<- App name", left: 100, top: 362
		end
	end
	
	def load_yaml 
    fl = ask_open_file
		if fl
			##loading values from yaml into app vars
		    opts = YAML.load_file(fl)
		    opts.each {|k, v| @values[k] = v}
			##updating text on all fields at page1
			@database.each_with_index { |d, i| d.text = @values["#{@output[i]}"] }
		end
	end
	
	def prerequisites
		if @must_have.any? { |m| @values["#{m}"].nil? || @values["#{m}"] == "" } then
			return 1
		else 
			return 0
		end
	end
	
	def page1
		subtitle "Wizard & application settings", align: "center", top: 10
		stack(left: 50, top: 70, width: 500, height: 750) do
			background darkgray;
			border black, strokewidth: 2; 
			button("Help", left: 350, top: 15, width: 80) { help }
			button("Load yaml", left: 65, top: 15, width: 80, tooltip: "Load existing yaml configuration") { load_yaml }
			line(30,55,470,55)
			para "* - required field", left: 340, top: 60
		end
		
		stack left: 70, top: 170, width: 460, height: 630, scroll: true do
			[ "Application name",    #### arranged the array vertically to make troubleshooting page 1 managable
			"Application subname",
			"Start Menu folder name",
			"Publisher name",
			"Website",
			"Starting script source",
			"Application folder",
			"Exe icon (.ico)",
			"Installer window name",
			"Installer side pic (164 x 309) .bmp",
			"Installer header pic (150 x 57) .bmp",	
			"Installer icon (.ico)",
			"License" ].each_with_index do | item, i |
				req = @must_have.include?(@output[i]) ? "* " : ""
				case @output[i] 
				when 'app_name' then para "Application properties", size: 16
				when 'installer_header' then para "Installation file properties", left: 0, size: 16, margin_top: 10
				end
				flow height: 60 do
					para "#{req}#{item}", left: 20, top: 0, height: @edit_box_height
					@database[i] = edit_line @values[@output[i]], left: 20, top: 28, height: @edit_box_height, width: @edit_box_width, tooltip: @tooltips[i] do
						@values[@output[i]]=@database[i].text
					end
					case @options[i] ## adding ask_folder, ask_file boxes where needed
						when 1 then button("Select file", left: 20 + @edit_box_width, top: 27, width: 100) { @database[i].text = fix_string(ask_open_file) }
						when 2 then button("Select folder", left: 20 + @edit_box_width, top: 27, width: 100) { @database[i].text = fix_string(ask_open_folder) }
						when 3 then @database[i].state = 'disabled';
						button("Select *.rb file", left: 20 + @edit_box_width, top: 27, width: 100) do
							 longfn = fix_string(ask_open_file)
							 @database[i].text = File.basename(longfn)
							 appdir = File.dirname(longfn)
							 @database[6].text = appdir
							 @values['app_loc'] = appdir
						end
						when -1 then @database[i].state = 'disabled'
					end					
				end
			end
		end
	end
	
	def page2
		subtitle "Manage gems", align: "center", top: 5
		stack(left: 50, top: 70, width: 500, height: 750) do
			background darkgray
			border black, strokewidth: 2
			line(30,55,470,55)
		end
		scroll_bug=stack left: 70, top: 170, width: 460, height: 640, scroll: true do
			para "Select aditional gems:", align: "center", size: 16, margin_bottom: 20
			Gem::Specification.each do |gs|
				flow margin_left: 50 do
					check(checked: @values['include_gems'].include?(gs.name) ? true : false ) do |c|
						c.checked? ? @values['include_gems'].push(gs.name) : @values['include_gems'].delete(gs.name)
					end
					para("#{gs.name} #{gs.version}")
				end
			end			
		end
		start { scroll_bug.scroll_top = 1 }  ## this line fixes bug with scroll bar
	end

	def page3
		subtitle "Configuration summary", align: "center", top: 5
		values_exist = {}
		@values.each do |k, v|
			if v != "" and v != nil and !v.empty? then
				values_exist["#{k}"] = v
			end
		end
		stack(left: 50, top: 70, width: 500, height: 750) do
			background darkgray
			border black, strokewidth: 2
			line(30,55,470,55)
			button "Save confg", left: 65, top: 15, width: 80 do
				File.open(ask_save_file, "w") { |f| f.write(values_exist.to_yaml) }
			end
			deploy=button "Deploy", left: 350, top: 15, width: 80 do
				start { deploy.state = 'disabled' }
				#File.open("temp", "w") { |f| f.write(values_exist.to_yaml) }
				#system("cshoes.exe --ruby cmd-merge.rb temp")
        require 'package/merge-exe'
        appdata = ENV['LOCALAPPDATA']
        appdata  = ENV['APPDATA'] if ! appdata
        exes = YAML.load_file("#{appdata}\\Shoes\\package\\util.yaml")
        values_exist['NSIS'] = exes['nsp']
        values_exist['RESH'] = exes['rhp']
        # packdir needs to be forward '/'
        values_exist['packdir'] = "#{appdata.gsub'\\','/'}/Shoes/package/temp"
        th = Thread.new do
          app.cursor = :watch_cursor
          PackShoes::merge_exe(values_exist) {|msg| @stmsg.text = msg }
          app.cursor = :arrow_cursor
        end
			end
      stack left: 70, top: 70, width: 400 do
        flow do
          para "Progress:"
          @stmsg = para "Not started", stroke: green
        end
      end
			para "Yaml output", top: 100, size: 16, align: "center"
		end
		stack left: 70, top: 200, width: 460, height: 640, scroll: true do
			para "#{values_exist.to_yaml}"
		end
		
	end
	
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
