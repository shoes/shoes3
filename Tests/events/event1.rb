Shoes.app do
  event do |evt|
    stderr.puts "event handler called"
    true
  end
  app.events = true
end
