#
# a translation from a processing example
# http://vormplus.be/weging/an-introduction-to-processing/
#
Shoes.app width: 420, height: 420, resizable: false do
   stage = 0
   wide = 3.0
   sw = 1.0
   basesize = 600
   step = 60
   stroke gray(127)
   nofill

   animate 40 do |i|
      stage = (1...8).rand if i % 40 == 0
      rotation = -(Shoes::HALF_PI / wide)
      clear do
         background gray(240)
         10.times do |i|
            strokewidth sw * i
            size = (basesize / 3) + ((step / 3) * i)
            shape do
               arc width / 2, height / 2,
                   size, size,
                   rotation * i, rotation * i + Shoes::TWO_PI - Shoes::HALF_PI
            end
         end
      end

      case stage
      when 1 then wide -= 0.1
      when 2 then wide += 0.1
      when 3 then basesize -= 1
      when 4 then basesize += 2
      when 5 then sw += 0.1
      when 6 then sw -= 0.01
      when 7 then step += 2
      else    step -= 1
      end
   end
end
