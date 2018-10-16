module Cassowary
###########################################################################
# Constraint
#
# Constraints are the restrictions on linear programming; an equality or
# inequality between two expressions.
###########################################################################
	class AbstractConstraint
		attr_accessor :strength, :weight, :s_edit_constraint, :is_inequality,
			:is_stay_contraint
			
		def initialize(strength, weight=1.0)
			@strength = strength
			@weight = weight
			@is_edit_constraint = false
			@is_inequality = false
			@is_stay_contraint = false
		end
		
		def is_required
		  @strength == REQUIRED
		end
	end
	
	class EditConstraint < AbstractConstraint
		attr_accessor :variable, :expression
		
		def initialize(variable, strength=STRONG, weight=1.0)
			super(strength, weight)
			@variable = variable
			@expression = Expression.new(variable, -1.0, variable.value)
			@is_edit_constraint = true
		end
	end
	
	class StayConstraint < AbstractConstraint
		attr_accessor :variable, :expression
		
		def initialize(variable, strength=STRONG, weight=1.0)
			super(strength, weight)
			@variable = variable
			@expression = Expression.new(variable, -1.0, variable.value)
			@is_stay_constraint = true
		end
	end
	

	class Constraint < AbstractConstraint
    LEQ = -1
    EQ = 0
    GEQ = 1

    def initialize(param1, operator=EQ, param2=None, strength=REQUIRED, weight=1.0)
        # Define a new linear constraint.
				#
        # param1 may be an expression or variable
        # param2 may be an expression, variable, or constant, or may be ommitted entirely.
        # If param2 is specified, the operator must be either LEQ, EQ, or GEQ

        if param1.kind_of? Expression
            if param2 == nil
                super(strength=strength, weight=weight)
                @expression = param1
            elsif param2.kind_of? Expression
                super(strength=strength, weight=weight)
                @expression = param1.clone()
                if operator == self.LEQ
                    @expression.multiply(-1.0)
                    @expression.add_expression(param2, 1.0)
                elsif operator == self.EQ
                    @expression.add_expression(param2, -1.0)
                elsif operator == self.GEQ
                    @expression.add_expression(param2, -1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
                end
            elsif param2.kind_of? Variable
                super(strength=strength, weight=weight)
                @expression = param1.clone()
                if operator == self.LEQ
                    @expression.multiply(-1.0)
                    @expression.add_variable(param2, 1.0)
                elsif operator == self.EQ
                   @expression.add_variable(param2, -1.0)
                elsif operator == self.GEQ
                   @expression.add_variable(param2, -1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
								end
            elsif param2.kind_of? Numeric
                super(strength=strength, weight=weight)
                @expression = param1.clone()
                if operator == self.LEQ
                    @expression.multiply(-1.0)
                   @expression.add_expression(Expression(constant=param2), 1.0)
                elsif operator == self.EQ
                    @expression.add_expression(Expression(constant=param2), -1.0)
                elsif operator == self.GEQ
                    @expression.add_expression(Expression(constant=param2), -1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
                end
            else
                raise InternalError("Invalid parameters to Constraint constructor")
						end
        elsif param1.kind_of? Variable
            if param2 == nil
                super(strength=strength, weight=weight)
                @expression = Expression.new(param1)
            elsif param2.kind_of? Expression
                super(strength=strength, weight=weight)
								@expression = param2.clone()
                if operator == self.LEQ
                    @expression.add_variable(param1, -1.0)
                elsif operator == self.EQ
                    @expression.add_variable(param1, -1.0)
                elsif operator == self.GEQ
                    @expression.multiply(-1.0)
										@expression.add_variable(param1, 1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
								end
            elsif param2.kind_of? Variable
                super(strength=strength, weight=weight)
                @expression = Expression.new(param2)
                if operator == self.LEQ
                    @expression.add_variable(param1, -1.0)
                elsif operator == self.EQ
                    @expression.add_variable(param1, -1.0)
                elsif operator == self.GEQ
                    @expression.multiply(-1.0)
                    @expression.add_variable(param1, 1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
								end
            elsif param2.kind_of? Numeric 
                super(strength=strength, weight=weight)
                @expression = Expression.new(constant=param2)
                if operator == self.LEQ
                    @expression.add_variable(param1, -1.0)
                elsif operator == self.EQ
                    @expression.add_variable(param1, -1.0)
                elsif operator == self.GEQ
                    @expression.multiply(-1.0)
                    @expression.add_variable(param1, 1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
                end
            else
                raise InternalError("Invalid parameters to Constraint constructor")
						end
        elsif param1.kind_of? Numeric
            if param2 == nil
                super(strength=strength, weight=weight)
                @expression = Expression(constant=param1)
            elsif param2.kind_of? Expression
                super(strength=strength, weight=weight)
                @expression = param2.clone()
                if operator == self.LEQ
                    @expression.add_expression(Expression.new(constant=param1), -1.0)
                elsif operator == self.EQ
                    @expression.add_expression(Expression.new(constant=param1), -1.0)
                elsif operator == self.GEQ
                    @expression.multiply(-1.0)
                    @expression.add_expression(Expression.new(constant=param1), 1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
								end
            elsif param2.kind_of? Variable
                super(strength=strength, weight=weight)
                @expression = Expression,new(constant=param1)
                if operator == self.LEQ
                    @expression.add_variable(param2, -1.0)
                elsif operator == self.EQ
                    @expression.add_variable(param2, -1.0)
                elsif operator == self.GEQ
                    @expression.multiply(-1.0)
                    @expression.add_variable(param2, 1.0)
                else
                    raise InternalError("Invalid operator in Constraint constructor")
								end
            elsif param2.kind_of? Numeric 
                raise InternalError("Cannot create an inequality between constants")
            else
                raise InternalError("Invalid parameters to Constraint constructor")
            end
        else
            raise InternalError("Invalid parameters to Constraint constructor")
				end
        @is_inequality = operator != self.EQ
		end
		
    def clone()
        c = Constraint(@expression, strength=@strength, weight=@weight)
        c.is_inequality = @is_inequality
        return c
     end
	end # class Constraint
end # module Cassowary
