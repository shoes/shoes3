Shoes.app do 
  stack do
    para "Check if we can access shoes.yaml settings"
    button "Check" do
      st = Shoes.settings
      @eb.append "Name: #{st.app_name}\n"
      @eb.append "Have settings for #{st}\n"
      if RUBY_PLATFORM =~ /(linux)|(bsd)/
        @eb.append "Dbus: #{st.dbus}\n"
        @eb.append "RDomain: #{st.rdomain}\n"
      end
      @eb.append "Theme: #{st.theme}\n"
      @eb.append "MDI: #{st.mdi}\n"
      @eb.append "Menus: #{st.use_menus}\n"
      moncnt = st.monitors.length
      if moncnt > 0
        @eb.append "Have #{moncnt} monitor\n"
      else
        @eb.append "Monitors not defined yet\n"
      end
    end 
    @eb = edit_box width: 400, height: 300
  end
end
