Shoes.setup do
	gem 'cassowary-ruby'
end
require 'cassowary'
include Cassowary
class CassowaryLayout 
  attr_accessor :canvas, :widgets, :solver, :attrs, :left_limit,
	:right_limit, :top_limit, :height_limit, :rl_stay, :hl_stay,
  :ready, :canvas_w, :canvas_h
  
  def initialize()
    #$stderr.puts "initialized"
    @widgets = {}
    @solver = SimplexSolver.new
    # @attrs[name][attr] = Variable for name.attr
    @attrs = {} 
    @ready = false
  end
  
  
  # canvas may not have a width or height. be careful
  def setup(canvas, attr)
    @canvas = canvas
    hgt = 0
    wid = 0
    if (attr && attr[:width]) 
      wid = attr[:width]
    end
    if (attr && attr[:height])
      hgt = attr[:height]
    end
    vars = {}
    vars['start'] = Variable.new(name: 'super.start', value: 0.0)
    vars['width'] = Variable.new(name: 'super.width', value: wid)
    vars['top'] = Variable.new(name: "super.top", value: 0.0)
    vars['height'] = Variable.new(name: "super.height", value: hgt)
    vars['end'] = Variable.new(name: "super.end", value: wid)
    vars['bottom'] = Variable.new(name: "super.bottom", value: hgt)
    @attrs['super'] = vars
		@left_limit = vars['start'] 
		@right_limit = vars['end']
		@top_limit = vars['top']
		@height_limit = vars['bottom']
		@rl_stay = @solver.add_stay(@right_limit)
    @hl_stay = @solver.add_stay(@height_limit)
    $stderr.puts "callback: setup #{wid} X #{hgt}"
  end
  
  def contents 
    @widgets
  end 
  
  def var(n,a)
    @attrs[n][a]
  end
  
  def solver
    @solver
  end
  
  # callback from Shoes to notify addition of widget to layout.
  # widget is not on-screen and allocated at this time. Pity
  # DO NOT enumerate attrs hash - a crash if you try.
  def add(canvas, widget, attrs)
    name = attrs && attrs[:name]
    $stderr.puts "callback add: #{name} #{widget.class} #{canvas.contents.size}"
    if name
      @widgets[name] = widget
      # create Cassowary Variables for Shoes element
      vars = {}
      vars['start'] = Variable.new(name: "#{name}.start", value: 0)
      vars['end'] = Variable.new(name: "#{name}.end", value: 0)
      vars['width'] = Variable.new(name: "#{name}.width", value: 0)
      vars['top'] = Variable.new(name: "#{name}.top", value: 0)
      vars['bottom'] = Variable.new(name: "#{name}.bottom", value: 0)
      vars['height'] = Variable.new(name: "#{name}.height", value: 0)
      @attrs[name] = vars
    end
  end
  
  def remove(canvas, widget, pos)
    $stderr.puts"callback: remove"
    return true
  end
  
  # add a constraint (of the hash variety from an emeus vlf_parse)
  # TODO: multiplier is this correct? Seems OK, but...
  # TODO: is constant correct?        Seems OK, but...
  # TODO: convert start to left, end to right? affects add() method
  def add_cs(h)
    $stderr.puts h.inspect
    ## convert hash to Cassowary::Constraint and add to solver
    tgt = h[:view1]
    tgta = h[:attr1]
    rel = h[:relation]
    src = h[:view2]
    srca = h[:attr2]
    cons = h[:constant]
    mult = h[:multiplier]
    strength = h[:strength]
    $stderr.puts "#{tgt}.#{tgta} #{rel} #{src}.#{srca} #{cons} #{mult} #{strength}"
    t = @attrs[tgt][tgta]
    if src
      s = @attrs[src][srca] 
    else 
      s = 0
    end
    stg = nil
    case strength
      when "required"
        stg = Strength::RequiredStrength
      when "strong"
        stg = Strength::StrongStrength
      when "medium"
        stg = Strength::MediumStrength
      when "weak"
        stg = Strength::WeakStrength
    end
    cn = nil
    case rel 
      when "EQ" 
        cn = (t * mult).cn_equal (s + cons), stg
      when "GE" 
        cn = (t * mult).cn_geq (s + cons), stg
      when "LE"
        cn = (t * mult).cn_leq (s + cons), stg
    end
    $stderr.puts "#{cn.inspect}"
    @solver.add_constraint(cn)
 end
  
  
  def size(canvas, pass)
    if pass == 0
      return
    else
      $stderr.puts "callback: size Change!  w: #{canvas.width} h:#{canvas.height}"
      # reset stays for new window size
      @solver.remove_constraint(@rl_stay) if @ready
      @right_limit.value = canvas.width
      @rl_stay = @solver.add_stay(@right_limit, Strength::RequiredStrength)
      @solver.remove_constraint(@hl_stay) if @ready
      @height_limit.value = canvas.height
      @solver.add_stay(@height_limit, Strength::RequiredStrength)
      #@solver.solve
      self.move_widgets if @ready
    end
  end
  
  
  def move_widgets
		@widgets.each_pair do |k, widget|
			l = widget.left
			t = widget.top
			h = widget.height
			w = widget.width
			$stderr.puts "#{k} #{widget.inspect} from #{l},#{t},#{w},#{h}"
      puts @attrs.inspect
      
			left = @attrs[k]['start'].value.to_i
      top = @attrs[k]['top'].value.to_i
      e = @attrs[k]['end'].value.to_i
			width = @attrs[k]['width'].value.to_i
      height =  @attrs[k]['height'].value.to_i
      b = @attrs[k]['bottom'].value.to_i
			$stderr.puts "move to #{left}, #{top} for w:#{width}|#{e-left} h:#{height}|#{b-top}"
			widget.style width: width, height: height
			widget.move(left, top)

    end
  end
 
  def resize(w, h)
    puts "resize called"
  end
  
  def clear()
    $stderr.puts "callback: clear"
  end
  
  def rules(arg)
    $stderr.puts "callback rules #{arg}"
  end
 
  # does the heavy work. Constraint arguments can be
  # the hash type from vfl_parse or painfully handcrafted
  def finish(constraints)
		$stderr.puts "callback finish"
    # move initial widget settings to Solver Variables.  
    @widgets.each_pair do |nm, ele|
			left = ele.left
			top = ele.top
			height = ele.height
			width = ele.width
			$stderr.puts "\n#{nm} #{ele.inspect} from #{left},#{top},#{width},#{height}"
      # update Solver Vars with real settings
      vars = @attrs[nm]
      vars['start'].value = ele.left
      vars['top'].value = ele.top
      vars['width'].value = ele.width
      vars['height'].value = ele.height
      vars['bottom'].value = ele.top + ele.height
      vars['end'].value = ele.left + ele.width
      
      $stderr.puts "Setting #{nm}: #{vars.inspect}"
      # need all 3 to get horizontal mostly correct
      @solver.add_constraint(vars['start'].cn_geq @attrs['super']['start'], Strength::RequiredStrength)
      @solver.add_constraint(vars['width'].cn_geq width)
      @solver.add_constraint(vars['width'].cn_equal vars['end'] - vars['start'] ,  Strength::RequiredStrength)
      # need all 3 to get vertical mostly correct
      @solver.add_constraint(vars['top'].cn_geq @attrs['super']['top'], Strength::RequiredStrength)
      @solver.add_constraint(vars['height'].cn_geq height)
      @solver.add_constraint(vars['height'].cn_equal vars['bottom'] - vars['top'] ,  Strength::RequiredStrength)
    end
    puts "\nAdd constraints #{@attrs.inspect}"
    # add the constraints
    if constraints
      constraints.each do |c|
        if c.class == Cassowary::LinearEquation
          # handcrafted cassowary-ruby style
          @solver.add_constraint(c)
        else
          self.add_cs(c)
        end
      end
    end
    @ready = true
    $stderr.puts "\nSolving..."
    @solver.solve
    self.move_widgets
  end
   
end
