require "ecore" 
require "ecore_x" 
require "evas" 
require "ecore_evas" 
require "edje"

class Edjewin
    def initialize(filename, part)
        @ee = Ecore::Evas::SoftwareX11.new
        @ee.borderless = true
        @ee.shaped = true
        @edje = Edje::Edje.new(@ee.evas)
        @edje.load(filename, part)
        @ee.set_size_min(*@edje.get_size_min)
        @ee.set_size_max(*@edje.get_size_max)
        @edje.resize(*@edje.get_size_min)
        @ee.resize(*@edje.get_size_min)
        @ee.title = "Launcher"
        @ee.sticky = true
    end

    def add_signal(name, part)
        @edje.on_signal(name, part) { |*data| yield data }
    end

    def show
        @ee.show
        @edje.show
        Ecore::main_loop_begin
    end

    attr_accessor :ee, :edje
end

