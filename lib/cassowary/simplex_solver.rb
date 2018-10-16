module Cassowary
	require_relative 'utils'
	require_relative 'tableau'
	require_relative 'variable'
	require_relative 'expression'
	require_relative 'constraint'
	
	class SolverEditContext
		attr_accessor :solver
		
		def initialize(solver)
			@solver = solver
		end
		
		def enter
			@solver.begin_edit
		end
		
		def exit(type, value, tb)
			@solver.end_edit
		end
	end # class SolverEditContext

	class SimplexSolver < Tableau 
		attr_accessor :stay_error_vars, :error_vars, :marker_vars, :objective,
			:edit_var_map, :slack_counter, :artificial_counter, :dummy_counter,
			:auto_solve, :need_solving, :optimize_count, :rows, :edit_variable_stack
			
    def initialize()
        super()
        @stay_error_vars = []
        @error_vars = {}
        @marker_vars = {}

        @objective = ObjectiveVariable.new('Z')
				@edit_var_map = {}

        @slack_counter = 0
        @artificial_counter = 0
        @dummy_counter = 0
        @auto_solve = true
        @needs_solving = false

        @optimize_count = 0

        @rows[self.objective] = Expression()
        @edit_variable_stack = [0]
		end

		def add_constraint(cn, strength=nil, weight=nil)
			if strength || weight
				cn = cn.clone
				if strength
					cn.strength = strength
				end
				if weight
					cn.weight
				end
			end
			expr, eplus, eminus, prev_edit_constant = self.new_expression(cn) # worry?
			if ! self.try_adding_directly(expr)
			  self.add_with_artificial_variable(expr)
			end
			@needs_solving = true
			if cn.is_edit_constraint
				i = @edit_var_map.size
				@edit_var_map[cn.variable] = EditInfo.new(cn, eplus, eminus, prev_edit_constant, i)
			end
			if @auto_solve
				self.optimize(@objective)
				self.set_external_variables()		
			end
		end

		def add_edit_var(v, strength=STRONG)
			rreturn self.add_constraint(EditConstraint.new(v, strength))
		end
		
		def remove_edit_var(v)
			self.remove_constraint(@edit_var_map[v].constraint)
		end
		
		def edit()
			return SolverEditContext.new(self)
		end
		
		def resolve()
			self.dual_optimize()
			self.set_external_variables()
			self.infeasible_rows.clear()
			self.reset_stay_constants()
		end
		
    #######################################################################
    # Internals
    #######################################################################
=begin
    def new_expression(self, cn):
        # print("* new_expression", cn)
        # print("cn.is_inequality == ", cn.is_inequality)
        # print("cn.is_required == ", cn.is_required)
        expr = Expression(constant=cn.expression.constant)
        eplus = None
        eminus = None
        prev_edit_constant = None
        for v, c in cn.expression.terms.items():
            e = self.rows.get(v)
            if not e:
                expr.add_variable(v, c)
            else:
                expr.add_expression(e, c)

        if cn.is_inequality:
            # print("Inequality, adding slack")
            self.slack_counter = self.slack_counter + 1
            slack_var = SlackVariable(prefix='s', number=self.slack_counter)
            expr.set_variable(slack_var, -1)

            self.marker_vars[cn] = slack_var
            if not cn.is_required:
                self.slack_counter = self.slack_counter + 1
                eminus = SlackVariable(prefix='em', number=self.slack_counter)
                expr.set_variable(eminus, 1)
                z_row = self.rows[self.objective]
                z_row.set_variable(eminus, cn.strength * cn.weight)
                self.insert_error_var(cn, eminus)
                self.note_added_variable(eminus, self.objective)
        else:
            if cn.is_required:
                # print("Equality, required")
                self.dummy_counter = self.dummy_counter + 1
                dummy_var = DummyVariable(number=self.dummy_counter)
                eplus = dummy_var
                eminus = dummy_var
                prev_edit_constant = cn.expression.constant
                expr.set_variable(dummy_var, 1)
                self.marker_vars[cn] = dummy_var
                # print("Adding dummy_var == d%s" % self.dummy_counter)
            else:
                # print("Equality, not required")
                self.slack_counter = self.slack_counter + 1
                eplus = SlackVariable(prefix='ep', number=self.slack_counter)
                eminus = SlackVariable(prefix='em', number=self.slack_counter)
                expr.set_variable(eplus, -1)
                expr.set_variable(eminus, 1)
                self.marker_vars[cn] = eplus

                z_row = self.rows[self.objective]
                # print("z_row", z_row)
                sw_coeff = cn.strength * cn.weight
                # if sw_coeff == 0:
                #     print("cn ==", cn)
                #     print("adding ", eplus, "and", eminus, "with sw_coeff", sw_coeff)
                z_row.set_variable(eplus, sw_coeff)
                self.note_added_variable(eplus, self.objective)
                z_row.set_variable(eminus, sw_coeff)
                self.note_added_variable(eminus, self.objective)

                self.insert_error_var(cn, eminus)
                self.insert_error_var(cn, eplus)

                if cn.is_stay_constraint:
                    self.stay_error_vars.append((eplus, eminus))
                elif cn.is_edit_constraint:
                    prev_edit_constant = cn.expression.constant

        # print('new_expression returning:', expr)
        if expr.constant < 0:
            expr.multiply(-1.0)
        return expr, eplus, eminus, prev_edit_constant
