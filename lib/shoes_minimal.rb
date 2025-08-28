# Minimal Shoes module for Ruby 3.x compatibility testing
# This replaces the full shoes.rb temporarily to avoid crashes during initialization

# The App and Dialog classes are defined by the C code in shoes_ruby_init
# We don't define them here to avoid conflicts

class Shoes
  # Define the args! method that shoes_start_begin expects
  # Accept optional argument for macOS compatibility
  def self.args!(launch_flag = nil)
    # Return the path to the script to load
    # In a real implementation, this would parse command line arguments
    ARGV.first || nil
  end
  
  # Define clean method that shoes_final expects
  def self.clean
    # Cleanup method called during shutdown
  end
  
  # Define visit method for loading scripts
  def self.visit(path)
    $stderr.puts "Shoes.visit called with: #{path}"
    # Load and execute the script
    if path && File.exist?(path)
      $stderr.puts "Loading script: #{path}"
      begin
        # Actually execute the script in the main context
        load path
        $stderr.puts "Script executed successfully"
      rescue => e
        $stderr.puts "Error executing script: #{e.message}"
        $stderr.puts e.backtrace.join("\n")
      end
    end
    # Return nil to prevent further processing
    nil
  end
  
  # Define show_log for error handling
  def self.show_log
    puts "Shoes.show_log called"
  end
  
  # The app method is defined in C as shoes_app_main
  # Let's check what methods are available
  $stderr.puts "[Ruby] Shoes singleton methods: #{singleton_methods.inspect}"
  
  # Check if Shoes module has the App class defined
  if const_defined?(:App)
    $stderr.puts "[Ruby] App class is defined"
  else
    $stderr.puts "[Ruby] App class is NOT defined"
  end
  
  # Don't override the C implementation if it exists
  unless singleton_methods.include?(:app)
    $stderr.puts "[Ruby] Defining fallback Shoes.app method"
    def self.app(opts = {}, &block)
      $stderr.puts "[Ruby] Warning: C version of Shoes.app not found, using stub"
      nil
    end
  else
    $stderr.puts "[Ruby] Shoes.app method already defined by C"
  end
  
  # Define splash method for the idle timer
  def self.splash
    $stderr.puts "Shoes.splash called"
  end
end

# Skip the problematic secret_exit_hook for now
# We'll implement proper exit handling later

puts "[Shoes] Minimal shoes.rb loaded successfully"