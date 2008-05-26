require 'evas'
require 'edje'

class Edjelist
    def initialize(filename, part, evas)
        @parent_evas = evas
        @evas = Evas::EvasObject.new(@parent_evas)
        @filename = filename
        @part = part        
        @list = []
        @cur = 0
        @cur_y = 0
        @cur_x = 0
    end

    def append(text, filename = @filename, part = @part)
        if @list[@cur]
            @cur_y += @list[@cur].get_size_max[1]
            @cur += 1
        end
        @filename = filename
        @part = part
        @list[@cur] = Edje::Edje.new(@evas.evas)
        @list[@cur].load(filename, part)
        @list[@cur].resize(*@list[@cur].get_size_max)    
        @list[@cur].move(@cur_x, @cur_y)
        @list[@cur].part("1_text").text = text
        @list[@cur].show
    end

    def add_signal(name)
        @list[@cur].on_signal(name, "*") { |*data| yield data }
    end


    def clear
        @list.each { |x| x.delete }
        @cur = 0
        @cur_y = 0
        @cur_x = 0
        @list.clear
    end

    def move(x,y)
        @cur_x = x
        @cur_y = y
    end

    def show
        @list.each { |x| x.show }        
    end

    def hide
        @list.each { |x| x.hide }
    end

    attr_accessor :evas

end


