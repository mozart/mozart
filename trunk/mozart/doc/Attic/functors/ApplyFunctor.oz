fun {`ApplyFunctor` FileName F}
   ModMan = {New Module.manager init()}
in
   {ModMan apply(url: FileName F $)}
end
