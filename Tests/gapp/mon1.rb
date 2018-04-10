Shoes.app do 
  stack do
    para "Muliple Monitor Test"
    st = Shoes.settings
    para "Default monitor is #{st.monitor_default}\n"
    if st.monitor_count > 1 
      flow do
        button "Second Monitor" do
          window title: "New Mon", monitor: 1 do
            button "help" do
              alert "Which monitor for alert?"
            end
          end
        end
        button "Move this app" do
          app.monitor = 1;
        end
      end
    end
  end
end
