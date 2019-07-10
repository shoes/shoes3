Shoes.app do
  stack do
    flow do
		  para "Start Here: "
		  @el = edit_line 
		  #@el.text = Dir.getwd
		  button "Open" do
		    fp = ask_open_file title: "My App wants", dir: @el.text, 
            types: {"Ruby" => "*.rb", "All" => "*"}, hidden: true 
		    if fp 
		      @el.text = File.dirname(fp)
		      @msgbox.append fp+"\n"
		    end
		  end
		end
	  @msgbox = edit_box width: 300
	end
end
