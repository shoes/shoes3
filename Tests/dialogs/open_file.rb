Shoes.app do
  para "Open" 
  button "Open" do
    fp = ask_open_file
    para "return #{fp}"
  end
end
