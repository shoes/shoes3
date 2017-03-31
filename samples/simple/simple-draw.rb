Shoes.app do
   background "#999"
   stroke "#000"
   x = nil
   y = nil
   motion do |_x, _y|
      if x && y && ((x != _x) || (y != _y))
         append do
            line x, y, _x, _y
         end
      end
      x = _x
      y = _y
   end
end
