require 'yaml'
Shoes.app do  
  stack do
    tagline "Replay Clicks for another app"
    para "Load events from"
    flow do 
      @sv = edit_line width: 450
      @sv.text = "#{DIR}/events.yaml"
      button "Change" do
        path = ask_file_open
        @sv.text = path if path
      end
    end

    flow do
      button "Start App" do
        evtflhash = YAML::load_file(@sv.text)
        hdr = evtflhash[:context]
        puts hdr
        script = hdr[:app]
        eval IO.read(script).force_encoding("UTF-8"), TOPLEVEL_BINDING, script
        w2 = Shoes.APPS[-1]
        x = w2.left
        y = w2.top
        w2.move x+80, y-40
        @events = evtflhash[:events]
        puts @events
      end
    end
    button "Replay Events" do
      w2 = Shoes.APPS[-1]
      delay = 0
      @events.each_index do |r|
        ev = @events[r]
        t = ev[:time]
        delay = t if r == 0
        wait = t - delay
        timer(wait) do
          puts "waited for #{wait}"
          puts "  move to #{ev[:x]}, #{ev[:y]} and click "
          # want to call: w2.replay_event(:click, ev)
          # need native move_cursor ability
        end
        delay = t
      end
    end
  end
end
