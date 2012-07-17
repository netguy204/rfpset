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
num_sets = 7
size = 100000

puts "building #{num_sets} of #{size}\n"
sets = num_sets.times.map do |ii|
  puts "Set #{ii}"
  ngrams = generate_ngrams(size, 4)
  start = Time.now
  FPSet.to_file!(ngrams, ii.to_s)
  stop = Time.now
  puts " Writing time elapsed #{(stop - start) * 1000} milliseconds\n"
  ngrams
end

puts "computing the intersection with FPSet\n"
start = Time.now
join = FPSet.intersect_files(num_sets.times.map { |x| x.to_s })
stop = Time.now
puts "Time elapsed #{(stop - start) * 1000} milliseconds\n"

puts "Set contains #{(join.size)} items"

puts "computing the intersection with Array\n"

start = Time.now
result = sets.reduce do |last, current|
  last & current
end
stop = Time.now
puts "Time elapsed #{(stop - start) * 1000} milliseconds\n"

puts "Set contains #{(result.size)} items"

if Set.new(join) == Set.new(result) then
  puts 'the sets match'
else
  puts 'the sets do not match'
end

