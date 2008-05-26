#/usr/bin/env ruby
require "desktop-menu"
require "Edjewin"
require "Edjelist"

class Launcher
    MAPPINGS = {"utilities" => ["Utility", "Settings"], 
                "admin" => ["System, Development"], 
                "web" => ["Network"],
                "multimedia" => ["Audio", "Video", "AudioVideo", "Graphics"], 
                "office" => ["Office"]}

    DEFAULT_CONFIG = {:geo => [800,480],
               :paths => ["/usr/share/applications", ENV["HOME"]+"/.local/share/applications"],
               :theme => "(DATADIR)/dwm_edje/themes/default.edj"
              }

    def initialize(config={})
        @config = DEFAULT_CONFIG.merge(config)
        @ew = Edjewin.new(@config[:theme], "main")
        @ew.ee.resize(*@config[:geo])
        @ew.edje.resize(*@config[:geo])
        @@tmp = X11menu.new(@config[:paths])
        @ew.add_signal("QUIT", "*") do
            Ecore::main_loop_quit
        end        
        MAPPINGS.each do |key, value|
            @ew.add_signal("CATEGORY", key) { add_run_parts(key) }
        end
        @ee_edje = Edjelist.new(@config[:theme], "launcher", @ew.edje.evas)
    end

    def add_spotlight_signal(name, desktopfile)
        @ew.edje.part(name).text = @@tmp[desktopfile].name
        @ew.add_signal("RUN", name) do
            @@tmp[desktopfile].execute
        end
        ee_img = Evas::Image.new(@ew.edje.evas)
        ee_img.set_file(@@tmp[desktopfile].icon)
        ee_img.resize(32,32)
        ee_img.show
        @ew.edje.part(name+"_icon").swallow(ee_img)
    end

    def add_run_parts(category)
        @ee_edje.clear
        @ee_edje.move((@config[:geo][0]*0.3).round, (@config[:geo][0]*0.08).round)

        launchers = MAPPINGS[category].collect do |x|
            @@tmp.collect_by_category(x)
        end.flatten
        launchers.length.times do |x|
            @ee_edje.append(launchers[x].name)
            @ee_edje.add_signal("RUN") { launchers[x].execute } 
        end
    end

    def show
        @ew.show
    end

    attr_accessor :ew, :tmp
end

