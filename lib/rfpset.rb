require "rfpset/version"
require "rfpset/rfpset"
require 'set'

module FPSet
  # Create a new set-file
  #
  # Example:
  #   >> FPSet.to_file([1,2,5,3,5,4], "numbers.dat")
  #
  # Arguments:
  #   data: (Enumerable, can contain duplicates)
  #   filename: (String)

  def to_file(data, filename)
    array = (data.collect { |d| Marshal.dump(d) }).to_a
    result = FPSetInternal.spit_array(array, filename)
    raise "does the file #{filename} exist?" if result == -1
    return result
  end
  module_function :to_file

  # Create a new set-file. Mutates provided data to save memory.
  #
  # Example:
  #   >> arr = [1,2,5,3,5,4]
  #   >> FPSet.to_file!(arr, "numbers.dat")
  #   >> arr = nil # array is full of garbage now
  #
  # Arguments:
  #   data: (Array, will be mutated)
  #   filename: (String)

  def to_file!(data, filename)
    return to_file(data, filename) if not data.kind_of?(Array)

    data.collect! { |d| Marshal.dump(d) }
    result = FPSetInternal.spit_array(data, filename)
    raise "does the file #{filename} exist?" if result == -1
    return result
  end
  module_function :to_file!

  # Slurp a set-file from disk into a Ruby set.
  #
  # Example:
  #   >> set = FPSet.from_file("numbers.dat")
  #
  # Arguments:
  #   filename: (String)

  def from_file(filename)
    result = FPSetInternal.slurp_array(filename)
    raise "does the file #{filename} exist?" if result == -1
    result.map { |s| Marshal.load(s) }
  end
  module_function :from_file

  # Compute the intersection of set-files
  #
  # Example:
  #   >> set = FPSet.intersect_files(["numbers1.dat", "numbers2.dat"])
  #
  # Arguments:
  #   filenames: (Enumerable of Strings)

  def intersect_files(filenames)
    array = Array(filenames.collect { |f| f.to_s })
    result = FPSetInternal.intersect_files(array)
    if result == -1 then
      names = array.join(", ")
      raise "do all files exist? (#{names})"
    end
    return result.map { |s| Marshal.load(s) }
  end
  module_function :intersect_files  
end
