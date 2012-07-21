require 'rubygems'
require 'rfpset'

# generate some random data
$alphabet = Array('a'..'z')
def generate_ngrams(count, width)
  count.times.map do 
    (width.times.map { $alphabet[Random.rand($alphabet.size)] }).join('')
  end
end

# make n big sets
#num_sets = 7
puts "num_sets, size, fpset_write, fpset_intersect, array_intersect, set_size"
(1..7).each do |num_sets|
  (1..5).each do |pre_size|

    size = 30000 * pre_size
  
    fpset_write = 0
    sets = num_sets.times.map do |ii|
      ngrams = generate_ngrams(size, 4)
      start = Time.now
      FPSet.to_file(ngrams, ii.to_s)
      stop = Time.now
      fpset_write += (stop - start)
      ngrams
    end
  
    start = Time.now
    join = FPSet.intersect_files(num_sets.times.map { |x| x.to_s })
    stop = Time.now
    fpset_intersect = stop - start

    set_size = join.size    
    
    start = Time.now
    result = sets.reduce do |last, current|
      last & current
    end

    stop = Time.now
    array_intersect = stop - start

    puts "#{num_sets}, #{size}, #{(fpset_write*1000)}, #{(fpset_intersect*1000)}, #{(array_intersect*1000)}, #{set_size}"

  end
end

