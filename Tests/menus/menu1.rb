Shoes.app menus: true do
  @mb = menubar
  @filem = @mb.menus[0]
  @item = @filem[0]
  stack do
    button "Disable #{@item.title}" do
      @item.enable = false
    end
    flow do
      button "Change Open to" do
        #@item = @filem[2]
        @item.title = @name.text
      end
      @name = edit_line
      @name.text = "Foo"
    end
  end
end
