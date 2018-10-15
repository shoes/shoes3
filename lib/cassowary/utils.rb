
module Cassowary

  # from utils.py
  EPSILON = 1e-8
  
  REQUIRED = 1001001000
  STRONG = 1000000
  MEDIUM = 1000
  WEAK = 1
  
  def approx_equal(a, b, epsilon=EPSILON)
    return (a - b).abs < epsilon
  end
  
  # from error.py
  class CassowaryException < StandardError; end
  class InternalError < CassowaryException; end
  class ConstraintNotFound < CassowaryException; end
  class RequiredFailure < CassowaryException; end
  class NotImplemented < CassowaryException; end
  class ZeroDivisionError < CassowaryException; end
  
  #from edit_info.py
  class EditInfo 
    attr_accessor :constraint, :edit_plus, :edit_minus, :prev_edit_constant, :index
    
    def initialize(constraint, edit_plus, edit_minus, prev_edit_constant, index)
      @constraint = constraint
      @edit_plus = edit_plus
      @edit_minus = edit_minus
      @prev_edit_constant = prev_edit_constant
      @index = index
    end
  end


end
