require File.join(File.dirname(__FILE__),'..','ext','x11');
  
module X11
  KEY_MASK_NAMES = [
    :Shift,
    :Lock,
    :Control,
    :Mod1,
    :Mod2,
    :Mod3,
    :Mod4,
    :Mod5,
    :Button1,
    :Button2,
    :Button3,
    :Button4,
    :Button5
  ]
  
  ShiftMask   = 1 << 0
  LockMask    = 1 << 1
  ControlMask = 1 << 2
  Mod1Mask    = 1 << 3
  Mod2Mask    = 1 << 4
  Mod3Mask    = 1 << 5
  Mod4Mask    = 1 << 6
  Mod5Mask    = 1 << 7
  Button1Mask = 1 << 8
  Button2Mask = 1 << 9
  Button3Mask = 1 << 10
  Button4Mask = 1 << 11
  Button5Mask = 1 << 12
  
  EVENT_NAMES = {
    2  => :KeyPress,
    3  => :KeyRelease,
    4  => :ButtonPress,
    5  => :ButtonRelease,
    6  => :MotionNotify,
    7  => :EnterNotify,
    8  => :LeaveNotify,
    9  => :FocusIn,
    10 => :FocusOut,
    11 => :KeymapNotify,
    12 => :Expose,
    13 => :GraphicsExpose,
    14 => :NoExpose,
    15 => :VisibilityNotify,
    16 => :CreateNotify,
    17 => :DestroyNotify,
    18 => :UnmapNotify,
    19 => :MapNotify,
    20 => :MapRequest,
    21 => :ReparentNotify,
    22 => :ConfigureNotify,
    23 => :ConfigureRequest,
    24 => :GravityNotify,
    25 => :ResizeRequest,
    26 => :CirculateNotify,
    27 => :CirculateRequest,
    28 => :PropertyNotify,
    29 => :SelectionClear,
    30 => :SelectionRequest,
    31 => :SelectionNotify,
    32 => :ColormapNotify,
    33 => :ClientMessage,
    34 => :MappingNotify,
    35 => :GenericEvent
  }

  KeyPress           = 2 
  KeyRelease         = 3 
  ButtonPress        = 4 
  ButtonRelease      = 5 
  MotionNotify       = 6 
  EnterNotify        = 7 
  LeaveNotify        = 8 
  FocusIn            = 9 
  FocusOut           = 10
  KeymapNotify       = 11
  Expose             = 12
  GraphicsExpose     = 13
  NoExpose           = 14
  VisibilityNotify   = 15 
  CreateNotify       = 16
  DestroyNotify      = 17
  UnmapNotify        = 18
  MapNotify          = 19
  MapRequest         = 20
  ReparentNotify     = 21
  ConfigureNotify    = 22
  ConfigureRequest   = 23 
  GravityNotify      = 24
  ResizeRequest      = 25
  CirculateNotify    = 26
  CirculateRequest   = 27 
  PropertyNotify     = 28
  SelectionClear     = 29
  SelectionRequest   = 30 
  SelectionNotify    = 31
  ColormapNotify     = 32
  ClientMessage      = 33
  MappingNotify      = 34
  GenericEvent       = 35

  class WindowManager
    def debug_event_loop
      loop do
        ev = get_event
        if ev.type == MapRequest && ev.client
          manage ev.client,10,10,100,100
        end
        break if ev.keysym == "Escape"
        puts "#{EVENT_NAMES[ev.type]} #{ev.wid.to_s(16)}:#{ev.client} " \
             "keysym=#{ev.keysym} mods=#{ev.mods} button=#{ev.button} coord=(#{ev.x},#{ev.y})"
      end
    end
  end
end
