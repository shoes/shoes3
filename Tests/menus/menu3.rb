
Shoes.app menus: true do
  def showall
    @eb.text = ""
    mb = menubar
    mb.menus.each do |mn|
      @eb.append "#{mn.title}\n"
      mn.items.each do |item|
        @eb.append "  #{item.title}\n"
      end
    end
  end
  
  def get_index(str)
    if str[/\-\d+|\d+/] 
      str.to_i
    else
      str
    end
  end
  
  stack do
    flow do
      button "Show all" do
        showall
      end
      button "Add Help menu" do
        mb = menubar
        mn = menu "Help"
        mi = menuitem "About"
        mn.append mi
        mb.append mn
        @el1.text = mn.title
        @el2.text = mi.title
        showall
      end
      stack do
        flow do
          para "menu:"
          @el1 = edit_line width: 80
          @el1.text = "Shoes"
          button "Get" do
            q = get_index @el1.text
            mb = menubar
            t = mb[q]
            @eb.text = t.title if t
          end
          button "Remove" do
            # TODO: get item name from @el1
          end
          button "Insert" do
            # TODO: get item name from @el1, position from @el6
          end 
          para "at (-1 is append)"
          @el6 = edit_line width: 40
          @el6.text = '-1'
        end
        flow do
          para "item:"
          @el2 = edit_line width: 80
          button "Get"  do
            mb = menubar
            n = get_index @el1.text
            mn = mb[n]
            q = get_index @el2.text
            t = mn[q]
            @eb.text = "#{mn.title}\n  #{t.title}"
          end
          button "Remove" do
            # TODO: get item name from @el2
          end
          button "Insert" do
            # get item name from @el2, position from @el5
            mb = menubar
            mni = get_index @el1.text
            mn = mb[mni]
            newmi = menuitem @el2.text
            mn.insert newmi, get_index(@el5.text)
            showall
          end 
          para "at (-1 is append)"
          @el5 = edit_line width: 40
          @el5.text = '-1'
        end
     end
    end
    @eb = edit_box width: 400, height: 400
  end
end

