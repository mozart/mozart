OZ_Return ConnectProp::propagate() {

printf("ConnectProp::propagate\n");
 
  OZ_FDIntVar fd(_fd);
  OZ_FSetVar  fs(_fs);

  // 1st propagation rule
  fs->putCard(1, 1); 

  // 2nd propagation rule
  FailOnEmpty(*fd -= fs->getNotInSet());

  // 3rd propagation rule
  FailOnInvalid(*fs <= OZ_FSetConstraint(*fd));

  return (fd.leave() | fs.leave()) ? OZ_SLEEP : OZ_ENTAILED;

failure:
  fd.fail(); fs.fail();
  return OZ_FAILED;
}
