# Not shoes, just ruby
# From https://cassowary.readthedocs.io/en/latest/topics/examples.html#gui-layout
# converted from python to ruby
#
# gem install cassowary-ruby
require 'cassowary'
include Cassowary

class Button
  attr_accessor :left, :width
  def initialize(identifier)
    @left = Variable.new(name: 'left' + identifier, value: 0)
    @width = Variable.new(name: 'width' + identifier, value: 0)
  end
end
solver = SimplexSolver.new()
b1 = Button.new('b1')
b2 = Button.new('b2')

# window horizontal bounds
left_limit = Variable.new(name: 'left')
right_limit = Variable.new(name: 'width')
left_limit.value = 0
right_limit.value = 0     # CJC: ruby needs a value. 

# left_limit stay is fixed (Required), right can be resized
solver.add_stay(left_limit)
solver.add_stay(right_limit, Strength::WeakStrength)

# The two buttons are the same width
solver.add_constraint(b1.width.cn_equal b2.width)

# Button1 starts 50 from the left margin.
solver.add_constraint(b1.left.cn_equal left_limit + 50)

# Button2 ends 50 from the right margin (???)
solver.add_constraint((left_limit + right_limit).cn_equal b2.left + b2.width + 50)

# Button2 starts at least 100 from the end of Button1. This is the
# "elastic" constraint in the system that will absorb extra space
# in the layout.
solver.add_constraint(b2.left.cn_equal b1.left + b1.width + 100)

# Button1 has a minimum width of 87
solver.add_constraint(b1.width.cn_geq 87)

# Button1's preferred width is 87
solver.add_constraint(b1.width.cn_equal 87, Strength::StrongStrength)

# Button2's minimum width is 113
solver.add_constraint(b2.width.cn_geq 113)

# Button2's preferred width is 113
solver.add_constraint(b2.width.cn_equal 113, Strength::StrongStrength)

puts "b1: #{b1.inspect}"
puts "b2: #{b2.inspect}"
puts "rl.value #{right_limit.value}"

puts "RESIZING Window to 500"
right_limit.value = 500
# example fails - args are wrong for a constraint
#right_limit_stay = solver.add_constraint(right_limit, Strength::RequiredStrength)
right_limit_stay = solver.add_stay(right_limit, Strength::RequiredStrength)
puts "b1: #{b1.inspect}"
puts "b2: #{b2.inspect}"
puts "rl.value #{right_limit.value}"


