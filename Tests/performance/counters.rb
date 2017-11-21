NUMBER_OF_COUNTERS = 150
FPS = 24

Shoes.app do
   @counter = []
   @text = []
   NUMBER_OF_COUNTERS.times do |n|
      @counter << Time.now
      @text << para
      animate(FPS) do |frames|
         if (0 == (frames % FPS))
            @text[n].text = "FPS #{ FPS / ((frames >= FPS) ? (Time.now - @counter[n]) : 1.0)}\n"
            @counter[n] = Time.now
         end
      end
   end
end
