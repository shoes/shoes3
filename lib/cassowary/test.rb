# really just tests if syntax is acceptable for loading.
# using it requires the other test pass. 
require_relative 'utils'
require_relative 'tableau'
require_relative 'variable'
require_relative 'expression'
require_relative 'constraint'
include Cassowary

# From utils.rb:
puts approx_equal 10, 10.00000001     # false
puts approx_equal 10, 10.0000000001   # true
puts approx_equal 10.1, 10.18, 0.01   # false
# From error.py
begin 
  raise RequiredFailure
rescue => e
  puts e
end
# From edit_info.py
puts EditInfo.new(1,2,3,4,5).inspect
# From expression.py
v = AbstractVariable.new('foo')
puts "v is #{v.inspect}"
puts "mult #{v * 12.34}"
# From tableau.py
tb = Tableau.new
puts tb.inspect

require_relative 'simplex_solver'

