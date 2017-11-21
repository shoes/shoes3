NUMBER_OF_SHAPES = 192

Shoes.app do
   @shapes = []

   NUMBER_OF_SHAPES.times do
      fill rgb(rand(255), rand(255), rand(255))
      @shapes << oval(rand(self.width), rand(self.height), rand(100))
   end

   animate(60) do
      @shapes.each do |shape|
         mx = rand > 0.5 ? +1 : -1
         my = rand > 0.5 ? +1 : -1
         shape.move shape.left + mx, shape.top + my
      end
   end
end
