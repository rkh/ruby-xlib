require 'ecore'
require 'ecore_x'
require 'evas'
require 'ecore_evas'
require 'edje'
require 'desktop-menu'

ee = Ecore::Evas::SoftwareX11.new
edje = Edje::Edje.new(ee.evas)
edje.load("launcher/launcher.edj", "launcher")
edje.resize(*edje.get_size_min)
ee.resize(*edje.get_size_max)

