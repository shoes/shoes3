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
=begin
    def __repr__(self):
        parts = []
        parts.append('Tableau info:')
        parts.append('Rows: %s (= %s constraints)' % (len(self.rows), len(self.rows) - 1))
        parts.append('Columns: %s' % len(self.columns))
        parts.append('Infeasible rows: %s' % len(self.infeasible_rows))
        parts.append('External basic variables: %s' % len(self.external_rows))
        parts.append('External parametric variables: %s' % len(self.external_parametric_vars))
        return '\n'.join(parts)
=end
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

=begin
    def add_row(self, var, expr):
        # print('add_row', var, expr)
        for clv in expr.terms:
            self.columns.setdefault(clv, set()).add(var)
            if clv.is_external:
                self.external_parametric_vars.add(clv)

        if var.is_external:
            self.external_rows.add(var)

        # print(self)
=end
       
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
=begin
    def remove_column(self, var):
        rows = self.columns.pop(var, None)

        if rows:
            for clv in rows:
                expr = self.rows[clv]
                expr.remove_variable(var)

        if var.is_external:
            try:
                self.external_rows.remove(var)
            except KeyError:
                pass

            try:
                self.external_parametric_vars.remove(var)
            except KeyError:
                pass
=end
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
=begin
    def remove_row(self, var):
        # print("remove_row", var)
        expr = self.rows.pop(var)

        for clv in expr.terms.keys():
            varset = self.columns[clv]
            if varset:
                # print("removing from varset", var)
                varset.remove(var)

        try:
            self.infeasible_rows.remove(var)
        except KeyError:
            pass
        if var.is_external:
            try:
                self.external_rows.remove(var)
            except KeyError:
                pass
        # print("remove_row returning", expr)
        return expr
=end
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
=begin
    def substitute_out(self, oldVar, expr):
        varset = self.columns[oldVar]
        for v in varset:
            row = self.rows[v]
            row.substitute_out(oldVar, expr, v, self)
            if v.is_restricted and row.constant < 0.0:
                self.infeasible_rows.add(v)

        if oldVar.is_external:
            self.external_rows.add(oldVar)
            try:
                self.external_parametric_vars.remove(oldVar)
            except KeyError:
                pass

        del self.columns[oldVar]
=end
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
