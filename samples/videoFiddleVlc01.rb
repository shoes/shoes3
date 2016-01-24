# encoding: UTF-8

class Shoes::Knob < Shoes::Widget
    attr_reader :slot, :fraction, :tint
    attr_accessor :range, :tick, :active

    def initialize(driven, opts = {})
        
        @driven = driven
        
        @active = true
        @range = opts[:range] || 0..100
        @fraction = opts[:fraction] || 80
        @click_proc = opts[:click]
        
        @tint = opts[:color] || lawngreen
        @tick = opts[:tick] || 10
        @size = opts[:size] || 1
        @padding = opts[:padding] || 0
        self.width = (50*@size).to_i + @padding     
        @cx, @cy = self.left + (25*@size) + @padding, self.top + (25*@size)
        
        @canvas = flow :height => (50*@size).to_i, click: @click_proc,
                    hover: proc {|s| s.cursor = :hand}, leave: proc {|s| s.cursor = :arrow} do
            
            fill black
            stroke white
            oval :left => @cx - (20*@size), :top => @cy - (20*@size), :radius => (20*@size)
            
            nofill
            oval :left => @cx - (12*@size), :top => @cy - (12*@size), :radius => (12*@size)
            
            strokewidth 1
            (ticks + 1).times do |i|
                radial_line 225 + ((270.0 / ticks) * i), (12*@size)..(15*@size)
            end
            
            strokewidth 2
            stroke @tint
            fill @tint
            @ovl = oval :left => @cx - (3*@size), :top => @cy - (3*@size), :radius => (3*@size)
            
            @needle = radial_line 225 + ((270.0 / @range.end) * @fraction), 0..(12*@size)
        end
        
    end
    
    def tint=(color)
        @tint = color
        @needle.remove
        @ovl.remove
        
        @canvas.append do
            strokewidth 2
            stroke @tint
            fill @tint
            @ovl = oval :left => @cx - (3*@size), :top => @cy - (3*@size), :radius => (3*@size)
            @needle = radial_line 225 + ((270.0 / @range.end) * @fraction), 0..(12*@size)
        end
    end
    
    def ticks; @range.end / @tick end

    def radial_line(deg, r)
        pos = ((deg / 360.0) * (2.0 * Math::PI)) - (Math::PI / 2.0)
        line (Math.cos(pos) * r.begin) + @cx, (Math.sin(pos) * r.begin) + @cy,
             (Math.cos(pos) * r.end) + @cx, (Math.sin(pos) * r.end) + @cy
    end

    def fraction= pos
        @fraction = pos
        @needle.remove
        
        @canvas.append do
            strokewidth 2
            stroke @tint
            fill @tint
            @needle = radial_line 225 + ((270.0 / @range.end) * @fraction), 0..(12*@size)
        end
    end
    
    # Calling bound method of the driven element.
    # args is an array of the arguments given to the method
    # if method has more than one argument, the driven argument is the one, in the array, marqued as "driver",
    #       ie: for shape.displace(x,y) --> tweak(left, ["driver", 5]) 
    #       displace.x beeing driven by knob, capturing mouse's left coordinates'
    # other arguments are given default values 
    def tweak(origin_left, args = nil)
        @released = false
        origin_pos = @fraction
        
        self.app.motion do |lf,t|
            pos =(origin_pos - (origin_left - lf))
            if @active and not @released and @range.cover?(pos)
                self.fraction= pos
                if args
                    @driven.call( *args.map { |v| v = @fraction if v == "driver" } )
                else
                    @driven.call @fraction
                end
            end
        end
        self.app.release {|b,l,t| @released = true}
    end
end


require 'shoes/videoffi'
Vlc.load_lib  #'/usr/lib/libvlc.so.5.5.0', #"./libvlc.so" 

Shoes.app width: 625, height: 540, resizable: false do
    LinkStyleStopped = [Shoes::Link, stroke: black, underline: "none"]
    LinkStyleStoppeddHover = [Shoes::LinkHover, stroke: darkred, underline: "none"]
    style(*LinkStyleStopped); style(*LinkStyleStoppeddHover)
    CtrlsText = "    ",
                link("play")  { play_media }    , "   ",
                link("pause") { toggle_media }  , "   ",
                link("stop")  { stop_media }    , "      ",
                link("hide")  { @svlc.hide }    , "   ",
                link("show")  { @svlc.show }    , "      ",
                link("+5 sec") { @svlc.time += 5000 } , "   ",
                link("-5 sec") { @svlc.time -= 5000 } , "   "
    
    stack do
        @info = para "", margin_left: 25
        @cont = stack width: 600, height: 400 do
            @svlc = video "", width: 600, height: 400, margin_left: 25, autoplay: true
        end
        
        @timeline = progress width: 1.0, height: 10, margin: [25,0,25,0]
        
        @controls = flow margin: [0,10,0,0] do
            @bckgrd = background ghostwhite
            @ctrls = para CtrlsText
            para "    autoplay :" 
            @chk = check checked: @svlc.autoplay, margin: [0,0,10,0] do |c| 
                @svlc.autoplay = c.checked? ? true : false
            end
            
            driven = @svlc.method(:volume=)
            @vol_knob = knob driven, fraction: 75, padding: 20, size: 0.75, color: red,
                            click: proc { |but,left,t| @vol_knob.tweak(left) }
        end
        
        flow margin: [10,10,0,5] do
            button "Open file" do
                @anim.stop
                file = ask_open_file
                unless file.nil?
                    @svlc.path = file 
                    @info.text = ""
                    @svlc.autoplay ? set_controls : reset_controls
                    @anim.start
                end
            end
            
            button "Quit", margin_left: 50 do; exit end;
        end
        
    end
    
    def set_controls(color=gray)
        @bckgrd.fill = color
        @controls.style(Shoes::LinkHover, stroke: black, underline: "none")
        @controls.style(Shoes::Link, stroke: lawngreen, underline: "none")
        @controls.refresh_slot
        @ctrls.text = CtrlsText # won't refresh until mouse hovering, otherwise 
        @vol_knob.tint = lawngreen
    end
    
    def reset_controls(color=ghostwhite)
        @bckgrd.fill = color
        @controls.style(*LinkStyleStoppeddHover)
        @controls.style(*LinkStyleStopped)
        @controls.refresh_slot
        @vol_knob.tint = red
    end
    
    def play_media
        @svlc.play
        set_controls 
        @anim.start
    end
    
    def toggle_media
        @svlc.playing? ? reset_controls(rgb(170,170,170)) : set_controls
        @svlc.pause
    end
    
    def stop_media
        @svlc.stop
        reset_controls
        @anim.stop
    end
    
    
    set_controls if @svlc.loaded && @svlc.autoplay
    
    @anim = animate(5) do |fr|
        now = @svlc.time/1000
        total = @svlc.length/1000
        
        @timeline.fraction = @svlc.position.round(2)
        @info.text = strong("#{File.basename(@svlc.path)}"), "  playing  #{now}s / #{total}s"
    end
end
