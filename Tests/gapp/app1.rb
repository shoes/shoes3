Shoes.app do 
  stack do
    para "Check if we can access shoes.yaml globals"
    button "Check" do
      @st = Shoes.settings
      @eb.append "Have settings for #{@st}\n"
    end 
    @eb = edit_box width: 400, height: 300
  end
end
