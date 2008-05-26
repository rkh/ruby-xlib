class Desktopfile
    require 'abbrev'
    require 'Pthreads'
    include Pthreads

    attr_accessor :name, :bin, :params, :icon, :category
    
    def << string
        if string =~ /Name=[ ]*([^\n]*)|Exec=[ ]*([^ \n]*)([-]*[^\n]*)|Icon=[ ]*([^\n]*)/
            @name ||= $1 if $1
            @bin ||= $2 if $2
            @params ||= $3.gsub(/%\w/, '').strip if $3                        
            if $4
                @icon ||= $4
                @icon = Dir["/usr/share/pixmaps/#@icon*"][0] if Dir["/usr/share/pixmaps/#@icon*"].compact[0]
                @icon = Dir["/usr/share/icons/Tango/128x128/apps/#@icon*"][0] if Dir["/usr/share/icons/Tango/128x128/apps/#@icon*"].compact[0]
            end
        elsif string =~ /Categories=.*?(AudioVideo)|(Audio)|(Video)|(Development)|(Education)|(Game).*?/
            @category = $1 if $1
            @category = $2 if $2
            @category = $3 if $3
            @category = $4 if $4
            @category = $5 if $5
            @category = $6 if $6        
        elsif string =~ /Categories=.*?(Graphics)|(Network)|(Office)|(Settings)|(System)|(Utility)|(Graphics).*?/
            @category = $1 if $1
            @category = $2 if $2
            @category = $3 if $3
            @category = $4 if $4
            @category = $5 if $5
            @category = $6 if $6
        end
        self
    end

    def to_s
        "Name=#@name\nExec=#@bin#@params\nIcon=#@icon"
    end

    def include? string
        [@name, @bin, @icon].any? { |s| s.upcase == string.upcase }
    end

    def include string
        return self if [@name, @bin, @icon].any? { |s| s.upcase == string.upcase }
        nil
    end

    def similar string
       # Wow! Ein Bug in abbrev! Die Konstruktion liefert viel zu wenige Strings  
       # return true if [@name.upcase, @bin.upcase, @icon.upcase].abbrev[string.upcase]
        ([@name].abbrev.keys + [@bin].abbrev.keys).collect! do |x|
            x.upcase
        end.include? string.upcase
    end

    def execute
        #Thread.new { system(@bin+@params) }
        pthreads(@bin+@params)
    end
end

class X11menu
    require 'fileutils'
    include FileUtils::Verbose

    @@files ||= []
    @@bin ||= []    

    def initialize dirs=["/usr/share/applications", ENV["HOME"]+"/.local/share/applications/"]
        if @@files.empty?
            dirs.each do |dir|
                Dir[dir+"/*.desktop"].each do |x|
                    tmp = Desktopfile.new
                    File.read(x).each_line do |y|
                        tmp << y 
                    end
                    @@files << tmp
                end
            end
        end     
    end

    def [] x
        @@files[x]
    end

    def << filename
        tmp = Desktopfile.new
        File.read(x).each_line { |y| tmp << y }
        @@files << tmp
    end

    def names
        (@@files.collect { |x| x.name })
    end

    def icons
        (@@files.collect { |x| x.icon })
    end

    def bin
        (@@files.collect { |x| x.bin })
    end    

    def collect_by_category category
        (@@files.collect { |x| x if x.category == category }).compact
    end

    def exec 
        (@@files.collect { |x| x.bin+x.params })
    end

    def to_s
        s = ""
        @@files.each { |x| s += x.to_s+"\n" }
        s
    end

    def include? string
        @@files.any? { |x| x.include? string }
    end

    def length
        @@files.length
    end

    def include string
        (@@files.collect { |x| x.include string }).compact
    end

    def similar string
        (@@files.collect { |x| x if x.similar string }).compact
    end
end
