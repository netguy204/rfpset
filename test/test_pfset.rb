require 'test/unit'
require 'rpfset'

class PFSetTest < Test::Unit::TestCase

  def test_bonjour
    assert_equal "hello world!", FPSetInternal.bonjour
  end
end