=end
		def begin_edit()
			#assert len(self.edit_var_map) > 0
			self.infeasible_rows.clear()
			self.reset_stay_constants()
			self.edit_variable_stack.append(@edit_var_map.size)
		end
		
		def end_edit()
			#assert len(self.edit_var_map) > 0
			self.resolve()
			self.edit_variable_stack.pop()
			self.remove_edit_vars_to(@edit_variable_stack[-1]) # worry?
		end
		
		def remove_all_edit_vars()
        self.remove_edit_vars_to(0)
    end
    

		def remove_edit_vars_to(n)
			begin
				
			rescue ConstraintNotFound => err
				raise InternalError('Constraint not found during internal removal')
			end
		end
		
		def add_stay(v, strength=WEAK, weight=1.0)
			return self.add_constraint(StayConstraint.new(v, strength, weight))
		end
		
=begin
    def remove_constraint(self, cn):
        # print("removeConstraint", cn)
        # print(self)
        self.needs_solving = True
        self.reset_stay_constants()
        z_row = self.rows[self.objective]

        e_vars = self.error_vars.get(cn)
        # print("e_vars ==", e_vars)
        if e_vars:
            for cv in e_vars:
                try:
                    z_row.add_expression(self.rows[cv], -cn.weight * cn.strength, self.objective, self)
                    # print('add expression', self.rows[cv])
                except KeyError:
                    z_row.add_variable(cv, -cn.weight * cn.strength, self.objective, self)
                    # print('add variable', cv)

        try:
            marker = self.marker_vars.pop(cn)
        except KeyError:
            raise ConstraintNotFound()

        # print("Looking to remove var", marker)
        if not self.rows.get(marker):
            col = self.columns[marker]
            # print("Must pivot -- columns are", col)
            exit_var = None
            min_ratio = 0.0
            for v in col:
                # print('check var', v)
                if v.is_restricted:
                    # print('var', v, ' is restricted')
                    expr = self.rows[v]
                    coeff = expr.coefficient_for(marker)
                    # print("Marker", marker, "'s coefficient in", expr, "is", coeff)
                    if coeff < 0:
                        r = -expr.constant / coeff
                        if exit_var is None or r < min_ratio: # EXTRA BITS IN JS?
                            # print('set exit var = ',v,r)
                            min_ratio = r
                            exit_var = v

            if exit_var is None:
                # print("exit_var is still None")
                for v in col:
                    # print('check var', v)
                    if v.is_restricted:
                        # print('var', v, ' is restricted')
                        expr = self.rows[v]
                        coeff = expr.coefficient_for(marker)
                        # print("Marker", marker, "'s coefficient in", expr, "is", coeff)
                        r = expr.constant / coeff
                        if exit_var is None or r < min_ratio:
                            # print('set exit var = ',v,r)
                            min_ratio = r
                            exit_var = v

            if exit_var is None:
                # print("exit_var is still None (again)")
                if len(col) == 0:
                    # print('remove column',marker)
                    self.remove_column(marker)
                else:
                    exit_var = [v for v in col if v != self.objective][-1] # ??
                    # print('set exit var', exit_var)

            if exit_var is not None:
                # print('Pivot', marker, exit_var,)
                self.pivot(marker, exit_var)

        if self.rows.get(marker):
            # print('remove row', marker)
            expr = self.remove_row(marker)

        if e_vars:
            # print('e_vars exist')
            for v in e_vars:
                if v != marker:
                    # print('remove column',v)
                    self.remove_column(v)

        if cn.is_stay_constraint:
            if e_vars:
                # for p_evar, m_evar in self.stay_error_vars:
                remaining = []
                while self.stay_error_vars:
                    p_evar, m_evar = self.stay_error_vars.pop()
                    found = False
                    try:
                        # print('stay constraint - remove plus evar', p_evar)
                        e_vars.remove(p_evar)
                        found = True
                    except KeyError:
                        pass
                    try:
                        # print('stay constraint - remove minus evar', m_evar)
                        e_vars.remove(m_evar)
                        found = True
                    except KeyError:
                        pass
                    if not found:
                        remaining.append((p_evar, m_evar))
                self.stay_error_vars = remaining

        elif cn.is_edit_constraint:
            assert e_vars is not None
            # print('edit constraint - remove column', self.edit_var_map[cn.variable].edit_minus)
            self.remove_column(self.edit_var_map[cn.variable].edit_minus)
            del self.edit_var_map[cn.variable]

        if e_vars:
            for e_var in e_vars:
                # print('Remove error var', e_var)
                del self.error_vars[e_var]

        if self.auto_solve:
            # print('final auto solve')
            self.optimize(self.objective)
            self.set_external_variables()
