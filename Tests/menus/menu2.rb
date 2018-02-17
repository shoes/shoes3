Shoes.app menus: true, width: 300, height: 200 do  
  mb = menubar
  helpmenu = menu "Help"
  aboutitem =  menuitem "About" do
    alert "This is a menu test", title: "About"
  end
  helpmenu << aboutitem
  mb << helpmenu
end

