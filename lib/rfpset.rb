require "rfpset/version"
require "rfpset/rfpset"

module FPSet
  # Your code goes here...
  def to_file(data, filename)
    array = Array(data.collect { |d| Marshal.dump(d) })
    result = FPSetInternal.spit_array(array, filename)
    raise "does the file #{filename} exist?" if result == -1
    return result
  end
  module_function :to_file

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