=end


		def resolve_array(new_edit_constants)
			@edit_var_map.each_pair do |v, cei|
				self.suggest_value(v, new_edit_constants[cei.index])  # TODO class/type of cei is?
			end
			self.resolve
		end
		
		def suggest_value(v, x)
			cei = @edit_var_map[v]
			if ! cei
				raise InternalError("suggestValue for variable %s, but var is not an edit variable", v) #TODO
			end
      delta = x - cei.prev_edit_constant
      cei.prev_edit_constant = x
      self.delta_edit_constant(delta, cei.edit_plus, cei.edit_minus)
		end
		
		def solve()
      if @needs_solving
        self.optimize(@objective)
        self.set_external_variables()
			end
		end

    def set_edited_value(v, n)
			#if v not in self.columns or v not in self.rows
			if ! @columns[v] || !@rows[v] 	# TODO correct?
					v.value = n
			end
			if ! approx_equal(n, v.value)
					self.add_edit_var(v)
					self.begin_edit()
					self.suggest_value(v, n)
					self.end_edit()
			end
		end
		
    def add_var(v)
      # if v not in self.columns or v not in self.rows:
			if ! @columns[v] || !@rows[v] 	# TODO correct?
        self.add_stay(v)
      end
    end
    
=begin

    def add_with_artificial_variable(self, expr):
        # print("add_with_artificial_variable", expr)
        self.artificial_counter = self.artificial_counter + 1
        av = SlackVariable(prefix='a', number=self.artificial_counter)
        az = ObjectiveVariable('az')
        az_row = expr.clone()
        # print('Before add_rows')
        # print(self)
        self.add_row(az, az_row)
        self.add_row(av, expr)
        # print('after add_rows')
        # print(self)
        self.optimize(az)
        az_tableau_row = self.rows[az]
        # print("azTableauRow.constant =", az_tableau_row.constant)
        if not approx_equal(az_tableau_row.constant, 0.0):
            # print("azTableauRow.constant is 0")
            self.remove_row(az)
            self.remove_column(av)
            raise RequiredFailure()

        e = self.rows.get(av)
        if e is not None:
            # print("av exists")
            if e.is_constant:
                # print("av is constant")
                self.remove_row(av)
                self.remove_row(az)
                return
            entry_var = e.any_pivotable_variable()
            self.pivot(entry_var, av)

        # print("av shouldn't exist now")
        assert av not in self.rows
        self.remove_column(av)
        self.remove_row(az)

=end
		def try_adding_directly(expr)
			subject = self.choose_subject(expr)
			if subject == nil
				# print("try_adding_directly returning: False")
				return false
			end
			expr.new_subject(subject)
			if @columns[subject]
				self.substitute_out(subject, expr)
			end
			self.add_row(subject, expr)
			# print("try_adding_directly returning: True")
			return True
		end
=begin
    def choose_subject(self, expr):
        # print('choose_subject', expr)
        subject = None
        found_unrestricted = False
        found_new_restricted = False

        retval_found = False
        retval = None
        for v, c in expr.terms.items(): # CHECK??
            if found_unrestricted:
                if not v.is_restricted:
                    if v not in self.columns:
                        retval_found = True
                        retval = v
                        break
            else:
                if v.is_restricted:
                    if not found_new_restricted and not v.is_dummy and c < 0:
                        col = self.columns.get(v)
                        if col == None or (len(col) == 1 and self.objective in self.columns):
                            subject = v
                            found_new_restricted = True
                else:
                    subject = v
                    found_unrestricted = True

        if retval_found:
            return retval

        if subject:
            return subject

        coeff = 0.0
        for v, c in expr.terms.items():
            if not v.is_dummy:
                retval_found = True
                retval = None
                break
            if not v in self.columns:
                subject = v
                coeff = c

        if retval_found:
            return retval

        if not approx_equal(expr.constant, 0.0):
            raise RequiredFailure()

        if coeff > 0:
            expr = expr * -1

        return subject
