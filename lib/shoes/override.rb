module Override
   def self.extended(mod)
      def mod.list_box(args = {})
         l = args[:left]
         t = args[:top]
         w = args[:width]
         h = 20
         w ||= 200
         bcolor = rgb(123, 158, 189)
         selected = []
         fimg = nil
         bimg = nil

         f = flow left: l, top: t, width: w, height: h do
            border bcolor
            selected[0] = inscription
            fimg = image "#{DIR}/static/listbox_button1.png", left: w - 17, top: 2
            bimg = image("#{DIR}/static/listbox_button2.png", left: w - 17, top: 2).hide
            fimg.hover{bimg.show}
            bimg.leave{bimg.hide}
            bimg.click{bimg.show}
         end

         rects = []
         inscs = []
         args[:items].length.times do |i|
            x = l
            y = t + (i + 1) * h
            r = rect(x, y, w - 1, h, stroke: bcolor, fill: white).hide
            s = inscription(args[:items][i], left: x, top: y).hide
            r.hover{r.style fill: blue}
            r.leave{r.style fill: white}
            r.click{selected[0].text = s.text; selected[1] = r}
            rects << r
            inscs << s
         end

         f.click do
            rects.each{|r| r.toggle; r.style(fill: blue) if r == selected[1]}
            inscs.each{|i| i.toggle}
         end
      end
   end
end
