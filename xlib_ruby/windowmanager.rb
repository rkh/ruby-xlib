#!/usr/bin/env ruby1.8

require 'x11'
include X11

# Monkeypatch event handlers for dynamic fill-in
class WindowManager

    def initialize
        events = Thread.new do
            loop do
                if self.event_pending?
                    self.query
                    eval("@"+self.next_event.downcase).call
                    self.event_pop
                    puts "Again..."
                end
                sleep 0.1
            end
        end
        a = Thread.new { loop { sleep 0.1; self.query if event_pending?; self.event_pop }}
        b = Thread.new { loop { sleep 0.4; puts self.next_event }}
    end

    @maprequest = Proc.new { |client| puts "MapRequest" }
    @destroynotify = Proc.new { |client| puts "DestroyNotify" }
    @motionnotify = Proc.new { |client| puts "MotionNotify" }
	@buttonpress = Proc.new { |client| puts "buttonpress" }
	@configurerequest = Proc.new { |client| puts "configurerequest" }
	@configurenotify = Proc.new { |client| puts "configurenotify" }
	@destroynotify = Proc.new { |client| puts "destroynotify" }
	@enternotify = Proc.new { |client| puts "enternotify" }
	@expose = Proc.new { |client| puts "expose" }
	@focusin = Proc.new { |client| puts "focusin" }
	@keypress = Proc.new { |client| puts "keypress" }
	@leavenotify = Proc.new { |client| puts "leavenotify" }
	@mappingnotify = Proc.new { |client| puts "mappingnotify" }
	@maprequest = Proc.new { |client| puts "maprequest" }
	@propertynotify = Proc.new { |client| puts "propertynotify" }
	@unmapnotify = Proc.new { |client| puts "unmapnotify" }

    def on_maprequest_do(&block)
        @maprequest = block
    end

    def on_destroynotify_do(&block)
        @destroynotify = block
    end

    def on_motionnotify_do(&block)
        @motionnotify = block
    end
end

# Create the WM global var
$windowmanager = WindowManager.new
$windowmanager.events.start

