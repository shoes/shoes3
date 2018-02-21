Shoes.app menus: true, width: 300, height: 200 do  
  mb = menubar
  helpmenu = menu "Help"
  aboutitem =  menuitem "About" do
    alert "This is a menu test", title: "About"
  end
  helpmenu << aboutitem
  cobbleritem = menuitem "Cobbler" do Shoes.cobbler end
  helpmenu << cobbleritem
  mb << helpmenu
  button "new app" do
    Shoes.app menus: true, width: 300, height: 200 do 
      para "Shared menus"
    end
  end
end

