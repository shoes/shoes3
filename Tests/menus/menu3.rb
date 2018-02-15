
Shoes.app menus: true do
  mb = app.menubar
  # add Help and Help->About to menubar
  helpmenu = menu "Help"
  helpmenu << menuitem "About" do
   alert "Version 12 of Menu Test App"
  end
  mb << helpmenu
  # replace default File -> Quit handler  with our block
  filemenu = mb[0]
  filemenu.each do |fmi|
    if fmi.name == 'Quit' 
      fmi = menuitem "Musical Quit", key: 'control_q' do
        alert "I can't quit her - \"Blood, Sweat and Tears\". Use decoration"
      end
    end
  end
end

