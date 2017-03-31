# https://github.com/casparjones/First-Ruby-Steps/blob/master/shoes-samples/simple-curve.rb
# http://www.cairographics.org/samples/curve_to/

# curve_to with visible control points.
# The ovals represent the control points.
Shoes.app(title: "Expert curve_to control points", width: 500, height: 400) do
   xy = [
      [width / 8, height / 2],
      [width / 8 * 3, height / 4 * 3],
      [width / 8 * 4, height / 4],
      [width / 8 * 7, height / 2]
   ]

   para "click and drag control points..."
   fill green(0.2)
   move_to *xy[0]

   @controller = nil
   xy.each do |n|
      (@controllers ||= []) << oval(left: n[0], top: n[1], radius: 10, center: true)
   end
   @controllers.each do |n|
      n.click do |_btn, _left, _top|
         n.style fill: red
         @controller = n
      end
      n.release do |_btn, _left, _top|
         n.style fill: green(0.2)
         @controller = nil
      end
   end

   motion do |left, top|
      unless @controller.nil?
         xy[@controllers.index(@controller)] = left, top
         @controller.left = left
         @controller.top = top
      end
   end

   @stack = stack top: 0, left: 0

   animate(10) do
      @stack.clear do
         fill red(0.2)
         shape do
            move_to *xy[0]
            curve_to *xy[1], *xy[2], *xy[3]
         end
      end
   end
end
