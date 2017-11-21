IMAGES = [
   "shoes-icon.png",
   "shoes-icon-blue.png",
   "shoes-icon-federales.png",
   "shoes-icon-red.png"
]

NUMBER_OF_IMAGES = 117

Shoes.app do
   @images = []
   @interpolator = (tmp = (0..50).collect { |n| -n }) + tmp.reverse

   NUMBER_OF_IMAGES.times do
      @images << image("#{DIR}/static/#{IMAGES.sample}")
   end

   @counter = 1
   animate(@fps = 60) do |frame|
      @images.each { |img| img.rotate(@interpolator.first) }
      if ((frame / @fps) == @counter)
         @counter += 1
         @interpolator.push @interpolator.shift
      end
   end
end
