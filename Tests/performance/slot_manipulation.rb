FPS = 10000

IMAGES = [
   "shoes-icon.png",
   "shoes-icon-blue.png",
   "shoes-icon-federales.png",
   "shoes-icon-red.png"
]

Shoes.app do
   @counter = 0
   @slot = flow { para "### #{@counter += 1} ###" }

   def add_element
      case rand(5)
         when 0
            para("### #{@counter += 1} ###")
         when 1
            button("### #{@counter += 1} ###")
         when 2
            image("#{DIR}/static/#{IMAGES.sample}")
         when 3
            spinner start: (1 == rand(2) ? true : false)
         when 4
            switch active: (1 == rand(2) ? true : false)
      end
   end

   animate(FPS) do |frame|
      case rand(7)
         when 0
            @slot.append { add_element }
         when 1
            n = rand(@slot.children.size)
            @slot.after(@slot.children[n]) { add_element }
         when 2
            n = rand(@slot.children.size)
            @slot.before(@slot.children[n]) { add_element }
         when 3
            @slot.clear if rand < 0.01
         when 4
            @slot.clear { add_element } if rand < 0.01
         when 5
            @slot.prepend { add_element }
         when 6
            @slot.refresh
      end
   end
end
