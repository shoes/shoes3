###########################################################################
# Expressions
#
# Expressions are combinations of variables with multipliers and constants
###########################################################################
module Cassowary

  class Expression
    attr_accessor :constant, :terms
    
		def initialize(variable=nil, value=1.0, constant=0.0)
			@constant = constant
			@terms = {}
			if variable
			  self.set_variable(variable,value)
			end
		end
		
		def set_variable(v, c)
      @terms[v] = c.to_f
		end
		
		def remove_variable(v)
      @terms.delete(v)
		end
		
		def is_constant
			return ! @terms
		end
		
		# Ruby Object.clone _should_ work for Expression.clone()
		


    ######################################################################
    # Mathematical operators
    ######################################################################
		def *(x)
		  result = 0.0
			if x.kind_of? Expression
			  if self.is_constant
			    result = x * @constant
			  elsif x.is_constant
					result = self. * x.constant  # ?
				else
					return NotImplemented
				end
			elsif x.kind_of? Variable
			  if self.is_constant
					result = Expression.new(x, @constant)
				else
					return NotImplemented
				end
			elsif x.kind_of? Numeric
			  result = Expression.new(constant=@constant * x)
			  @terms.each_pair do |clv, value| 
					result.set_variable(clv, value * x)
				end
			else
				return NotImplemented
			end
			return result
		end
			
		def /(x)
		  result = 0.0
			if x.kind_of? Numeric
				if approx_equal(x, 0)
					raise ZeroDivisionError
				end
				result = Expression.new(constant=@constant / x)
				terms.each_pair do |clv, value|
					result.set_variable(clv, value / x)
				end
			else
				if x.is_constant
					result = self / x.constant	# don't like 'self' here
				else
					return NotImplemented
				end
			end
			return result	
		end
		
		def +(x)
			if x.kind_of? Expression
				result = self.clone
				result.add_expression(x, 1.0)
				return result
			elsif x.kind_of? Variable
				result = self.clone
				result.add_variable(x, 1.0)
				return result
			elsif x.kind_of? Numeric
				result = self.clone
				result.add_expression(Expression.new(constant=x), 1.0)
				return result
			else
				return NotImplemented
			end
		end

		def -(x)
			case x.kind_of? 
			when Expression
				result = self.clone
				result.add_expression(x, -1.0)
				return result
			when Variable
				result = self.clone
				result.add_variable(x, -1.0)
				return result
			when Numeric
				result = self.clone
				result.add_expression(Expression.new(constant = x), -1.0)
				return result
			else
				return NotImplemented
			end
		end

    ######################################################################
    # Mathematical operators
    ######################################################################
		def ==(other)	# TODO: can't use = in Ruby
			case other.kind_of? 
			when Expression,Variable,Numeric
				return Constraint(self, Constraint.EQ, other)
			else
				return NotImplemented
			end
		end
		
		def <(other)
      # < and <= are equivalent in the API; it's effectively true
      # due to float arithmetic, and it makes the API a little less hostile,
      # because all the comparison operators exist.
			case other.kind_of? 
			when Expression,Variable,Numeric
				return Constraint(self, Constraint.LEQ, other)
			else
				return NotImplemented
			end
		end

		def <=(other)
			case other.kind_of? 
			when Expression,Variable,Numeric
				return Constraint(self, Constraint.LEQ, other)
			else
				return NotImplemented
			end
		end
		
		def >(other)
      # < and <= are equivalent in the API; it's effectively true
      # due to float arithmetic, and it makes the API a little less hostile,
      # because all the comparison operators exist.
			case other.kind_of? 
			when Expression,Variable,Numeric
				return Constraint(self, Constraint.GEQ, other)
			else
				return NotImplemented
			end
		end

		def >=(other)
			case other.kind_of? 
			when Expression,Variable,Numeric
				return Constraint(self, Constraint.GEQ, other)
			else
				return NotImplemented
			end
		end

    ######################################################################
    # Internal mechanisms
    ######################################################################
		def add_expression(expr, n=1.0, subject=nil, solver=nil)
			if expr.kind_of? AbstractVariable
				expr = Expression.new(variable=expr)
			end
			@constant = @constant + n * expr.constant
			@terms.each_pair do |clv, coeff|
				self.add_variable(clv, coeff * n, subject, solver)
			end
		end
		
		def add_variable(v, cd=1.0, subject=nil, solver=nil)
		  coeff = @terms[v]
		  if coeff
				new_coefficient = coeff + cd
				if approx_equal(new_coefficient, 0.0)
					if solver
						solver.note_removed_variable(v, subject)
					end
					self.remove_variable(v)
				else
					self.set_variable(v, new_coefficient)
				end
			else
				if ! approx_equal(cd, 0.0)
					self.set_variable(v, cd)
					if solver
						solver.note_added_variable(v, subject)
					end
				end
			end
		end

    def any_pivotable_variable
      if self.is_constant
            raise InternalError('any_pivotable_variable called on a constant')
      end
      retval = nil
      @terms.each_pair do |clv, c|
				if clv.is_pivotable
					retval = clv
					break
				end
			end
			return retval
	  end
	  
	  def substitute_out(outvar, expr, subject=nil, solver=nil)
			multiplier = @terms.delete(outvar)
			@constant = @constant + multiplier * expr.constant
			expr.terms.each_pair do |clv, coeff|
				old_coefficient = @terms[clv]
				if old_coefficient
					new_coefficient = old_coefficient + multiplier * coeff
					if approx_equal(new_coefficient, 0)
						solver.note_removed_variable(clv, subject)
						@terms.delete(cv)
					else
						self.set_variable(clv,new_coefficient)
					end
				else
					self.set_variable(clv, multiplier * coeff)
					if solver
						solver.note_added_variable(clv, subject)
					end
				end
			end
	  end
			
    def change_subject(old_subject, new_subject)
      self.set_variable(old_subject, self.new_subject(new_subject))
    end

    def multiply(x)
      @constant = @constant * x.to_f
      @terms.each_pair do |clv, value|
				self.set_variable(clv, value * x)
      end
    end

		def new_subject(subject)
		  value = @terms.delete(subject)
		  reciprocal = 1.0 / value
      self.multiply(-reciprocal)
      return reciprocal
		end
		
    def coefficient_for(clv)
        return terms[clv] || 0.0
    end
	end # class Expression

end # Cassowary module
