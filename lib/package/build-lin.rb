Shoes.app(title: "Package app with shoes in .deb", width: 600, height: 900, resizable: false ) do
require("yaml")
	
	@edit_box_height, @edit_box_width = 28, 250 ### box dimmensions
  # template [ [yaml-name, option, prompt, tooltip], ...] in Visual order.
  @template = [
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
		@other_win = window tittle:"Deb builder help" do
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
			@database.each_with_index do |d, i|
        if @options[i] == 4  # check box
          d.checked =  @values["#{@output[i]}"]
        else
          d.text = @values["#{@output[i]}"]
        end
      end
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
			##button("Help", left: 350, top: 15, width: 80) { help }
			button("Load yaml", left: 65, top: 15, width: 80, tooltip: "Load existing yaml configuration") { load_yaml }
			line(30,55,470,55)
			para "* - required field", left: 340, top: 60
		end
		
		stack left: 70, top: 170, width: 460, height: 630, scroll: true do
      @prompts.each_with_index do |item, i|
				req = @must_have.include?(@output[i]) ? "* " : ""
				flow height: 60 do
					para "#{req}#{item}", left: 20, top: 0, height: @edit_box_height
          case @options[i] # checkbox or editline? 
            when 4 then
              @database[i] = check checked: @values[@output[i]], left: 20, top: 28, tooltip: @tooltips[i] do |ck| 
                @values[@output[i]] = ck.checked?
              end
            else
					    @database[i] = edit_line @values[@output[i]], left: 20, top: 28, height: @edit_box_height, width: @edit_box_width, tooltip: @tooltips[i] do
						    @values[@output[i]]=@database[i].text
              end
          end
					case @options[i] ## adding ask_folder, ask_file boxes where needed
						when 1 then button("Select file", left: 20 + @edit_box_width, top: 27, width: 100, margin: 3) { @database[i].text = fix_string(ask_open_file) }
						when 2 then button("Select folder", left: 20 + @edit_box_width, top: 27, width: 100, margin: 3) { @database[i].text = fix_string(ask_open_folder) }
						when 3 then @database[i].state = 'disabled';
						button("Select *.rb file", left: 20 + @edit_box_width, top: 27, width: 120, margin: 3) do
							 longfn = fix_string(ask_open_file)
							 @database[i].text = File.basename(longfn)
							 appdir = File.dirname(longfn)
							 @database[5].text = appdir
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
      if ([true, false].include? v) 
         # a boolean - check box
         values_exist["#{k}"] = v
			elsif v != "" and v != nil and !v.empty? then
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
        require 'package/merge-lin'
        th = Thread.new do
          app.cursor = :watch_cursor
          PackShoes::merge_linux(values_exist) {|msg| @stmsg.text = msg }
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
