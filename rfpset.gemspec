# -*- encoding: utf-8 -*-
$:.push File.expand_path("../lib", __FILE__)
require "rfpset/version"

Gem::Specification.new do |s|
  s.name        = "rfpset"
  s.version     = Rfpset::VERSION
  s.authors     = ["Brian Taylor"]
  s.email       = ["el.wubo@gmail.com"]
  s.homepage    = "http://www.50ply.com/blog/2012/07/21/introducing-fast/"
  s.summary     = %q{Fast, persistent sets}
  s.description = %q{Fast, persistent sets supporting efficient intersections of many very large sets.}

  s.rubyforge_project = "rfpset"

  s.files         = `git ls-files`.split("\n")
  s.extensions    = ['ext/rfpset/extconf.rb']

  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.executables   = `git ls-files -- bin/*`.split("\n").map{ |f| File.basename(f) }
  s.require_paths = ["lib"]

  # specify any dependencies here; for example:
  # s.add_development_dependency "rspec"
  # s.add_runtime_dependency "rest-client"
end