=end
    def delta_edit_constant(delta, plus_error_var, minus_error_var)
			expr_plus = @rows[plus_error_var]
			if expr_plus 
				expr_plus.constant = expr_plus.constant + delta
				if expr_plus.constant < 0.0
						@infeasible_rows.add(plus_error_var)
				end
				return
			end
			expr_minus = @rows.get[minus_error_var]
			if expr_minus 
				expr_minus.constant = expr_minus.constant - delta
				if expr_minus.constant < 0
						@infeasible_rows.add(minus_error_var)
				end
				return
			end
			begin
				@columns[minus_error_var].each do |basic_var|
					expr = @rows[basic_var]
					c = expr.coefficient_for(minus_error_var)
					expr.constant = expr.constant + (c * delta)
					if basic_var.is_restricted && expr.constant < 0
							@infeasible_rows.add(basic_var)
					end
				end
			rescue KeyError => e	# Wrong!
				pass
			end
    end

    def dual_optimize()
			z_row = @rows[@objective]
			@infeasible_rows.each do |t|	# a Set
				exit_var = @infeasible_rows.delete(t)
				entry_var = None
				expr = @rows[exit_var]
				if expr
					if expr.constant < 0
						ratio = float('inf')  # TODO 
						expr.terms.each_pair do |v, cd|
							if cd > 0 && v.is_pivotable
								zc = z_row.coefficient_for(v)
								r = zc / cd
								if r < ratio # JS difference?
									entry_var = v
									ratio = r
								end
							end
						end
						if ratio == float('inf') # TODO
							raise InternalError("ratio == nil (MAX_VALUE) in dual_optimize")
						end
						self.pivot(entry_var, exit_var)
					end
				end
			end
		end

=begin
    def optimize(self, z_var):
        # print("optimize", z_var)
        # print(self)
        self.optimize_count = self.optimize_count + 1

        z_row = self.rows[z_var]
        entry_var = None
        exit_var = None

        # print(self.objective)
        # print(z_var)
        # print(self.rows[self.objective])
        # print(self.rows[z_var])

        while True:
            objective_coeff = 0.0

            # Not convinced the sort is correct here; but test suite
            # doesn't pass reliably without it.
            for v, c in sorted(z_row.terms.items(), key=lambda x: x[0].name):
                # print('term check', v, v.is_pivotable, c)
                if v.is_pivotable and c < objective_coeff:
                    # print('candidate found')
                    objective_coeff = c
                    entry_var = v
                    break;

            if objective_coeff >= -EPSILON or entry_var is None:
                return

            # print('entry_var:', entry_var)
            # print("objective_coeff:", objective_coeff)

            min_ratio = float('inf')
            r = 0

            for v in self.columns[entry_var]:
                # print("checking", v)
                if v.is_pivotable:
                    expr = self.rows[v]
                    coeff = expr.coefficient_for(entry_var)
                    # print('pivotable, coeff =', coeff)
                    if coeff < 0:
                        r = -expr.constant / coeff
                        if r < min_ratio:
                            min_ratio = r
                            exit_var = v

            if min_ratio == float('inf'):
                raise RequiredFailure('Objective function is unbounded')

            self.pivot(entry_var, exit_var)

            # print(self)
=end
    def pivot(entry_var, exit_var)
			# print('pivot:',entry_var, exit_var)
			if entry_var == nil
					puts("WARN - entry_var is None")
			end
			if exit_var == nil
					puts("WARN - exit_var is None")
			end
			p_expr = self.remove_row(exit_var)
			p_expr.change_subject(exit_var, entry_var)
			self.substitute_out(entry_var, p_expr)
			self.add_row(entry_var, p_expr)
    end
	
		def reset_stay_constants()
			@stay_error_vars.each_pair do |p_var, m_var| 
				expr = @rows[p_var]
				if !expr 
					expr = @rows[m_var]
				end
				if expr
					expr.constant = 0.0
				end
			end
		end
	
		def set_external_variables()
			@external_parametric_vars.each do |v|
				if @rows[v]
					continue
				end
				v.value = 0.0
			end
			@external_rows.each do |v|
				expr = @rows[v]
				v.value = expr.constant
			end
			@needs_solving = false
		end
			
		def insert_error_var(cn, var)
			constraint_set = @error_vars[var]
			if !constraint_set 
				constraint_set = Set.new()
				@error_vars[cn] = constraint_set
			end
			constraint_set.add(var)
			if !@error_vars[var] 
			  @error_vars[var] = Set.new(var) # TODO: correct?
			end
		end

	end # class SimplexSolver
end #module Cassowary
