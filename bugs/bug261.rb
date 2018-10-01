Shoes.app {

   vivaldifnt = font "vivaldi.ttf"
   if vivaldifnt 
     para "This is the #{vivaldifnt[0]} font\n", font: "vivaldi"
   else 
     alert "Failed to load Vivaldi"
   end
   coolfnt = font "#{DIR}/fonts/Coolvetica.ttf"
   if coolfnt
     para "This is the #{coolfnt[0]} font\n", font: coolfnt[0]
   else
     alert "Coolvetica failed to load"
   end
=begin
   lacunafnt = font "#{DIR}/fonts/Lacuna.ttf"
   if lacunafnt
     para "This is the #{lacunafnt[0]} font\n", font: lacunafnt[0]
   else
    alert "Failed to load Lacuna"
   end 
=end
  djfont = font "#{DIR}/fonts/DejaVuSans.ttf"
  if djfont 
     para "This is the #{djfont[0]} font\n", font: djfont[0]
   else
     alert "Dejavu failed to load"
   end
   para "This is the Default font\n"
   # nothing below here is displayed on Linux!
   lcfont = font "LaCosta_Regular.ttf"
   if lcfont 
     para "This is the #{lcfont[0]} font\n", font: lcfont[0]
   else 
     alert "La Costa Regular failed to load"
   end

   para "\n..............................................\n"
   para "#{Shoes::FONTS}\n", font: "Tunga"
   para "\n..............................................\n"
   para "#{Shoes::FONTS}\n"
   para "\n..............................................\n"
   button "quit" do Shoes.quit end 

}
