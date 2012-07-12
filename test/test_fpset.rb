require 'test/unit'
require 'rfpset'
require 'set'

class FPSetTest < Test::Unit::TestCase

  def test_primitive
    data = ["one", "two", "three"]
    testfile = "test.dat"
    testfile2 = "test2.dat"

    assert_equal 3, FPSetInternal.spit_array(data, testfile)

    new_data = FPSetInternal.slurp_array(testfile)
    assert_equal data, new_data

    data2 = ["three", "four", "five"]
    assert_equal 3, FPSetInternal.spit_array(data2, testfile2)

    intersect = FPSetInternal.intersect_files([testfile, testfile2])
    assert_equal 1, intersect.size
    assert_equal "three", intersect[0]
  end

  def test_porcelain
    test1 = "test.dat"
    test2 = "test2.dat"

    FPSet.to_file(1..5, test1)
    FPSet.to_file(3..6, test2)
    assert_equal 3, FPSet.intersect_files([test1, test2]).size
    assert_equal (1..5).to_set, FPSet.from_file(test1)
  end
end

