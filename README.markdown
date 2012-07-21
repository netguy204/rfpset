FPSet - Fast, Persistent Sets
=============================

FPSet is a very specialized library for performing large set
intersections against data that is too large to fit into memory. It
does this by storing the sets on disk in an ordered binary format and
performing the intersection as it streams just-enough data from
disk. The result is a very memory friendly and performant set
intersection that's appropriate for very large sets.

To use:

Ahead of time, presumably off-line in a monthly cron-job or something,
we build our sets:

``` ruby
setNames.each do |name|
  strings = fetch_set_named(name)
  FPSet.to_file(name, strings)
end
```

Then, presumably at runtime, we can do our big set intersections:

``` ruby
common_terms = FPSet.intersect_files(setNames)
```

To slurp in a set from just one of the files:

```ruby
set = FBSet.from_file(setNames[0])
```

This is a bundler created gem. To build and install just run:

``` bash
gem build rfpset.gemspec
gem install rfpset-0.0.1.gem
```

