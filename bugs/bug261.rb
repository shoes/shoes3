Shoes.app {
   vivaldifnt = font "vivaldi.ttf"
   lacunafnt = font "#{DIR}/fonts/Lacuna.ttf"
   para "This is the #{vivaldifnt[0]} font\n", font: vivaldifnt[0]
   para "This is the Default font\n"
   para "This is the #{lacunafnt[0]} font\n", font: "Lacuna"
   #para "#{Shoes::FONTS}\n", font: "Vivaldi"
   para "#{Shoes::FONTS}\n", font: "Tunga"
   para "#{Shoes::FONTS}\n"
   para "\n..............................................\n"
   button "quit" do Shoes.quit end 
}
