require 'yaml'
Shoes.app do  
  stack do
    tagline "Capture Clicks for another app"
    para "Load this"
    flow do
      @el = edit_line width: 450
      @el.text = "#{DIR}/samples/simple/chipmunk.rb"
      button "select app" do
        path = ask_file_open
        @el.text = path if path
      end
    end
    para "Save events to"
    flow do 
      @sv = edit_line width: 450
      @sv.text = "#{DIR}/events.yaml"
      button "Change" do
        path = ask_file_save 
        @sv.text = path if path
      end
    end
    flow do 
      button "Start app and capture" do
        $capture = []
        eval IO.read(@el.text).force_encoding("UTF-8"), TOPLEVEL_BINDING, @el.text
        w2 = Shoes.APPS[-1]
        x = w2.left
        y = w2.top
        w2.move x+80, y-40
        $base_t = Time.now
        $capture << {captured: @el.text, time: $base_t}
        w2.event = proc do |evt|
          case evt.type
          when :click
            #$stderr.puts "click handler2 with #{evt.type}"
            $capture << { time: (Time.now - $base_t), x: evt.x, y: evt.y}
            evt.accept = true
          else
            evt.accept = true
          end
        end
      end
    end
    flow do 
      button "Stop and save" do
        File.open(@sv.text, 'w') {|f| YAML.dump($capture, f)}
      end
    end
    flow do
      button "Load events" do
        @evts = YAML::load_file(@sv.text)
        puts @evts
        hdr  = @evts[0]
        script = hdr[:captured]
        eval IO.read(script).force_encoding("UTF-8"), TOPLEVEL_BINDING, script
        w2 = Shoes.APPS[-1]
        x = w2.left
        y = w2.top
        w2.move x+80, y-40
     end
      button "Replay" do
        w2 = Shoes.APPS[-1]
        # Hard part here
        #w2.replay_events = proc do
        #end
        delay = 0
        for r in 1..@evts.size
          ev = @evts[r]
          t = ev[:time]
          delay = t if r==1 
          wait = t - delay
          puts wait
          th = Thread.new do
            sleep wait
            th.join
          end
          delay = t
        end
      end
    end
  end
end
