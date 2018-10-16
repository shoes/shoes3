module Cassowary
  require 'set'
  class Tableau
    attr_accessor :colums,:rows,:infeasible_rows, :external_rows, :external_parametric_vars
    
    def initialize
        # Map of variable to set of variables
        @columns = {}

        # Map of variable to LinearExpression
        @rows = {}

        # Set of Variables
        @infeasible_rows = Set.new()

        # Set of Variables
        @external_rows = Set.new()

        # Set of Variables.
        @external_parametric_vars = Set.new()
      end

    def note_removed_variable(var, subject)
      if subject
        @columns[var].delete(subject) 
      end
    end

    # TODO: is my version of Dict.setdefault correct?
    def note_added_variable(var, subject)
      if subject
        # python: self.columns.setdefault(var, set()).add(subject)
        @columns[var] ||  @columns[var] = Set.new(subject)
      end
    end
       
    def add_row(var, expr)
      @rows[var] = expr
      expr.terms.each  do |clv|
        @columns[clv] || @columns[clv] = Set.new(var)
        if clv.is_external
          @external_parametric_vars.add(clv)
        end
      end
      if var.is_external
        @external_rows.add(var)
      end
    end

		def remove_column(var)
			rows = @columns.delete(var)
			if rows
				rows.each do |clv|
					expr = @rows[clv]
					expr.remove_variable(var)
				end
			end
			if var.is_external
				@external_rows.delete(var)
				@external_parametric_vars.delete(var)
			end
		end

		def remove_row(var)
			expr = @rows.delete(var)
			expr.terms.each_key do |clv|
				varset = @columns[clv]
				if varset
					varset.remove(var)
				end
			end
			@infeasible_rows.delete(var)
			if var.is_external
				@external_rows.delete(var)
			end
			return expr
		end

		def substitute_out(oldvar, expr)
		  varset = @columns[oldvar]
		  varset.each_value do |v|		# TODO: correct?
				rows = @rows[v]
				row.substitute_out(oldvar, expr, v, self)
				if v.is_restricted && row.constant < 0.0
					@infeasible_rows.add(oldvar)
				end
				if oldvar.is_external
					@external_rows.add(oldvar)
					@external_parametrics_vars.delete(oldvar) # TODO correct?
				end
			end
			@columns.delete(oldvar)
		end
  end # tableau class
end # module Cassowary
