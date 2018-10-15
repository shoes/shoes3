###########################################################################
# Variables
#
# Variables are the atomic unit of linear programming, describing the
# quantities that are to be solved and constrained.
###########################################################################
module Cassowary
		  
	class AbstractVariable
		attr_accessor :name, :is_dummy, :is_external, :is_restricted
	  def initialize(name)
			@name = name
			@is_dummy = false
			@is_external = false
			@is_pivotable = false
			@is_restricted = false
	  end
	    
	  ## TODO verify 
	  # Var * x
	  def *(x)
			if x.kind_of? Numeric
			  return Expression.new(self, x) 
			end
			if x.kind_of? Expression
			  if x.is_constant
					return Expression.new(self, x.constant)
				else
				  raise NotImplmented
				end
			end
	  end
	  
	  # Var / x
	  def /(x)
	    if x.kind_of? Numeric
				if approx_equal(x, 0)
	         raise ZeroDivisionError
	      end
	      return Expression.new(value = 1.0 / x.constant)
	    elsif x.kind_of? Expression
				if x.is_constant
				  return  Expression(value = 1.0 /x.contant)
				else
				  return NotImplemented
				end
			else
				return NotImplemented
			end
	  end
	  
	  # Var + x
	  def +(x)
			if x.kind_of? Numeric
				return	Expression.new(constant = x)
			elsif x.kind_of? Expression
			  return Expression.new(self) + x
			elsif x.kind_of? AbstractVariable
			  return Expression.new(self) + Expression.new(x)
			else
				return NotImplemented
			end
	  end
	  
	  # Var - x
	  def -(x)
	    if x.kind_of? Numeric
	      return Expression.new(constant=-x)
	    elsif x.kind_of? Expression
				return Expression.new(self) - x
			elsif x.kind_of? AbstractVariable
				return Expression.new(self) - Expression.new(x)
			else
			  return NotImplemented
			end
	  end
	  
	end # class AbstractVariable
	
	class Variable < AbstractVariable
	  attr_accessor :value, :is_external
	  
    def initialize(name, value=0.0)
        super(name)
        @value = value.to_f
        @is_external = True
		end
    # __hash__ = object.__hash__
    
    # Var == x  # eq,  NOT assignment! NOT standard? 
    def ==(other)
			case other.kind_of? 
			  when Expression, Variable, Numeric
			    return Constrait.new(self, Constraint.EQ, other)
			  else
					return NotImplemented
			end
		end
		
		# Var < x
		def <(other)
			# < and <= are equivalent in the API; it's effectively true
      # due to float arithmetic, and it makes the API a little less hostile,
      # because all the comparison operators exist.
			return self <= other
		end
		
		# Var <= x
    def <=(other)
      case other.kind_of? 
				when Expression, Variable, Numeric
					return Constraint.new(self, Constraint.LEQ, other)
				else
					return NotImplemented
			end
		end
		
		# Var > x
		def >(other)
			# > and >= are equivalent in the API; it's effectively true
			# due to float arithmetic, and it makes the API a little less hostile,
			# because all the comparison operators exist.
		  return self >= other
		end
		
		def >= (other)
			case other.kind_of? 
				when Expression, Variable, Numeric
					return Constraint.new(self, Constraint.GEQ, other)
				else
				  return NotImplemented
			end
		end
		
	end # class Variable


	class DummyVariable < AbstractVariable
	  attr_accessor :is_dummy, :is_restricted
	  
    def initialize(number)
			super(name = sprintf("d%s",number))
      @is_dummy = true
      @is_restricted = true
		end
	end # class DummyVariable

	# what does this do other than a sugar layer 
	class ObjectiveVariable < AbstractVariable
    def initialize(name)
			super(name)
    end
	end # class ObjectiveVariable

	class SlackVariable < AbstractVariable
	  attr_accessor :is_pivotable, :is_restricted
	  def initialize(prefix, number)
		super(name=sprintf("%s%s", prefix, number))
		@is_pivotable = true
		@is_restricted = true
	  end
	end # class SlackVariable
end # module
